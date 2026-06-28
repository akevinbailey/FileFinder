#!/usr/bin/env bash
#
# Copyright (c) 2026. Andrew Kevin Bailey
# This code, firmware, and software is released under the MIT License (http://opensource.org/licenses/MIT).
#
# The MIT License (MIT)
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or significant portions of
# the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
# BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Package the macOS arm64 app bundle as a ZIP archive.
# Usage: copy_and_package.sh <archive_type> <architecture_name> <source_dir> <dest_dir> <executable_name> [extra_paths...]
#   archive_type:  type of archive (tar, gz, or zip)
#   architecture_name: name of the target compile architecture
#   source_dir: where files live now
#   dest_dir:   staging directory (will be deleted then recreated)
#   executable_name: file name of the main executable (relative to source_dir)
#   extra_paths: zero or more additional files/dirs to copy (relative to source_dir)

./copy_and_package.sh zip macos_arm64 ~/Development/RemoteBuilds/FileFinder/cmake-build-release-macbook-pro ~/Development/RemoteBuilds/FileFinder/cmake-build-release-macbook-pro/temp FileFinder.app
