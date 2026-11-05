#!/usr/bin/env bash
#
# Run the UE 5.5 Clang static analyzer on PubnubLibrary and fail when it reports an issue.
#
# The plugin repository has no .uproject, so this creates a temporary editor host,
# links this checkout in as Plugins/PubnubLibrary, and analyzes only PubnubLibrary.
# PubnubLibraryTests sets bDisableStaticAnalysis and is not passed to -Module.
#
# The macos-gh runner has UE 5.5. That engine uses the Clang shipped with
# Xcode 15.2 through 16.9 (see Engine/Config/Apple/Apple_SDK.json).
# UnrealBuildTool does not look at a separately installed LLVM, so this script
# does not download one.
#
# HTML reports are written under <UE>/Engine/Source/<File>.plist because that is
# the analyzer process working directory. A report folder is created only for a
# .cpp file with a finding. This script removes those folders for PubnubLibrary
# before the run, then fails if any come back.
#
# Usage:
#   ./run_static_analysis_mac.sh
#   UE_ROOT="/Users/Shared/Epic Games/UE_5.5" ./run_static_analysis_mac.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLUGIN_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
UE_ROOT="${UE_ROOT:-/Users/Shared/Epic Games/UE_5.5}"
REPORT_ROOT="${PLUGIN_ROOT}/clang-analysis-reports"
XCODE_MIN="15.2.0"
XCODE_MAX="16.9.0"

die() {
  echo "error: $*" >&2
  exit 1
}

normalize_version() {
  local major minor patch
  IFS=. read -r major minor patch <<< "$1"
  minor="${minor:-0}"
  patch="${patch:-0}"
  major="${major%%[!0-9]*}"
  minor="${minor%%[!0-9]*}"
  patch="${patch%%[!0-9]*}"
  printf '%d.%d.%d' "${major:-0}" "${minor:-0}" "${patch:-0}"
}

version_ge() {
  local a_major a_minor a_patch b_major b_minor b_patch
  IFS=. read -r a_major a_minor a_patch <<< "$(normalize_version "$1")"
  IFS=. read -r b_major b_minor b_patch <<< "$(normalize_version "$2")"
  if (( a_major > b_major )); then return 0; fi
  if (( a_major < b_major )); then return 1; fi
  if (( a_minor > b_minor )); then return 0; fi
  if (( a_minor < b_minor )); then return 1; fi
  (( a_patch >= b_patch ))
}

require_macos() {
  [[ "$(uname -s)" == "Darwin" ]] || die "This check runs on the macOS CI runner (uname is '$(uname -s)')."
}

require_xcode() {
  if ! xcode-select -p >/dev/null 2>&1; then
    die "Xcode is not selected. UE 5.5 analyzes with the Clang from Xcode ${XCODE_MIN} through ${XCODE_MAX}. Install that Xcode and run xcode-select -s. A separate LLVM install is not used."
  fi
  command -v xcodebuild >/dev/null 2>&1 || die "xcodebuild was not found. Install Xcode ${XCODE_MIN} through ${XCODE_MAX}."

  local version_output version
  if ! version_output="$(xcodebuild -version 2>&1)"; then
    die "xcodebuild -version failed. Accept the Xcode license and select Xcode ${XCODE_MIN} through ${XCODE_MAX}. Output: ${version_output}"
  fi
  version="$(printf '%s\n' "${version_output}" | awk '/^Xcode / { print $2; exit }')"
  [[ -n "${version}" ]] || die "Could not read the Xcode version from: ${version_output}"
  if ! version_ge "${version}" "${XCODE_MIN}" || ! version_ge "${XCODE_MAX}" "${version}"; then
    die "Xcode ${version} is outside UE 5.5's supported range ${XCODE_MIN}–${XCODE_MAX}. Select a supported Xcode with xcode-select. Do not install a separate LLVM; UnrealBuildTool uses Xcode's Clang."
  fi

  local clang
  clang="$(xcrun --find clang)"
  [[ -x "${clang}" ]] || die "Xcode ${version} does not provide clang at ${clang}."
  echo "Xcode ${version}"
  echo "Clang: ${clang}"
  "${clang}" --version
}

