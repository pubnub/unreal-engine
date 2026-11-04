---
name: ue-plugin-compile
description: >-
  Verifies Unreal plugin compilation with RunUAT BuildPlugin for the PubNub
  core SDK, chat SDK, or gaming SDK on a named engine version. Use when the
  user asks to verify plugin compilation, package a plugin with RunUAT, or
  invokes ue-plugin-compile with a plugin and a UE version.
---

# UE plugin compile

Verify one plugin on one installed engine. The project hook `.cursor/hooks/ue_plugin_compile_wrap.py` rewrites `RunUAT BuildPlugin` and returns a short error summary. Run the `RunUAT` command itself. Do not call the wrap script.

## Select the plugin

| User says | `-Plugin` | `-Package` |
|---|---|---|
| core, core sdk, ue core sdk, Pubnub library | `C:\Kamil\UEProjects\SDKsTest\Plugins\Pubnub\PubnubLibrary.uplugin` | `C:\Kamil\PluginPackage\PubnubLibrary` |
| chat, chat sdk, Pubnub chat | `C:\Kamil\UEProjects\SDKsTest\Plugins\PubnubChat\PubnubChat.uplugin` | `C:\Kamil\PluginPackage\PubnubChat` |
| gaming, gaming sdk, fab | `C:\Kamil\UEProjects\SDKsTest\Plugins\PubnubGamingSDK\PubnubLibrary.uplugin` | `C:\Kamil\PluginPackage\PubnubGamingSDK` |

If the plugin or the engine version is missing or ambiguous, ask. Do not guess.

Map `UE 5.6`, `5.6`, and `UE_5.6` to folder `UE_5.6`. A patch number still uses major.minor (`5.6.1` → `UE_5.6`).

## Check the engine first

The version is supported only when this file exists:

`C:\Program Files\Epic Games\UE_<version>\Engine\Build\BatchFiles\RunUAT.bat`

If it does not exist, say that this UE version is not supported and do not run anything.

Also require the selected `.uplugin` to exist. If the gaming plugin is missing, say `Plugins/PubnubGamingSDK` is not present (it is produced by `python Plugins/Pubnub/Scripts/merge_into_fab_sdk.py`) and do not run `BuildPlugin`.

## Command

PowerShell:

```powershell
& "C:\Program Files\Epic Games\UE_<version>\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="<plugin>" -Package="<package>" -Rocket
```

Example: core SDK on UE 5.6.

```powershell
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="C:\Kamil\UEProjects\SDKsTest\Plugins\Pubnub\PubnubLibrary.uplugin" -Package="C:\Kamil\PluginPackage\PubnubLibrary" -Rocket
```

Set `block_until_ms` to at least `1800000`. If the command is backgrounded, wait until it exits. Do not start a second `BuildPlugin` while one is running.

The shell result is the hook summary, between `PLUGIN COMPILE SUMMARY` and `END PLUGIN COMPILE SUMMARY`. Use that summary as the compile result. Open cited source files. Read `raw_log` only when a file or line is missing from the summary.

## Fix loop

Budget: **5** `BuildPlugin` runs for this request. The engine check does not count.

1. Run the command.
2. `status: SUCCEEDED` → report plugin, engine, and attempt count. Stop.
3. `status: FAILED` with errors in this repo's plugin sources → fix those sources, then run the same command again. Do not edit the engine install or `C:\Kamil\PluginPackage`.
4. Keep the engine's default C++ standard. Do not set `CppStandard`, `/std:c++20`, or any other language-standard override in `Build.cs` or the compile command. Change the failing code so it compiles with that default. Designated initializers are the usual case: rewrite them into a form the default standard accepts.
5. Rerun only after a source change that addresses the reported errors. If the failure is environmental (missing component, missing platform toolchain) and there is no source change to make, report it and stop.
6. If attempt 5 is still `FAILED`, report the remaining errors from that summary and stop. Do not run a 6th time.
