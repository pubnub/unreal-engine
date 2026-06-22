#!/usr/bin/env python3
"""
Prepare the Pubnub Unreal SDK for release.

Updates:
  - PubnubLibrary.uplugin: increments Version (VersionName is unchanged)
  - PubnubLibraryVersion.h: sets PUBNUB_LIBRARY_VERSION_{MAJOR,MINOR,PATCH}
    and optionally PUBNUB_C_CORE_VERSION
  - .pubnub.yml: sets top-level version (changelog entries are unchanged)

Run from any directory; paths are resolved from script location.

CLI examples:
  python prepare_for_release.py --release-version 2.0.6
  python prepare_for_release.py -r 2.0.6 --c-core-version 7.2.4
  python prepare_for_release.py -r 2.0.6 --c-core-version
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


_VERSION_PATTERN = re.compile(r"^(\d+)\.(\d+)\.(\d+)$")


def _parse_semver(label: str, value: str) -> tuple[int, int, int]:
    match = _VERSION_PATTERN.match(value.strip())
    if not match:
        raise ValueError(f"{label} must be in MAJOR.MINOR.PATCH format (e.g. 2.0.5), got: {value!r}")
    return int(match.group(1)), int(match.group(2)), int(match.group(3))


def _prompt(label: str, *, required: bool = True) -> str:
    while True:
        value = input(f"{label}: ").strip()
        if value or not required:
            return value
        print("  Value is required.")


def _resolve_input(label: str, arg_value: str | None, *, required: bool = True) -> str:
    if arg_value is not None:
        return arg_value.strip()
    return _prompt(label, required=required)


def _build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Prepare the Pubnub Unreal SDK for release.")
    parser.add_argument(
        "-r",
        "--release-version",
        help="Release version in MAJOR.MINOR.PATCH format. Prompts when omitted.",
    )
    parser.add_argument(
        "-c",
        "--c-core-version",
        nargs="?",
        const="",
        default=None,
        help=(
            "C-Core version in MAJOR.MINOR.PATCH format. "
            "Omit the flag to prompt; pass the flag alone to skip updating."
        ),
    )
    return parser


def _update_uplugin_version(uplugin_path: Path) -> int:
    with uplugin_path.open("r", encoding="utf-8") as f:
        uplugin = json.load(f)

    old_version = uplugin.get("Version")
    if not isinstance(old_version, int):
        raise SystemExit(f"Unexpected Version field in {uplugin_path}: {old_version!r}")

    new_version = old_version + 1
    uplugin["Version"] = new_version

    with uplugin_path.open("w", encoding="utf-8", newline="\n") as f:
        json.dump(uplugin, f, indent="\t")
        f.write("\n")

    return new_version


def _update_version_header(
    header_path: Path,
    major: int,
    minor: int,
    patch: int,
    c_core_version: str | None,
) -> None:
    content = header_path.read_text(encoding="utf-8")

    content, count = re.subn(
        r"(#define PUBNUB_LIBRARY_VERSION_MAJOR )\d+",
        rf"\g<1>{major}",
        content,
        count=1,
    )
    if count != 1:
        raise SystemExit(f"Could not update PUBNUB_LIBRARY_VERSION_MAJOR in {header_path}")

    content, count = re.subn(
        r"(#define PUBNUB_LIBRARY_VERSION_MINOR )\d+",
        rf"\g<1>{minor}",
        content,
        count=1,
    )
    if count != 1:
        raise SystemExit(f"Could not update PUBNUB_LIBRARY_VERSION_MINOR in {header_path}")

    content, count = re.subn(
        r"(#define PUBNUB_LIBRARY_VERSION_PATCH )\d+",
        rf"\g<1>{patch}",
        content,
        count=1,
    )
    if count != 1:
        raise SystemExit(f"Could not update PUBNUB_LIBRARY_VERSION_PATCH in {header_path}")

    if c_core_version is not None:
        content, count = re.subn(
            r'(#define PUBNUB_C_CORE_VERSION ")[^"]*(")',
            rf"\g<1>{c_core_version}\g<2>",
            content,
            count=1,
        )
        if count != 1:
            raise SystemExit(f"Could not update PUBNUB_C_CORE_VERSION in {header_path}")

    header_path.write_text(content, encoding="utf-8", newline="\n")


def _update_pubnub_yml_version(pubnub_yml_path: Path, release_version: str) -> None:
    content = pubnub_yml_path.read_text(encoding="utf-8")

    # Top-level key is unindented; changelog entries use "    version:".
    content, count = re.subn(
        r"^version: .+$",
        f"version: {release_version}",
        content,
        count=1,
        flags=re.MULTILINE,
    )
    if count != 1:
        raise SystemExit(f"Could not update top-level version in {pubnub_yml_path}")

    pubnub_yml_path.write_text(content, encoding="utf-8", newline="\n")


def main(argv: list[str] | None = None) -> None:
    args = _build_arg_parser().parse_args(argv)

    script_dir = Path(__file__).resolve().parent
    plugin_root = script_dir.parent
    uplugin_path = plugin_root / "PubnubLibrary.uplugin"
    header_path = plugin_root / "Source" / "PubnubLibrary" / "Public" / "PubnubLibraryVersion.h"
    pubnub_yml_path = plugin_root / ".pubnub.yml"

    if not uplugin_path.is_file():
        raise SystemExit(f"Plugin descriptor not found: {uplugin_path}")
    if not header_path.is_file():
        raise SystemExit(f"Version header not found: {header_path}")
    if not pubnub_yml_path.is_file():
        raise SystemExit(f"PubNub manifest not found: {pubnub_yml_path}")

    print("Prepare Pubnub Unreal SDK for release\n")

    release_version = _resolve_input("Release version (MAJOR.MINOR.PATCH)", args.release_version)
    try:
        major, minor, patch = _parse_semver("Release version", release_version)
    except ValueError as exc:
        raise SystemExit(str(exc)) from exc

    c_core_input = _resolve_input(
        "C-Core version (MAJOR.MINOR.PATCH, leave empty to skip)",
        args.c_core_version,
        required=False,
    )
    c_core_version: str | None = None
    if c_core_input:
        try:
            c_major, c_minor, c_patch = _parse_semver("C-Core version", c_core_input)
        except ValueError as exc:
            raise SystemExit(str(exc)) from exc
        c_core_version = f"{c_major}.{c_minor}.{c_patch}"

    release_version_str = f"{major}.{minor}.{patch}"

    new_uplugin_version = _update_uplugin_version(uplugin_path)
    _update_version_header(header_path, major, minor, patch, c_core_version)
    _update_pubnub_yml_version(pubnub_yml_path, release_version_str)

    print("\nUpdated files:")
    print(f"  {uplugin_path}")
    print(f"    Version -> {new_uplugin_version} (VersionName unchanged)")
    print(f"  {pubnub_yml_path}")
    print(f"    version -> {release_version_str} (changelog unchanged)")
    print(f"  {header_path}")
    print(f"    PUBNUB_LIBRARY_VERSION -> {release_version_str}")
    if c_core_version is not None:
        print(f"    PUBNUB_C_CORE_VERSION -> {c_core_version}")
    else:
        print("    PUBNUB_C_CORE_VERSION -> unchanged")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nAborted.", file=sys.stderr)
        sys.exit(130)