require_engine() {
  local build_sh="${UE_ROOT}/Engine/Build/BatchFiles/Mac/Build.sh"
  [[ -x "${build_sh}" || -f "${build_sh}" ]] || die "UE 5.5 Mac Build.sh was not found at '${build_sh}'. Set UE_ROOT to the engine install (default: /Users/Shared/Epic Games/UE_5.5)."
  echo "Engine: ${UE_ROOT}"
}

library_cpp_basenames() {
  find "${PLUGIN_ROOT}/Source/PubnubLibrary" -name '*.cpp' -type f | while IFS= read -r cpp; do
    basename "${cpp}" .cpp
  done | sort -u
}

clean_report_dirs() {
  local engine_source="${UE_ROOT}/Engine/Source"
  local base
  while IFS= read -r base; do
    [[ -n "${base}" ]] || continue
    rm -rf "${engine_source}/${base}.plist"
  done < <(library_cpp_basenames)
}

write_host_project() {
  local host="$1"
  mkdir -p "${host}/Source/AnalyzerHost" "${host}/Plugins"

  ln -sfn "${PLUGIN_ROOT}" "${host}/Plugins/PubnubLibrary"

  cat > "${host}/AnalyzerHost.uproject" <<'EOF'
{
	"FileVersion": 3,
	"EngineAssociation": "5.5",
	"Category": "",
	"Description": "Temporary host used to run the PubnubLibrary Clang static analyzer.",
	"Modules": [
		{
			"Name": "AnalyzerHost",
			"Type": "Runtime",
			"LoadingPhase": "Default"
		}
	],
	"Plugins": [
		{
			"Name": "PubnubLibrary",
			"Enabled": true
		}
	]
}
EOF

  cat > "${host}/Source/AnalyzerHostEditor.Target.cs" <<'EOF'
using UnrealBuildTool;
using System.Collections.Generic;

public class AnalyzerHostEditorTarget : TargetRules
{
	public AnalyzerHostEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("AnalyzerHost");
	}
}
EOF

  cat > "${host}/Source/AnalyzerHost.Target.cs" <<'EOF'
using UnrealBuildTool;
using System.Collections.Generic;

public class AnalyzerHostTarget : TargetRules
{
	public AnalyzerHostTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("AnalyzerHost");
	}
}
EOF

  cat > "${host}/Source/AnalyzerHost/AnalyzerHost.Build.cs" <<'EOF'
using UnrealBuildTool;

public class AnalyzerHost : ModuleRules
{
	public AnalyzerHost(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bDisableStaticAnalysis = true;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"PubnubLibrary"
		});
	}
}
EOF

  cat > "${host}/Source/AnalyzerHost/AnalyzerHost.h" <<'EOF'
#pragma once

#include "CoreMinimal.h"
EOF

  cat > "${host}/Source/AnalyzerHost/AnalyzerHost.cpp" <<'EOF'
#include "AnalyzerHost.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, AnalyzerHost, "AnalyzerHost");
EOF
}

