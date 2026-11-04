#!/usr/bin/env python3
"""Cursor preToolUse hook: wrap RunUAT BuildPlugin and keep only a summary.

preToolUse cannot replace a Shell tool's output after the fact. The command is
rewritten so the process the agent runs prints compiler and AutomationTool
errors instead of the raw RunUAT log. The full log is still written to disk.

Matches BuildPlugin only. Other RunUAT commands (BuildCookRun, and so on) are
left alone. Ordinary UBT/MSBuild compiles stay with ue_compile_wrap.py.

Usage:
  python .cursor/hooks/ue_plugin_compile_wrap.py --hook
  python .cursor/hooks/ue_plugin_compile_wrap.py --run --cmd-file <path>
  python .cursor/hooks/ue_plugin_compile_wrap.py --self-test
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
MAX_ERRORS = 40
MAX_WARNINGS_SHOWN = 8
MAX_LINE_CHARS = 400
MAX_TAIL_LINES = 16
MAX_LOG_BYTES = 32 * 1024 * 1024
START_BANNER = (
    "PLUGIN COMPILE WRAP: running RunUAT BuildPlugin; "
    "model will receive a summary, not the raw log."
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

MSVC_ERROR = re.compile(
    r"(?i)(?P<file>.+?)\((?P<line>\d+)(?:,\d+)?\):\s+(?:fatal\s+)?error\s+(?P<code>[A-Z]+\d+)\s*:\s*(?P<msg>.*)$"
)
MSVC_WARNING = re.compile(
    r"(?i)(?P<file>.+?)\((?P<line>\d+)(?:,\d+)?\):\s+warning\s+(?P<code>[A-Z]+\d+)\s*:\s*(?P<msg>.*)$"
)
UBT_PAREN_ERROR = re.compile(
    r"(?i)(?P<file>.+?)\((?P<line>\d+)(?:,\d+)?\):\s+(?:fatal\s+error|error)\s*:\s*(?P<msg>.*)$"
)
UBT_PAREN_WARNING = re.compile(
    r"(?i)(?P<file>.+?)\((?P<line>\d+)(?:,\d+)?\):\s+warning\s*:\s*(?P<msg>.*)$"
)
CLANG_ERROR = re.compile(
    r"(?i)(?P<file>.+?):(?P<line>\d+)(?::\d+)?:\s+(?:fatal\s+)?error\s*:\s*(?P<msg>.*)$"
)
CLANG_WARNING = re.compile(
    r"(?i)(?P<file>.+?):(?P<line>\d+)(?::\d+)?:\s+warning\s*:\s*(?P<msg>.*)$"
)
UAT_ERROR = re.compile(
    r"(?i)(?:^|\s)(?:\*{1,3}\s*)?(?:Error|ERROR)\s*:\s*(?P<msg>\S.*)$"
)
UAT_WARNING = re.compile(
    r"(?i)(?:^|\s)Warning\s*:\s*(?P<msg>\S.*)$"
)
SUCCESS_LINE = re.compile(
    r"(?i)\b(BUILD SUCCESSFUL|AutomationTool exiting with ExitCode=0|Result:\s*Succeeded)\b"
)
FAIL_LINE = re.compile(
    r"(?i)\b(BUILD FAILED|Result:\s*Failed|Plugin .* failed to compile|ExitCode=(?!0\b)\d+)\b"
)
PROGRESS_LINE = re.compile(
    r"(?i)^\s*(?:\[\d+\s*/\s*\d+\]|@progress|Compiling\s+\.\.\.|Creating makefile|Parsing headers)"
)
ZERO_ERRORS = re.compile(r"(?i)\b0\s+Error\(s\)\b")
NOISE_ERROR = re.compile(
    r"(?i)\b(error handling|no error|if error|on error|error_code|errorcode)\b"
)
WIN_PATH = re.compile(r"([A-Za-z]:\\.+)$")
SOURCE_FILE = re.compile(
    r"(\S+\.(?:cpp|h|hpp|c|cc|inl|cs|usf|ush|uplugin|uproject|Build\.cs|Target\.cs))$",
    re.IGNORECASE,
)


def emit(payload: dict[str, Any]) -> None:
    sys.stdout.write(json.dumps(payload, ensure_ascii=False) + "\n")
    sys.stdout.flush()


def allow() -> int:
    emit({"permission": "allow"})
    return 0


def wrap_state_dir() -> Path:
    root = Path(os.environ.get("LOCALAPPDATA") or tempfile.gettempdir()) / "cursor-plugin-compile-wrap"
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
        "plugin_compile_wrap.py" in command
        or "CURSOR_PLUGIN_COMPILE_WRAP" in command
        or "compile_wrap.py" in command
        or "CURSOR_COMPILE_WRAP" in command
        or "ue_test_wrap.py" in command
        or "CURSOR_UE_TEST_WRAP" in command
        or "go_test_wrap.py" in command
        or "CURSOR_GO_TEST_WRAP" in command
    )


def is_plugin_compile_command(command: str) -> bool:
    if not command or not command.strip():
        return False
    if is_already_wrapped(command):
        return False
    if VIEWER_PREFIX.match(command):
        return False
    if "runuat" not in command.lower():
        return False
    return re.search(r"(?i)\bBuildPlugin\b", command) is not None


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


def clean_file(file_text: str) -> str:
    stripped = file_text.strip().strip('"').strip("'")
    win = WIN_PATH.search(stripped)
    if win:
        return win.group(1).strip()
    source = SOURCE_FILE.search(stripped)
    if source:
        return source.group(1)
    return stripped


def format_compiler_hit(match: re.Match[str], *, kind: str) -> str:
    code = match.groupdict().get("code")
    code_text = f" {code}" if code else ""
    label = "error" if kind == "error" else "warning"
    return f"{clean_file(match.group('file'))}({match.group('line')}): {label}{code_text}: {match.group('msg').strip()}"


def classify_line(line: str) -> tuple[str, str] | None:
    stripped = line.strip()
    if not stripped or PROGRESS_LINE.match(stripped) or ZERO_ERRORS.search(stripped):
        return None
    if NOISE_ERROR.search(stripped) and not MSVC_ERROR.search(stripped):
        return None

    msvc_error = MSVC_ERROR.search(stripped)
    if msvc_error and not re.match(r"(?i)warning\b", msvc_error.group("code") or ""):
        return "error", format_compiler_hit(msvc_error, kind="error")

    msvc_warning = MSVC_WARNING.search(stripped)
    if msvc_warning:
        return "warning", format_compiler_hit(msvc_warning, kind="warning")

    ubt_error = UBT_PAREN_ERROR.search(stripped)
    if ubt_error:
        return "error", format_compiler_hit(ubt_error, kind="error")

    ubt_warning = UBT_PAREN_WARNING.search(stripped)
    if ubt_warning:
        return "warning", format_compiler_hit(ubt_warning, kind="warning")

    # Clang-style "file:line: error:" is only used when the line is not an MSVC/UBT diagnostic.
    if not msvc_error:
        clang_error = CLANG_ERROR.search(stripped)
        if clang_error and "warning" not in clang_error.group("file").lower().split(":")[-1]:
            # Avoid treating "0 Error(s)" and UAT "Error:" lines as clang hits.
            file_text = clang_error.group("file")
            if not re.search(r"(?i)(?:^|\s)error\s*$", file_text) and "error(s)" not in file_text.lower():
                return "error", format_compiler_hit(clang_error, kind="error")

    if not msvc_warning:
        clang_warning = CLANG_WARNING.search(stripped)
        if clang_warning and not re.search(r"(?i)(?:^|\s)warning\s*$", clang_warning.group("file")):
            return "warning", format_compiler_hit(clang_warning, kind="warning")

    uat_error = UAT_ERROR.search(stripped)
    if uat_error and not ZERO_ERRORS.search(uat_error.group("msg")):
        return "tool_error", truncate(uat_error.group("msg"), MAX_LINE_CHARS)

    if FAIL_LINE.search(stripped) and not SUCCESS_LINE.search(stripped):
        return "fail_marker", truncate(stripped)

    if SUCCESS_LINE.search(stripped):
        return "success_marker", truncate(stripped)

    uat_warning = UAT_WARNING.search(stripped)
    if uat_warning:
        return "warning", truncate(uat_warning.group("msg"))
    return None


def format_summary(
    *,
    command: str,
    exit_code: int,
    duration_sec: float,
    errors: list[str],
    tool_errors: list[str],
    warnings: list[str],
    warning_count: int,
    fail_marker: bool,
    success_marker: bool,
    tail: Iterable[str],
    log_path: Path,
    raw_lines: int,
) -> str:
    failed = bool(errors) or exit_code != 0 or fail_marker
    if exit_code == 0 and success_marker and not errors:
        failed = False
    status = "FAILED" if failed else "SUCCEEDED"
    shown_errors = errors[:MAX_ERRORS]
    if not shown_errors and failed:
        shown_errors = tool_errors[:MAX_ERRORS]
    omitted_errors = max(0, (len(errors) if errors else len(tool_errors)) - len(shown_errors))
    shown_warnings = warnings[:MAX_WARNINGS_SHOWN]
    omitted_warnings = max(0, warning_count - len(shown_warnings))

    lines = [
        "=== PLUGIN COMPILE SUMMARY (raw RunUAT log stripped to save context) ===",
        f"status: {status}",
        f"exit_code: {exit_code}",
        f"duration_sec: {duration_sec:.1f}",
        f"errors: {len(errors) if errors else len(tool_errors) if failed else 0}",
        f"warnings: {warning_count}",
        f"raw_log_lines: {raw_lines}",
        f"raw_log: {log_path}",
        f"original_command: {truncate(command, 300)}",
        "",
        "Use this summary as the plugin compile result. Do not re-run BuildPlugin just to inspect errors.",
        "Open the cited source files. Read raw_log only if a file/line is missing here.",
    ]

    if shown_errors:
        lines.append("")
        lines.append("ERRORS:")
        for index, error in enumerate(shown_errors, start=1):
            lines.append(f"{index}. {truncate(error)}")
        if omitted_errors:
            lines.append(f"... {omitted_errors} more error lines omitted; see raw_log")

    if shown_warnings:
        lines.append("")
        lines.append(f"WARNINGS (first {len(shown_warnings)} of {warning_count}):")
        for index, warning in enumerate(shown_warnings, start=1):
            lines.append(f"{index}. {truncate(warning)}")
        if omitted_warnings:
            lines.append(f"... {omitted_warnings} more warnings omitted")

    tail_list = [truncate(item) for item in tail if item.strip()]
    if failed and not shown_errors and tail_list:
        lines.append("")
        lines.append("TAIL (no structured errors parsed; last RunUAT output):")
        lines.extend(f"| {item}" for item in tail_list)

    if success_marker and not failed:
        lines.append("")
        lines.append("RunUAT reported success")

    lines.append("=== END PLUGIN COMPILE SUMMARY ===")
    return "\n".join(lines) + "\n"


def error_fingerprint(line: str) -> str:
    return " ".join(line.split()).lower()


def run_compile(command: str, cwd: str | None, log_path: Path) -> tuple[int, float, str]:
    errors: list[str] = []
    tool_errors: list[str] = []
    seen: set[str] = set()
    warnings: list[str] = []
    warning_count = 0
    fail_marker = False
    success_marker = False
    tail: deque[str] = deque(maxlen=MAX_TAIL_LINES)
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
        "env": {**os.environ, "CURSOR_PLUGIN_COMPILE_WRAP": "1"},
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

            classified = classify_line(line)
            stripped = line.rstrip("\n")
            if classified is None or classified[0] not in {"error", "tool_error"}:
                if not PROGRESS_LINE.match(stripped):
                    tail.append(stripped)
            if classified is None:
                continue
            kind, display = classified
            key = error_fingerprint(display)
            if kind == "error":
                if key not in seen:
                    seen.add(key)
                    errors.append(display)
            elif kind == "tool_error":
                if key not in seen:
                    seen.add(key)
                    tool_errors.append(display)
                if FAIL_LINE.search(display):
                    fail_marker = True
            elif kind == "warning":
                warning_count += 1
                if len(warnings) < MAX_WARNINGS_SHOWN and key not in seen:
                    seen.add(key)
                    warnings.append(display)
            elif kind == "fail_marker":
                fail_marker = True
            elif kind == "success_marker":
                success_marker = True

    exit_code = proc.wait()
    duration = time.perf_counter() - started
    summary = format_summary(
        command=command,
        exit_code=exit_code,
        duration_sec=duration,
        errors=errors,
        tool_errors=tool_errors,
        warnings=warnings,
        warning_count=warning_count,
        fail_marker=fail_marker,
        success_marker=success_marker,
        tail=tail,
        log_path=log_path,
        raw_lines=raw_lines,
    )
    return exit_code, duration, summary


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
    if not is_plugin_compile_command(command):
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
                "BuildPlugin command wrapped: you will receive a structured error summary "
                "instead of the raw RunUAT log. Full log is saved on disk."
            ),
        }
    )
    return 0


def run_main(cmd_file: Path) -> int:
    try:
        payload = json.loads(cmd_file.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"PLUGIN COMPILE WRAP: failed to read command file {cmd_file}: {exc}", file=sys.stderr)
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
        print("PLUGIN COMPILE WRAP: empty command", file=sys.stderr)
        return 1

    stamp = time.strftime("%Y%m%d-%H%M%S")
    log_path = wrap_state_dir() / "logs" / f"{stamp}-{uuid.uuid4().hex[:8]}.log"
    exit_code, _, summary = run_compile(command, cwd, log_path)
    sys.stdout.write(summary)
    sys.stdout.flush()
    return exit_code


def self_test() -> int:
    compile_cases = [
        r'C:\Program Files\Epic Games\UE_5.2\Engine\Build\BatchFiles\RunUAT.bat BuildPlugin -Plugin="C:\Kamil\UEProjects\SDKsTest\Plugins\Pubnub\PubnubLibrary.uplugin" -Package="C:\Kamil\PluginPackage\PubnubLibrary" -Rocket',
        r'& "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="C:\Kamil\UEProjects\SDKsTest\Plugins\PubnubChat\PubnubChat.uplugin" -Package="C:\Kamil\PluginPackage\PubnubChat" -Rocket',
        r'& "C:\Program Files\Epic Games\UE_5.1\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="C:\Kamil\UEProjects\SDKsTest\Plugins\PubnubGamingSDK\PubnubLibrary.uplugin" -Package="C:\Kamil\PluginPackage\PubnubGamingSDK" -Rocket',
    ]
    skip_cases = [
        "git status",
        r'Get-Content "C:\Program Files\Epic Games\UE_5.2\Engine\Build\BatchFiles\RunUAT.bat"',
        r'& "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" SDKsTestEditor Win64 Development',
        r'& "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project=Game.uproject',
        f"{sys.executable} {SCRIPT_PATH} --run --cmd-file x.json",
        "echo BuildPlugin",
    ]
    failed = 0
    for command in compile_cases:
        if not is_plugin_compile_command(command):
            print(f"FAIL expected plugin compile: {command}")
            failed += 1
    for command in skip_cases:
        if is_plugin_compile_command(command):
            print(f"FAIL expected skip: {command}")
            failed += 1

    sample = [
        r"UATHelper: Packaging (Win64): C:\proj\PubnubClient.cpp(2574): error C2065: 'Foo': undeclared identifier",
        r"C:\proj\Foo.cpp(10,2): error: use of undeclared identifier",
        r"C:\proj\PubnubClient.cpp(88): warning C4100: 'Bar': unreferenced parameter",
        "Parsing headers for PubnubLibrary",
        "Error: Plugin 'PubnubLibrary' failed to compile.",
        "BUILD FAILED",
        "AutomationTool exiting with ExitCode=0",
        "BUILD SUCCESSFUL",
        "0 Error(s)",
    ]
    kinds = [None if item is None else item[0] for item in (classify_line(line) for line in sample)]
    expected = ["error", "error", "warning", None, "tool_error", "fail_marker", "success_marker", "success_marker", None]
    if kinds != expected:
        print(f"FAIL classify_line: {kinds} != {expected}")
        failed += 1

    cleaned = classify_line(sample[0])
    if cleaned is None or not cleaned[1].startswith(r"C:\proj\PubnubClient.cpp(2574): error C2065:"):
        print(f"FAIL cleaned diagnostic: {cleaned}")
        failed += 1

    summary = format_summary(
        command=compile_cases[0],
        exit_code=1,
        duration_sec=1.2,
        errors=[cleaned[1]] if cleaned else [],
        tool_errors=["Plugin 'PubnubLibrary' failed to compile."],
        warnings=[],
        warning_count=0,
        fail_marker=True,
        success_marker=False,
        tail=[],
        log_path=Path("raw.log"),
        raw_lines=10,
    )
    if "status: FAILED" not in summary or "error C2065" not in summary:
        print("FAIL summary missing failure details")
        failed += 1

    success = format_summary(
        command=compile_cases[0],
        exit_code=0,
        duration_sec=1.0,
        errors=[],
        tool_errors=["stale tool error that must not fail a green build"],
        warnings=[],
        warning_count=0,
        fail_marker=False,
        success_marker=True,
        tail=[],
        log_path=Path("raw.log"),
        raw_lines=4,
    )
    if "status: SUCCEEDED" not in success or "stale tool error" in success:
        print("FAIL success summary")
        failed += 1

    if failed:
        print(f"self-test failed: {failed} check(s)")
        return 1
    print("self-test passed")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Wrap RunUAT BuildPlugin for Cursor agents.")
    parser.add_argument("--hook", action="store_true", help="Read Cursor preToolUse JSON from stdin.")
    parser.add_argument("--run", action="store_true", help="Execute a previously captured BuildPlugin command.")
    parser.add_argument("--cmd-file", type=Path, help="JSON file written by --hook.")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args(argv)

    if args.self_test:
        return self_test()
    if args.run:
        if not args.cmd_file:
            print("PLUGIN COMPILE WRAP: --cmd-file is required with --run", file=sys.stderr)
            return 1
        return run_main(args.cmd_file)
    return hook_main()


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # noqa: BLE001 - hook must fail open
        if "--run" in sys.argv:
            print(f"PLUGIN COMPILE WRAP: {exc}", file=sys.stderr)
            raise
        emit({"permission": "allow"})
        raise SystemExit(0)
