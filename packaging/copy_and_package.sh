#!/usr/bin/env bash
# copy_and_package.sh - copy app + extras, restrict write permissions, and make an archive
# Usage: copy_and_package.sh <archive_type> <architecture_name> <source_dir> <dest_dir> <executable_name> [extra_paths...]
#   archive_type:  type of archive (tar, gz, or zip)
#   architecture_name: name of the target compile architecture (linux-x64, linux-arm64, macos-arm64)
#   source_dir: where files live now
#   dest_dir:   staging directory (will be deleted then recreated)
#   executable_name: file name of the main executable (relative to source_dir)
#   extra_paths: zero or more additional files/dirs to copy (relative to source_dir)

set -euo pipefail

usage() {
  echo "Usage: $(basename "$0") <archive_type> <architecture_name> <source_dir> <dest_dir> <executable_name> [extra_paths...]"
  exit 1
}

absolute_path() {
  local path=$1
  local component
  local -a components normalized
  local normalized_count=0

  [[ "$path" = /* ]] || path="$PWD/$path"
  IFS="/" read -r -a components <<< "$path"

  for component in "${components[@]}"; do
    case "$component" in
      ""|.) ;;
      ..)
        if [[ $normalized_count -gt 0 ]]; then
          normalized_count=$((normalized_count - 1))
          unset "normalized[normalized_count]"
        fi
        ;;
      *)
        normalized[normalized_count]=$component
        normalized_count=$((normalized_count + 1))
        ;;
    esac
  done

  if [[ $normalized_count -eq 0 ]]; then
    printf "/\n"
  else
    local IFS="/"
    printf "/%s\n" "${normalized[*]}"
  fi
}

[[ $# -ge 5 ]] || usage

ARCHIVE_TYPE=$1
ARCH_NAME=$2
SRC_DIR=$3
DST_DIR=$4
EXEC_NAME=$5
shift 5
EXTRAS=( "$@" )
EXTRA_COUNT=$#

# Basic checks
case "$ARCHIVE_TYPE" in
  tar|gz|zip) ;;
  *) echo "ERROR: unsupported archive_type '$ARCHIVE_TYPE' (expected tar, gz, or zip)"; exit 5 ;;
esac
[[ "$ARCHIVE_TYPE" != "zip" ]] || command -v zip >/dev/null || { echo "ERROR: zip command not found"; exit 7; }
[[ -n "$ARCH_NAME" && "$ARCH_NAME" != */* ]] || { echo "ERROR: invalid architecture_name: '$ARCH_NAME'"; exit 6; }
[[ -d "$SRC_DIR" ]] || { echo "ERROR: source_dir not found: $SRC_DIR"; exit 2; }
[[ -e "$SRC_DIR/$EXEC_NAME" ]] || { echo "ERROR: executable not found: $SRC_DIR/$EXEC_NAME"; exit 3; }

# Always recreate destination dir (with safety guard)
DST_DIR_ABS=$(absolute_path "$DST_DIR")
[[ -z "$DST_DIR_ABS" || "$DST_DIR_ABS" = "/" ]] && { echo "ERROR: refusing to use dangerous dest_dir: '$DST_DIR'"; exit 9; }
rm -rf -- "$DST_DIR_ABS"
mkdir -p -- "$DST_DIR_ABS"

# Copy executable
cp -a -- "$SRC_DIR/$EXEC_NAME" "$DST_DIR_ABS/"

# Copy extras (if any)
if [[ $EXTRA_COUNT -gt 0 ]]; then
  for p in "${EXTRAS[@]}"; do
    [[ -e "$SRC_DIR/$p" ]] || { echo "ERROR: extra path not found: $SRC_DIR/$p"; exit 4; }
    cp -a -- "$SRC_DIR/$p" "$DST_DIR_ABS/"
  done
fi

# Paths we just copied (in dest)
COPIED_NAMES=( "$EXEC_NAME" )
if [[ $EXTRA_COUNT -gt 0 ]]; then
  COPIED_NAMES+=( "${EXTRAS[@]}" )
fi
COPIED_PATHS=( )
for n in "${COPIED_NAMES[@]}"; do
  COPIED_PATHS+=( "$DST_DIR_ABS/$n" )
done

# Preserve existing execute bits, including executables inside macOS .app
# bundles, while removing write access for group and others.
chmod -R go-w "${COPIED_PATHS[@]}"

# A directly packaged executable must remain runnable. Directory payloads such
# as macOS .app bundles retain their internal executable modes from cp -a.
if [[ -f "$DST_DIR_ABS/$EXEC_NAME" ]]; then
  chmod a+x "$DST_DIR_ABS/$EXEC_NAME"
fi

# Create the requested archive in the source directory.
EXEC_BASENAME=${EXEC_NAME##*/}
ARCHIVE_STEM=$EXEC_BASENAME
case "$ARCHIVE_STEM" in
  *.AppImage) ARCHIVE_STEM=${ARCHIVE_STEM%.AppImage} ;;
  *.app) ARCHIVE_STEM=${ARCHIVE_STEM%.app} ;;
esac
ARCHIVE_BASE="$SRC_DIR/${ARCHIVE_STEM}_$ARCH_NAME"
case "$ARCHIVE_TYPE" in
  tar)
    ARCHIVE_PATH="$ARCHIVE_BASE.tar"
    tar -cf "$ARCHIVE_PATH" -C "$DST_DIR_ABS" "${COPIED_NAMES[@]}"
    ;;
  gz)
    ARCHIVE_PATH="$ARCHIVE_BASE.tar.gz"
    tar -czf "$ARCHIVE_PATH" -C "$DST_DIR_ABS" "${COPIED_NAMES[@]}"
    ;;
  zip)
    ARCHIVE_PATH="$ARCHIVE_BASE.zip"
    rm -f -- "$ARCHIVE_PATH"
    (
      cd -- "$DST_DIR_ABS"
      zip -r "$ARCHIVE_PATH" "${COPIED_NAMES[@]}"
    )
    ;;
esac

echo "OK: Packed files into $ARCHIVE_PATH"