collect_findings() {
  local engine_source="${UE_ROOT}/Engine/Source"
  local base report
  local found=0

  rm -rf "${REPORT_ROOT}"
  mkdir -p "${REPORT_ROOT}"

  while IFS= read -r base; do
    [[ -n "${base}" ]] || continue
    local report_dir="${engine_source}/${base}.plist"
    [[ -d "${report_dir}" ]] || continue
    shopt -s nullglob
    local reports=( "${report_dir}"/report-*.html )
    shopt -u nullglob
    if (( ${#reports[@]} == 0 )); then
      continue
    fi
    found=1
    mkdir -p "${REPORT_ROOT}/${base}.plist"
    for report in "${reports[@]}"; do
      cp "${report}" "${REPORT_ROOT}/${base}.plist/"
      echo "finding: ${report}"
    done
  done < <(library_cpp_basenames)

  if (( found == 0 )); then
    echo "Static code analyser found no issues in PubnubLibrary."
    return 0
  fi

  echo "Static code analyser found issues. HTML reports: ${REPORT_ROOT}" >&2
  if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    {
      echo "## Static code analyser"
      echo
      echo "PubnubLibrary has Clang static analyzer findings. Reports are uploaded as the \`static-code-analyser-reports\` artifact."
      echo
      find "${REPORT_ROOT}" -name 'report-*.html' | sort | sed 's/^/- /'
    } >> "${GITHUB_STEP_SUMMARY}"
  fi
  return 1
}

kill_tree() {
  local pid="$1"
  local child
  local children
  children="$(pgrep -P "${pid}" 2>/dev/null || true)"
  for child in ${children}; do
    kill_tree "${child}"
  done
  kill "${pid}" 2>/dev/null || true
}

cleanup_host() {
  if [[ -n "${UBT_PID:-}" ]]; then
    kill_tree "${UBT_PID}"
  fi
  if [[ -n "${TAIL_PID:-}" ]]; then
    kill "${TAIL_PID}" 2>/dev/null || true
    wait "${TAIL_PID}" 2>/dev/null || true
  fi
  if [[ -n "${HOST_DIR:-}" ]]; then
    rm -rf "${HOST_DIR}"
  fi
}

# GitHub's timeout-minutes cancels the step's shell. It does not reliably kill
# UnrealBuildTool started under `script`, and it does not apply to a job that
# has already started. Stream the UBT log ourselves and print a process snapshot
# every minute so a quiet analyzer is visible in the job log.
run_analyzer() {
  local build_sh="$1"
  local ubt_log="${HOST_DIR}/UnrealBuildTool.log"
  local engine_log="${UE_ROOT}/Engine/Programs/UnrealBuildTool/Log.txt"
  : > "${ubt_log}"

  echo "UBT log for this run: ${ubt_log}"
  echo "Engine UBT log: ${engine_log}"

  "${build_sh}" AnalyzerHostEditor Mac Development \
    -Project="${HOST_DIR}/AnalyzerHost.uproject" \
    -StaticAnalyzer=Clang \
    -StaticAnalyzerOutputType=html \
    -StaticAnalyzerProjectOnly \
    -DisableUnity \
    -Module=PubnubLibrary \
    -Log="${ubt_log}" > "${HOST_DIR}/ubt-console.log" 2>&1 &
  UBT_PID=$!
  echo "UnrealBuildTool pid: ${UBT_PID}"

  tail -n +1 -F "${ubt_log}" &
  TAIL_PID=$!

  local next_heartbeat=$((SECONDS + 60))
  local status=0
  while kill -0 "${UBT_PID}" 2>/dev/null; do
    sleep 5
    if (( SECONDS >= next_heartbeat )); then
      echo "---- still running after ${SECONDS}s ----"
      ps -o pid,ppid,etime,%cpu,command -p "${UBT_PID}" || true
      pgrep -lf "clang|UnrealBuildTool" || true
      if [[ ! -s "${ubt_log}" && -f "${engine_log}" ]]; then
        echo "---- engine log (UBT has not written ${ubt_log} yet) ----"
        tail -n 20 "${engine_log}" || true
      fi
      next_heartbeat=$((SECONDS + 60))
    fi
  done

  wait "${UBT_PID}" || status=$?
  sleep 1
  kill "${TAIL_PID}" 2>/dev/null || true
  wait "${TAIL_PID}" 2>/dev/null || true
  TAIL_PID=""
  UBT_PID=""
  return "${status}"
}

main() {
  require_macos
  require_xcode
  require_engine

  HOST_DIR="$(mktemp -d "${TMPDIR:-/tmp}/pubnub-analyzer-host.XXXXXX")"
  trap cleanup_host EXIT

  echo "Host project: ${HOST_DIR}"
  write_host_project "${HOST_DIR}"
  clean_report_dirs

  # -WaitMutex blocks inside UnrealBuildTool before it prints anything, so a
  # second engine build on this runner looks like a hung job. Fail instead.
  if pgrep -lf "UnrealBuildTool" >/dev/null 2>&1; then
    echo "Another UnrealBuildTool is still running on this machine:" >&2
    pgrep -lf "UnrealBuildTool" >&2 || true
    die "Stop that process or wait for it to exit, then rerun this job. This check does not wait for the engine lock."
  fi

  local build_sh="${UE_ROOT}/Engine/Build/BatchFiles/Mac/Build.sh"
  echo "Starting UnrealBuildTool. A snapshot of the process is printed every minute."
  run_analyzer "${build_sh}"

  collect_findings
}

main "$@"
