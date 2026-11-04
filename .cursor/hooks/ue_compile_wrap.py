#!/usr/bin/env python3
"""Cursor preToolUse hook: wrap Unreal/MSBuild compiles and keep only a summary.

preToolUse cannot replace a Shell tool's output after the fact. The only way to
keep the raw compiler log out of the model context is to rewrite the command so
the process the agent actually runs prints a short summary instead of UBT/MSVC
spam. The full log is still written to disk as an escape hatch.

Usage:
  python .cursor/hooks/ue_compile_wrap.py --hook
  python .cursor/hooks/ue_compile_wrap.py --run --cmd-file <path>
  python .cursor/hooks/ue_compile_wrap.py --self-test
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
MAX_TAIL_LINES = 12
MAX_LOG_BYTES = 32 * 1024 * 1024
START_BANNER = "COMPILE WRAP: running compiler; model will receive a summary, not the raw log."

# Substrings are intentional: agent commands quote paths with spaces
# (`& "C:\Program Files\...\Build.bat" ...`) which is easy to miss with a path regex.
COMPILE_MARKERS = (
    "build.bat",
    "rebuild.bat",
    "runubt.bat",
    "unrealbuildtool.exe",
    "unrealbuildtool.dll",
    "msbuild.exe",
)
COMPILE_PATTERNS = (
    re.compile(r'(?i)(?:^|[\s"\'\\/])UnrealBuildTool(?:\s|"|\'|$)'),
    re.compile(r'(?i)(?:^|[\s"\'\\/])MSBuild(?:\s|/|-|"|\'|$)'),
    re.compile(r'(?i)\bdotnet\s+(?:build|msbuild)\b'),
)

# First token is inspecting/editing, not compiling. Avoid wrapping `Get-Content Build.bat`.
VIEWER_PREFIX = re.compile(
    r'(?i)^\s*(?:&\s*)?(?:'
    r'Get-Content|Get-Item|Get-ChildItem|Get-Command|Get-Help|'
    r'Select-String|Out-File|Set-Content|'
    r'gc\b|cat\b|type\b|more\b|less\b|tail\b|head\b|'
    r'findstr\b|find\b|rg\b|grep\b|where(?:\.exe)?|'
    r'notepad(?:\+\+)?|code\b|cursor\b|vim\b|emacs\b|'
    r'dir\b|ls\b|echo\b|Write-Output|Write-Host'
    r')\b'
)

MSVC_ERROR = re.compile(
    r'(?i)^(?P<file>.+?)\((?P<line>\d+)(?:,\d+)?\):\s+(?:fatal\s+)?error\s+(?P<code>[A-Z]+\d+)\s*:\s*(?P<msg>.*)$'
)
MSVC_WARNING = re.compile(
    r'(?i)^(?P<file>.+?)\((?P<line>\d+)(?:,\d+)?\):\s+warning\s+(?P<code>[A-Z]+\d+)\s*:\s*(?P<msg>.*)$'
)
CLANG_ERROR = re.compile(
    r'^(?P<file>.+?):(?P<line>\d+)(?::\d+)?:\s+error:\s+(?P<msg>.*)$'
)
CLANG_WARNING = re.compile(
    r'^(?P<file>.+?):(?P<line>\d+)(?::\d+)?:\s+warning:\s+(?P<msg>.*)$'
)
GENERIC_ERROR = re.compile(
    r'(?i)(?:\bfatal\s+error\b|\berror\s+[A-Z]{1,5}\d+\b|\berror\s+LNK\d+\b|(?:^|\s)error\s*:|:\s+error\s*:|^\s*(?:ERROR|Error):)'
)
GENERIC_WARNING = re.compile(
    r'(?i)(?:\bwarning\s+[A-Z]{1,5}\d+\b|(?:^|\s)warning\s*:|:\s+warning\s*:|^\s*(?:WARNING|Warning):)'
)
SUCCESS_LINE = re.compile(r'(?i)\b(Build succeeded|Result:\s*Succeeded|\[-\]\s*Compile\s+result:\s*Succeeded)\b')
FAIL_LINE = re.compile(r'(?i)\b(Build FAILED|Result:\s*Failed|COMPILE FAILED)\b')
PROGRESS_LINE = re.compile(r'(?i)^\s*(?:\[\d+\s*/\s*\d+\]|@progress|Compiling\s+\.\.\.|Creating makefile)')
ZERO_ERRORS = re.compile(r'(?i)\b0\s+Error\(s\)\b')


def emit(payload: dict[str, Any]) -> None:
    sys.stdout.write(json.dumps(payload, ensure_ascii=False) + "\n")
    sys.stdout.flush()


def allow() -> int:
    emit({"permission": "allow"})
    return 0


def wrap_state_dir() -> Path:
    root = Path(os.environ.get("LOCALAPPDATA") or tempfile.gettempdir()) / "cursor-compile-wrap"
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


def first_token(command: str) -> str:
    stripped = command.strip()
    stripped = re.sub(r'^\s*&\s*', '', stripped)
    if stripped.startswith('"'):
        end = stripped.find('"', 1)
        return stripped[1:end] if end != -1 else stripped
    return stripped.split()[0] if stripped else ""


def is_already_wrapped(command: str) -> bool:
    return (
        "compile_wrap.py" in command
        or "CURSOR_COMPILE_WRAP" in command
        or "go_test_wrap.py" in command
        or "CURSOR_GO_TEST_WRAP" in command
    )


def is_compile_command(command: str) -> bool:
    if not command or not command.strip():
        return False
    if is_already_wrapped(command):
        return False
    # UE test runs are handled by ue_test_wrap.py, even if the command also mentions Build.bat.
    if "ue_test_wrap.py" in command or "CURSOR_UE_TEST_WRAP" in command:
        return False
    if re.search(r"(?i)\bAutomation\s+Run(?:Tests|All)\b", command):
        return False
    if VIEWER_PREFIX.match(command):
        return False
    lowered = command.lower()
    marked = any(marker in lowered for marker in COMPILE_MARKERS) or any(
        pattern.search(command) for pattern in COMPILE_PATTERNS
    )
    if not marked:
        return False
    token = first_token(command).lower().replace("/", "\\")
    if token.endswith(("unrealeditor.exe", "unrealeditor", "devenv.exe", "rider64.exe")):
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


def classify_line(line: str) -> str | None:
    stripped = line.strip()
    if not stripped or PROGRESS_LINE.match(stripped) or ZERO_ERRORS.search(stripped):
        return None
    if MSVC_ERROR.match(stripped) or CLANG_ERROR.match(stripped):
        return "error"
    if MSVC_WARNING.match(stripped) or CLANG_WARNING.match(stripped):
        return "warning"
    if FAIL_LINE.search(stripped):
        return "fail_marker"
    if SUCCESS_LINE.search(stripped):
        return "success_marker"
    if GENERIC_ERROR.search(stripped) and "0 Error" not in stripped:
        # Avoid treating "error handling" / "no error" as compiler errors.
        if re.search(r'(?i)\b(error handling|no error|if error|on error)\b', stripped):
            return None
        return "error"
    if GENERIC_WARNING.search(stripped):
        return "warning"
    return None


def format_summary(
    *,
    command: str,
    exit_code: int,
    duration_sec: float,
    errors: list[str],
    warnings: list[str],
    warning_count: int,
    fail_marker: bool,
    success_marker: bool,
    tail: Iterable[str],
    log_path: Path,
    raw_lines: int,
) -> str:
    failed = exit_code != 0 or fail_marker or bool(errors)
    if exit_code == 0 and errors:
        # MSVC can print errors then still return 0 in odd wrapper cases.
        failed = True
    status = "FAILED" if failed else "SUCCEEDED"
    shown_warnings = warnings[:MAX_WARNINGS_SHOWN]
    omitted_errors = max(0, len(errors) - MAX_ERRORS)
    omitted_warnings = max(0, warning_count - len(shown_warnings))

    lines = [
        "=== COMPILE SUMMARY (raw compiler log stripped to save context) ===",
        f"status: {status}",
        f"exit_code: {exit_code}",
        f"duration_sec: {duration_sec:.1f}",
        f"errors: {len(errors)}",
        f"warnings: {warning_count}",
        f"raw_log_lines: {raw_lines}",
        f"raw_log: {log_path}",
        f"original_command: {truncate(command, 300)}",
        "",
        "Use this summary as the compile result. Do not re-run the compiler just to inspect errors.",
        "Open the cited source files. Read raw_log only if a file/line is missing here.",
    ]

    if errors:
        lines.append("")
        lines.append("ERRORS:")
        for index, error in enumerate(errors[:MAX_ERRORS], start=1):
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
    if failed and not errors and tail_list:
        lines.append("")
        lines.append("TAIL (no structured errors parsed; last compiler output):")
        lines.extend(f"| {item}" for item in tail_list)

    if success_marker and not failed:
        lines.append("")
        lines.append("compiler reported success")

    lines.append("=== END COMPILE SUMMARY ===")
    return "\n".join(lines) + "\n"


def error_fingerprint(line: str) -> str:
    stripped = line.strip()
    msvc = MSVC_ERROR.match(stripped)
    if msvc:
        return f"{msvc.group('file').lower()}|{msvc.group('line')}|{msvc.group('code')}"
    clang = CLANG_ERROR.match(stripped)
    if clang:
        return f"{clang.group('file').lower()}|{clang.group('line')}|{clang.group('msg')}"
    return stripped


def run_compile(command: str, cwd: str | None, log_path: Path) -> tuple[int, float, str]:
    errors: list[str] = []
    seen_errors: set[str] = set()
    error_total = 0
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
        "env": {**os.environ, "CURSOR_COMPILE_WRAP": "1"},
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

            kind = classify_line(line)
            stripped = line.rstrip("\n")
            if kind != "error" and not PROGRESS_LINE.match(stripped):
                tail.append(stripped)
            if kind == "error":
                error_total += 1
                key = error_fingerprint(stripped)
                if key not in seen_errors:
                    seen_errors.add(key)
                    errors.append(stripped)
            elif kind == "warning":
                warning_count += 1
                if len(warnings) < MAX_WARNINGS_SHOWN:
                    warnings.append(stripped)
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
        warnings=warnings,
        warning_count=warning_count,
        fail_marker=fail_marker or error_total > 0,
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
    if not is_compile_command(command):
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
                "Compile command wrapped: you will receive a structured error summary "
                "instead of the raw UBT/MSBuild log. Full log is saved on disk."
            ),
        }
    )
    return 0


def run_main(cmd_file: Path) -> int:
    try:
        payload = json.loads(cmd_file.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"COMPILE WRAP: failed to read command file {cmd_file}: {exc}", file=sys.stderr)
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
        print("COMPILE WRAP: empty command", file=sys.stderr)
        return 1

    stamp = time.strftime("%Y%m%d-%H%M%S")
    log_path = wrap_state_dir() / "logs" / f"{stamp}-{uuid.uuid4().hex[:8]}.log"
    exit_code, _, summary = run_compile(command, cwd, log_path)
    sys.stdout.write(summary)
    sys.stdout.flush()
    return exit_code


def self_test() -> int:
    compile_cases = [
        r'Engine\Build\BatchFiles\Build.bat SDKsTestEditor Win64 Development',
        r'& "C:\UE\Engine\Build\BatchFiles\Build.bat" GameEditor Win64 Development',
        r'& "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" SDKsTestEditor Win64 Development -Project="C:\Kamil\UEProjects\SDKsTest\SDKsTest.uproject" -WaitMutex',
        r'UnrealBuildTool.exe SDKsTestEditor Win64 Development',
        r'"C:\Program Files\Microsoft Visual Studio\2022\MSBuild\Current\Bin\MSBuild.exe" Game.sln /t:Build',
        r'dotnet build Plugins\Foo\Foo.csproj -c Release',
        r'MSBuild Game.sln -p:Configuration=Development',
    ]
    non_compile_cases = [
        "git status",
        "Get-Content Build.bat",
        r'type C:\log\UnrealBuildTool.log',
        "rg error Intermediate\\Build",
        "UnrealEditor.exe SDKsTest.uproject",
        f'{sys.executable} {SCRIPT_PATH} --run --cmd-file x.json',
        "dir",
        "echo Build.bat",
    ]
    failed = 0
    for command in compile_cases:
        if not is_compile_command(command):
            print(f"FAIL expected compile: {command}")
            failed += 1
    for command in non_compile_cases:
        if is_compile_command(command):
            print(f"FAIL expected skip: {command}")
            failed += 1

    sample = [
        r"C:\proj\PubnubClient.cpp(2574): error C2065: 'Foo': undeclared identifier",
        r"C:\proj\PubnubClient.cpp(88): warning C4100: 'Bar': unreferenced parameter",
        "[3/10] Compile PubnubClient.cpp",
        "Build FAILED.",
    ]
    kinds = [classify_line(line) for line in sample]
    expected = ["error", "warning", None, "fail_marker"]
    if kinds != expected:
        print(f"FAIL classify_line: {kinds} != {expected}")
        failed += 1

    if failed:
        print(f"self-test failed: {failed} check(s)")
        return 1
    print("self-test passed")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Wrap Unreal/MSBuild compiles for Cursor agents.")
    parser.add_argument("--hook", action="store_true", help="Read Cursor preToolUse JSON from stdin.")
    parser.add_argument("--run", action="store_true", help="Execute a previously captured compile command.")
    parser.add_argument("--cmd-file", type=Path, help="JSON file written by --hook.")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args(argv)

    if args.self_test:
        return self_test()
    if args.run:
        if not args.cmd_file:
            print("COMPILE WRAP: --cmd-file is required with --run", file=sys.stderr)
            return 1
        return run_main(args.cmd_file)
    return hook_main()


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # noqa: BLE001 - hook must fail open
        if "--run" in sys.argv:
            print(f"COMPILE WRAP: {exc}", file=sys.stderr)
            raise
        emit({"permission": "allow"})
        raise SystemExit(0)
