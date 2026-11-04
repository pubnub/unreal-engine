#!/usr/bin/env python3
"""Cursor preToolUse hook: wrap Unreal Engine test runs and keep only failures.

preToolUse cannot replace a Shell tool's output after the fact. The command is
rewritten so the process the agent runs prints failed tests, their errors, and
the final result. The full log is still written to disk.

Matches editor automation (Automation RunTests / RunAll, -TestExit), Gauntlet
(-test= / RunUnreal), and LowLevelTests. Ordinary compiles stay with ue_compile_wrap.py.

Usage:
  python .cursor/hooks/ue_test_wrap.py --hook
  python .cursor/hooks/ue_test_wrap.py --run --cmd-file <path>
  python .cursor/hooks/ue_test_wrap.py --self-test
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
import time
import uuid
from collections import deque
from pathlib import Path
from typing import Any, Iterable


SCRIPT_PATH = Path(__file__).resolve()
MAX_FAILED = 40
MAX_ERRORS_PER_TEST = 8
MAX_ORPHAN_ERRORS = 12
MAX_LINE_CHARS = 400
MAX_TAIL_LINES = 12
MAX_LOG_BYTES = 32 * 1024 * 1024
START_BANNER = "UE TEST WRAP: running Unreal tests; model will receive failures and the final result, not the raw log."

# Substrings are intentional: agent commands quote paths with spaces.
UE_TEST_PATTERNS = (
    re.compile(r"(?i)\bAutomation\s+Run(?:Tests|All)\b"),
    re.compile(r"(?i)-TestExit\b"),
    re.compile(r"(?i)(?:^|[\s\"'\\/])LowLevelTests(?:\.exe)?(?:\s|\"|'|$)"),
    re.compile(r"(?i)\bGauntlet(?:\.exe)?\b"),
    re.compile(r"(?i)-test=UE\."),
    re.compile(r"(?i)\bRunUnreal\b"),
    re.compile(r"(?i)-ExecCmds=[\"'][^\"']*\bAutomation\b"),
    re.compile(r"(?i)\bUnrealEditor(?:-Cmd|-Win64-[A-Za-z]+-Cmd)?(?:\.exe|-Cmd)?\b.*\bAutomation\b"),
)

VIEWER_PREFIX = re.compile(
    r"(?i)^\s*(?:&\s*)?(?:"
    r"Get-Content|Get-Item|Get-ChildItem|Get-Command|Get-Help|"
    r"Select-String|Out-File|Set-Content|"
    r"gc\b|cat\b|type\b|more\b|less\b|tail\b|head\b|"
    r"findstr\b|find\b|rg\b|grep\b|where(?:\.exe)?|"
    r"notepad(?:\+\+)?|code\b|cursor\b|vim\b|emacs\b|"
    r"dir\b|ls\b|echo\b|Write-Output|Write-Host"
    r")\b"
)

TEST_STARTED = re.compile(
    r"Test Started\. Name=\{(?P<name>[^}]*)\}(?:\s+Path=\{(?P<path>[^}]*)\})?"
)
TEST_COMPLETED = re.compile(
    r"Test Completed\. Result=\{(?P<result>[^}]+)\}"
    r"(?:\s+Name=\{(?P<name>[^}]*)\})?"
    r"(?:\s+Path=\{(?P<path>[^}]*)\})?"
)
TEST_EXIT = re.compile(r"\*\*\*\* TEST COMPLETE\. EXIT CODE:\s*(?P<code>-?\d+)\s*\*\*\*\*")
QUEUE_EMPTY = re.compile(
    r"Automation Test Queue Empty\s+(?P<count>\d+)\s+tests performed"
)
AUTOMATION_ERROR = re.compile(
    r"(?i)LogAutomation(?:Test|Controller|CommandLine)\s*:\s*(?:Error|Fatal)\s*:\s*(?P<msg>.*)$"
)
FATAL_LINE = re.compile(
    r"(?i)(?:^|\s)(?:Fatal error:|Assertion failed:|Ensure condition failed:|Critical error:)"
)
# Catch2 / LowLevelTests summary vs a single failed case.
CATCH_CASE_FAILED = re.compile(r"\[\s*FAILED\s*\]\s+(?P<name>\S+)\s*$")
CATCH_CASE_PASSED = re.compile(r"\[\s*PASSED\s*\]\s+(?P<name>\S+)\s*$")
CATCH_SUMMARY_FAILED = re.compile(r"\[\s*FAILED\s*\]\s+\d+\s+tests?")
CATCH_SUMMARY_PASSED = re.compile(r"\[\s*PASSED\s*\]\s+(?P<count>\d+)\s+tests?")
NOISE_ERROR = re.compile(
    r"(?i)(?:0 Error\(s\)|error handling|no error|if error|on error)"
)


def emit(payload: dict[str, Any]) -> None:
    sys.stdout.write(json.dumps(payload, ensure_ascii=False) + "\n")
    sys.stdout.flush()


def allow() -> int:
    emit({"permission": "allow"})
    return 0


def wrap_state_dir() -> Path:
    root = Path(os.environ.get("LOCALAPPDATA") or tempfile.gettempdir()) / "cursor-ue-test-wrap"
    root.mkdir(parents=True, exist_ok=True)
    (root / "logs").mkdir(exist_ok=True)
    cutoff = time.time() - 24 * 3600
    for stale in root.glob("cmd-*.json"):
        try:
            if stale.stat().st_mtime < cutoff:
                stale.unlink()
        except OSError:
            pass
    return root


def is_already_wrapped(command: str) -> bool:
    return (
        "ue_test_wrap.py" in command
        or "CURSOR_UE_TEST_WRAP" in command
        or "compile_wrap.py" in command
        or "go_test_wrap.py" in command
        or "CURSOR_GO_TEST_WRAP" in command
    )


def is_ue_test_command(command: str) -> bool:
    if not command or not command.strip():
        return False
    if is_already_wrapped(command):
        return False
    if VIEWER_PREFIX.match(command):
        return False
    if not any(pattern.search(command) for pattern in UE_TEST_PATTERNS):
        return False
    # RunUAT BuildCookRun is a cook/package, not a test, unless it launches tests.
    lowered = command.lower()
    if "runuat" in lowered and "automation" not in lowered and "-test=" not in lowered and "gauntlet" not in lowered and "rununreal" not in lowered:
        return False
    return True


def parse_tool_input(payload: dict[str, Any]) -> dict[str, Any]:
    tool_input = payload.get("tool_input") or {}
    if isinstance(tool_input, str):
        try:
            parsed = json.loads(tool_input)
            tool_input = parsed if isinstance(parsed, dict) else {"command": tool_input}
        except json.JSONDecodeError:
            tool_input = {"command": tool_input}
    if not isinstance(tool_input, dict):
        return {}
    return dict(tool_input)


def command_from_input(tool_input: dict[str, Any], payload: dict[str, Any]) -> str:
    for key in ("command", "cmd"):
        value = tool_input.get(key)
        if isinstance(value, str) and value.strip():
            return value
    value = payload.get("command")
    return value if isinstance(value, str) else ""


def cwd_from_input(tool_input: dict[str, Any], payload: dict[str, Any]) -> str:
    for source in (tool_input, payload):
        for key in ("working_directory", "workingDirectory", "cwd"):
            value = source.get(key)
            if isinstance(value, str) and value.strip():
                return value
    return ""


def truncate(text: str, limit: int = MAX_LINE_CHARS) -> str:
    text = " ".join(text.split())
    if len(text) <= limit:
        return text
    return text[: limit - 3] + "..."


def message_after_colon(line: str) -> str:
    match = AUTOMATION_ERROR.search(line.strip())
    if match:
        return match.group("msg").strip()
    return line.strip()


class TestRun:
    def __init__(self) -> None:
        self.passed = 0
        self.failed = 0
        self.skipped = 0
        self.other = 0
        self.failed_tests: list[dict[str, Any]] = []
        self.orphan_errors: list[str] = []
        self.current: dict[str, Any] | None = None
        self.ue_exit_code: int | None = None
        self.tests_performed: int | None = None
        self.saw_fatal = False
        self.tail: deque[str] = deque(maxlen=MAX_TAIL_LINES)

    def _remember_error(self, text: str) -> None:
        text = truncate(text)
        if not text or NOISE_ERROR.search(text):
            return
        if TEST_COMPLETED.search(text):
            return
        if self.current is not None:
            errors: list[str] = self.current["errors"]
            if text not in errors and len(errors) < MAX_ERRORS_PER_TEST:
                errors.append(text)
            return
        if text not in self.orphan_errors and len(self.orphan_errors) < MAX_ORPHAN_ERRORS:
            self.orphan_errors.append(text)

    def _close_current(self, result: str | None) -> None:
        current = self.current
        self.current = None
        if current is None:
            return
        if result is None:
            result = "Incomplete"
        normalized = result.strip().lower()
        name = current["name"] or current["path"] or "(unnamed test)"
        entry = {
            "name": name,
            "path": current["path"],
            "result": result,
            "errors": list(current["errors"]),
        }
        if normalized == "success":
            self.passed += 1
        elif normalized in {"fail", "failed", "error"}:
            self.failed += 1
            self.failed_tests.append(entry)
        elif normalized == "skipped":
            self.skipped += 1
        else:
            self.other += 1
            if normalized not in {"notrun", "not run", "inprocess"}:
                self.failed_tests.append(entry)

    def consume(self, line: str) -> None:
        stripped = line.rstrip("\n").strip()
        if not stripped:
            return

        started = TEST_STARTED.search(stripped)
        if started:
            self._close_current(None)
            self.current = {
                "name": started.group("name") or "",
                "path": started.group("path") or "",
                "errors": [],
            }
            return

        completed = TEST_COMPLETED.search(stripped)
        if completed:
            if self.current is None:
                self.current = {
                    "name": completed.group("name") or "",
                    "path": completed.group("path") or "",
                    "errors": [],
                }
            elif completed.group("name"):
                self.current["name"] = completed.group("name") or self.current["name"]
                self.current["path"] = completed.group("path") or self.current["path"]
            self._close_current(completed.group("result"))
            return

        exit_match = TEST_EXIT.search(stripped)
        if exit_match:
            self.ue_exit_code = int(exit_match.group("code"))
            return

        queue_match = QUEUE_EMPTY.search(stripped)
        if queue_match:
            self.tests_performed = int(queue_match.group("count"))
            return

        if CATCH_SUMMARY_FAILED.search(stripped) or CATCH_SUMMARY_PASSED.search(stripped):
            passed_summary = CATCH_SUMMARY_PASSED.search(stripped)
            if passed_summary and self.passed == 0:
                self.passed = int(passed_summary.group("count"))
            return

        catch_failed = CATCH_CASE_FAILED.search(stripped)
        if catch_failed:
            name = catch_failed.group("name")
            self.failed += 1
            self.failed_tests.append(
                {"name": name, "path": "", "result": "Fail", "errors": []}
            )
            return

        catch_passed = CATCH_CASE_PASSED.search(stripped)
        if catch_passed and "tests" not in catch_passed.group("name").lower():
            self.passed += 1
            return

        if FATAL_LINE.search(stripped):
            self.saw_fatal = True
            self._remember_error(stripped)
            return

        auto_error = AUTOMATION_ERROR.search(stripped)
        if auto_error:
            self._remember_error(auto_error.group("msg"))
            return

        if not stripped.lower().startswith("["):
            self.tail.append(truncate(stripped))


def format_summary(
    *,
    command: str,
    exit_code: int,
    duration_sec: float,
    run: TestRun,
    log_path: Path,
    raw_lines: int,
) -> str:
    ue_failed = run.ue_exit_code not in (None, 0)
    failed = exit_code != 0 or run.failed > 0 or ue_failed or run.saw_fatal
    status = "FAILED" if failed else "SUCCEEDED"
    shown = run.failed_tests[:MAX_FAILED]
    omitted = max(0, len(run.failed_tests) - len(shown))
    performed = run.tests_performed
    if performed is None:
        performed = run.passed + run.failed + run.skipped + run.other

    lines = [
        "=== UE TEST SUMMARY (raw test log stripped to save context) ===",
        f"status: {status}",
        f"exit_code: {exit_code}",
        f"ue_exit_code: {run.ue_exit_code if run.ue_exit_code is not None else 'n/a'}",
        f"duration_sec: {duration_sec:.1f}",
        f"tests_performed: {performed}",
        f"tests_passed: {run.passed}",
        f"tests_failed: {run.failed}",
        f"tests_skipped: {run.skipped}",
        f"raw_log_lines: {raw_lines}",
        f"raw_log: {log_path}",
        f"original_command: {truncate(command, 300)}",
        "",
        "Use this summary as the Unreal test result. Do not re-run the tests just to inspect failures.",
        "Open the cited source files. Read raw_log only if a failure message is missing here.",
    ]

    if shown:
        lines.append("")
        lines.append("FAILED TESTS:")
        for index, test in enumerate(shown, start=1):
            label = test["path"] or test["name"]
            if test["path"] and test["name"] and test["name"] not in test["path"]:
                label = f"{test['name']} ({test['path']})"
            lines.append(f"{index}. {truncate(label)} result={test['result']}")
            for error in test["errors"]:
                lines.append(f"   - {error}")
            if not test["errors"]:
                lines.append("   - (no parsed error text; see raw_log)")
        if omitted:
            lines.append(f"... {omitted} more failed tests omitted; see raw_log")

    if run.orphan_errors and (failed or not shown):
        lines.append("")
        lines.append("ERRORS:")
        for error in run.orphan_errors:
            lines.append(f"- {error}")

    tail = [item for item in run.tail if item.strip()]
    if failed and not shown and not run.orphan_errors and tail:
        lines.append("")
        lines.append("TAIL (no structured test failures parsed; last output):")
        lines.extend(f"| {item}" for item in tail)

    if not failed:
        lines.append("")
        lines.append("tests reported success")

    lines.append("=== END UE TEST SUMMARY ===")
    return "\n".join(lines) + "\n"


def run_tests(command: str, cwd: str | None, log_path: Path) -> tuple[int, str]:
    run = TestRun()
    raw_lines = 0
    log_bytes = 0

    popen_kwargs: dict[str, Any] = {
        "args": [
            "powershell.exe",
            "-NoProfile",
            "-NonInteractive",
            "-ExecutionPolicy",
            "Bypass",
            "-Command",
            command,
        ],
        "stdout": subprocess.PIPE,
        "stderr": subprocess.STDOUT,
        "cwd": cwd or None,
        "env": {**os.environ, "CURSOR_UE_TEST_WRAP": "1"},
        "text": True,
        "encoding": "utf-8",
        "errors": "replace",
        "bufsize": 1,
    }
    if os.name == "nt":
        popen_kwargs["creationflags"] = getattr(subprocess, "CREATE_NO_WINDOW", 0)

    started = time.perf_counter()
    proc = subprocess.Popen(**popen_kwargs)
    assert proc.stdout is not None

    print(START_BANNER, flush=True)

    with log_path.open("w", encoding="utf-8", errors="replace") as log_file:
        log_file.write(f"# original_command: {command}\n")
        log_file.write(f"# cwd: {cwd or os.getcwd()}\n\n")
        for line in proc.stdout:
            raw_lines += 1
            encoded = line.encode("utf-8", "replace")
            if log_bytes < MAX_LOG_BYTES:
                writable = encoded[: MAX_LOG_BYTES - log_bytes]
                log_file.write(writable.decode("utf-8", "replace"))
                log_bytes += len(writable)
                if log_bytes >= MAX_LOG_BYTES:
                    log_file.write("\n# truncated: log exceeded 32 MiB\n")
            run.consume(line)

    run._close_current("Incomplete")
    exit_code = proc.wait()
    duration = time.perf_counter() - started
    summary = format_summary(
        command=command,
        exit_code=exit_code,
        duration_sec=duration,
        run=run,
        log_path=log_path,
        raw_lines=raw_lines,
    )
    return exit_code, summary


def hook_main() -> int:
    try:
        raw = sys.stdin.read()
        payload = json.loads(raw) if raw.strip() else {}
    except json.JSONDecodeError:
        return allow()

    if payload.get("tool_name") and payload.get("tool_name") != "Shell":
        return allow()

    tool_input = parse_tool_input(payload)
    command = command_from_input(tool_input, payload)
    if not is_ue_test_command(command):
        return allow()

    cwd = cwd_from_input(tool_input, payload)
    cmd_path = wrap_state_dir() / f"cmd-{uuid.uuid4().hex}.json"
    cmd_path.write_text(
        json.dumps(
            {
                "command": command,
                "cwd": cwd,
                "conversation_id": payload.get("conversation_id"),
            },
            ensure_ascii=False,
            indent=2,
        ),
        encoding="utf-8",
    )

    wrap_command = subprocess.list2cmdline(
        [sys.executable, str(SCRIPT_PATH), "--run", "--cmd-file", str(cmd_path)]
    )
    updated = dict(tool_input)
    updated["command"] = wrap_command
    emit(
        {
            "permission": "allow",
            "updated_input": updated,
            "agent_message": (
                "UE test command wrapped: you will receive failed tests, their errors, "
                "and the final result instead of the raw automation log. Full log is saved on disk."
            ),
        }
    )
    return 0


def run_main(cmd_file: Path) -> int:
    try:
        payload = json.loads(cmd_file.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"UE TEST WRAP: failed to read command file {cmd_file}: {exc}", file=sys.stderr)
        return 1
    finally:
        try:
            cmd_file.unlink(missing_ok=True)
        except OSError:
            pass

    command = payload.get("command") or ""
    cwd = payload.get("cwd") or None
    if cwd and not Path(cwd).is_dir():
        cwd = None
    if not command.strip():
        print("UE TEST WRAP: empty command", file=sys.stderr)
        return 1

    stamp = time.strftime("%Y%m%d-%H%M%S")
    log_path = wrap_state_dir() / "logs" / f"{stamp}-{uuid.uuid4().hex[:8]}.log"
    exit_code, summary = run_tests(command, cwd, log_path)
    sys.stdout.write(summary)
    sys.stdout.flush()
    return exit_code


def self_test() -> int:
    test_cases = [
        r'& "C:\UE\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Game\Game.uproject" -ExecCmds="Automation RunTests Now Project.Functional" -TestExit="Automation Test Queue Empty" -unattended -NullRHI',
        r'UnrealEditor-Cmd.exe Game.uproject -ExecCmds="Automation RunAll"',
        r'Engine\Binaries\Win64\UnrealEditor-Win64-Development-Cmd.exe Proj.uproject -ExecCmds="Automation RunTests Filter" -unattended',
        r'& "C:\UE\Engine\Build\BatchFiles\RunUAT.bat" RunUnreal -platform=Win64 -test=UE.EditorAutomation',
        r'LowLevelTests.exe --gtest_filter=Pubnub.*',
        r'"C:\UE\Engine\Binaries\Win64\LowLevelTests.exe"',
        r'Gauntlet.exe RunTests -project=Game',
    ]
    non_test_cases = [
        r'Engine\Build\BatchFiles\Build.bat GameEditor Win64 Development',
        r'& "C:\UE\Engine\Binaries\Win64\UnrealEditor.exe" Game.uproject',
        "git status",
        "Get-Content Automation.log",
        "rg \"Automation RunTests\" Saved\\Logs",
        "echo Automation RunTests",
        "pytest tests/",
        "ctest --output-on-failure",
        f"{sys.executable} {SCRIPT_PATH} --run --cmd-file x.json",
    ]
    failed = 0
    for command in test_cases:
        if not is_ue_test_command(command):
            print(f"FAIL expected ue test: {command}")
            failed += 1
    for command in non_test_cases:
        if is_ue_test_command(command):
            print(f"FAIL expected skip: {command}")
            failed += 1

    sample = [
        "LogTemp: Display: Loading editor",
        "LogAutomationController: Display: Test Started. Name={Encrypts payload} Path={Project.Pubnub.Encrypts payload}",
        "LogAutomationTest: Error: Expected 'cipher' to be 1, but it was 0.",
        "LogAutomationController: Error: Test Completed. Result={Fail} Name={Encrypts payload} Path={Project.Pubnub.Encrypts payload}",
        "LogAutomationController: Display: Test Started. Name={Other} Path={Project.Pubnub.Other}",
        "LogAutomationController: Display: Test Completed. Result={Success} Name={Other} Path={Project.Pubnub.Other}",
        "LogAutomationCommandLine: Display: ...Automation Test Queue Empty 2 tests performed.",
        "LogAutomationCommandLine: Display: **** TEST COMPLETE. EXIT CODE: -1 ****",
        "[  FAILED  ] Pubnub.LegacyCryptor",
        "[  PASSED  ] 3 tests.",
    ]
    run = TestRun()
    for line in sample:
        run.consume(line)
    run._close_current(None)
    if run.failed < 2 or run.passed < 1:
        print(f"FAIL counts failed={run.failed} passed={run.passed}")
        failed += 1
    if run.ue_exit_code != -1 or run.tests_performed != 2:
        print(f"FAIL markers exit={run.ue_exit_code} performed={run.tests_performed}")
        failed += 1
    first = run.failed_tests[0]
    if "Expected 'cipher'" not in " ".join(first["errors"]):
        print(f"FAIL missing error text: {first}")
        failed += 1
    if any("Loading editor" in item for item in run.orphan_errors):
        print("FAIL startup log leaked into errors")
        failed += 1

    summary = format_summary(
        command=test_cases[0],
        exit_code=255,
        duration_sec=1.2,
        run=run,
        log_path=Path("C:/temp/ue-test.log"),
        raw_lines=len(sample),
    )
    if "status: FAILED" not in summary or "Encrypts payload" not in summary or "tests reported success" in summary:
        print("FAIL summary content")
        print(summary)
        failed += 1

    if failed:
        print(f"self-test failed: {failed} check(s)")
        return 1
    print("self-test passed")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Wrap Unreal Engine test runs for Cursor agents.")
    parser.add_argument("--hook", action="store_true", help="Read Cursor preToolUse JSON from stdin.")
    parser.add_argument("--run", action="store_true", help="Execute a previously captured test command.")
    parser.add_argument("--cmd-file", type=Path, help="JSON file written by --hook.")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args(argv)

    if args.self_test:
        return self_test()
    if args.run:
        if not args.cmd_file:
            print("UE TEST WRAP: --cmd-file is required with --run", file=sys.stderr)
            return 1
        return run_main(args.cmd_file)
    return hook_main()


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # noqa: BLE001 - hook must fail open
        if "--run" in sys.argv:
            print(f"UE TEST WRAP: {exc}", file=sys.stderr)
            raise
        emit({"permission": "allow"})
        raise SystemExit(0)
