#!/usr/bin/env bash
# Package the Linux arm64 release as a compressed tar archive.
# Usage: copy_and_package.sh <archive_type> <architecture_name> <source_dir> <dest_dir> <executable_name> [extra_paths...]
#   archive_type:  type of archive (tar, gz, or zip)
#   architecture_name: name of the target compile architecture
#   source_dir: where files live now
#   dest_dir:   staging directory (will be deleted then recreated)
#   executable_name: file name of the main executable (relative to source_dir)
#   extra_paths: zero or more additional files/dirs to copy (relative to source_dir)

./copy_and_package.sh gz linux_arm64 ~/Development/FileFinder/cmake-build-release-wks-pi5 ~/Development/FileFinder/cmake-build-release-wks-pi5/temp FileFinder.AppImage icons
