#!/usr/bin/env python3
"""
Merge Pubnub and PubnubChat plugins into a single FAB-ready plugin (PubnubGamingSDK).

Output: Plugins/PubnubGamingSDK/
  - PubnubLibrary.uplugin (merged descriptor; no plugin dependency)
  - Source/ (PubnubLibrary, PubnubLibraryTests, ThirdParty, PubnubChatSDK, PubnubChatSDKTests)
  - Content/ (from PubnubChat)
  - LICENSE (from Pubnub, or PubnubChat if Pubnub has none)

Run from project root or any directory; paths are resolved from script location.
"""

from __future__ import annotations

import json
import shutil
from pathlib import Path


# Names to exclude when copying directory trees.
# Note: "Config" at plugin root is not copied (we only copy Source/ and Content/).
# Do not add "Config" here so that Source/.../Config (e.g. settings classes) is preserved.
_COPY_IGNORE = frozenset({
    "__pycache__", ".git", ".gitignore", ".pyc", ".pyo",
    "Intermediate", "Binaries", ".github",
    ".vs", ".idea", ".user", "*.sln", "*.suo",
})


def _should_ignore(name: str) -> bool:
    if name in _COPY_IGNORE:
        return True
    if name.endswith(".user") or name.endswith(".suo"):
        return True
    return False


def _ignore_func(_dir: Path, names: list[str]) -> list[str]:
    return [n for n in names if _should_ignore(n)]


def main() -> None:
    script_dir = Path(__file__).resolve().parent
    # Project root: Scripts -> Pubnub -> Plugins -> project root
    project_root = script_dir.parent.parent.parent
    pubnub_root = project_root / "Plugins" / "Pubnub"
    pubnub_chat_root = project_root / "Plugins" / "PubnubChat"
    out_root = project_root / "Plugins" / "PubnubGamingSDK"

    if not pubnub_root.is_dir():
        raise SystemExit(f"Pubnub plugin not found: {pubnub_root}")
    if not pubnub_chat_root.is_dir():
        raise SystemExit(f"PubnubChat plugin not found: {pubnub_chat_root}")

    pubnub_uplugin = pubnub_root / "PubnubLibrary.uplugin"
    pubnub_chat_uplugin = pubnub_chat_root / "PubnubChat.uplugin"
    if not pubnub_uplugin.is_file():
        raise SystemExit(f"Missing {pubnub_uplugin}")
    if not pubnub_chat_uplugin.is_file():
        raise SystemExit(f"Missing {pubnub_chat_uplugin}")

    # Load descriptors
    with open(pubnub_uplugin, encoding="utf-8") as f:
        pubnub_desc = json.load(f)
    with open(pubnub_chat_uplugin, encoding="utf-8") as f:
        chat_desc = json.load(f)

    # Build merged .uplugin: base on Pubnub (version unchanged), merge modules, drop Plugins
    merged = dict(pubnub_desc)
    merged.pop("Plugins", None)

    pubnub_modules = list(merged.get("Modules", []))
    chat_modules = list(chat_desc.get("Modules", []))
    merged["Modules"] = pubnub_modules + chat_modules

    # Create output (replace existing)
    if out_root.exists():
        shutil.rmtree(out_root)
    out_root.mkdir(parents=True)

    with open(out_root / "PubnubLibrary.uplugin", "w", encoding="utf-8") as f:
        json.dump(merged, f, indent="\t")

    # Copy Pubnub Source (copytree creates out_root/Source)
    shutil.copytree(
        pubnub_root / "Source",
        out_root / "Source",
        ignore=_ignore_func,
    )

    # Merge PubnubChat Source into same Source tree
    out_source = out_root / "Source"
    chat_source = pubnub_chat_root / "Source"
    for item in chat_source.iterdir():
        dst = out_source / item.name
        if item.is_dir():
            shutil.copytree(item, dst, ignore=_ignore_func)
        else:
            shutil.copy2(item, dst)

    # Copy Content from PubnubChat
    chat_content = pubnub_chat_root / "Content"
    if chat_content.is_dir():
        shutil.copytree(chat_content, out_root / "Content", ignore=_ignore_func)

    # Copy LICENSE (Pubnub first, then PubnubChat)
    for root in (pubnub_root, pubnub_chat_root):
        license_file = root / "LICENSE"
        if license_file.is_file():
            shutil.copy2(license_file, out_root / "LICENSE")
            break

    print(f"Done. PubnubGamingSDK at {out_root}")


if __name__ == "__main__":
    main()
