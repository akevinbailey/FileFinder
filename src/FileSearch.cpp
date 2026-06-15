// Copyright (c) 2025. Andrew Kevin Bailey
// This code, firmware, and software is released under the MIT License (http://opensource.org/licenses/MIT).
//
// The MIT License (MIT)
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or significant portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "FileSearch.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <ranges>
#include <string>
#include <system_error>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace {

#ifdef _WIN32
bool enablePrivilege(const wchar_t *name) {
  HANDLE token = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
    return false;
  }

  TOKEN_PRIVILEGES privileges{};
  if (!LookupPrivilegeValueW(nullptr, name, &privileges.Privileges[0].Luid)) {
    CloseHandle(token);
    return false;
  }

  privileges.PrivilegeCount = 1;
  privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  const BOOL adjusted = AdjustTokenPrivileges(token, FALSE, &privileges, sizeof(privileges), nullptr, nullptr);
  const DWORD error = GetLastError();
  CloseHandle(token);

  return adjusted != FALSE && error == ERROR_SUCCESS;
}
#endif

std::u16string visitedKey(const fs::path &path) {
  std::error_code ec;
  auto canonical = fs::weakly_canonical(path, ec);
  if (ec) {
    canonical = path.lexically_normal();
  }

  auto key = canonical.generic_u16string();
#ifdef _WIN32
  std::ranges::transform(key, key.begin(), [](const char16_t ch) {
    return static_cast<char16_t>(std::towlower(static_cast<wint_t>(ch)));
  });
#endif
  return key;
}

std::u16string sortKey(const fs::path &path) {
  auto key = path.filename().generic_u16string();
#ifdef _WIN32
  std::ranges::transform(key, key.begin(), [](const char16_t ch) {
    return static_cast<char16_t>(std::towlower(static_cast<wint_t>(ch)));
  });
#endif
  return key;
}

bool shouldDeferDirectory(const fs::path &path) {
  const auto name = path.filename().generic_u16string();
  if (!name.empty() && name.front() == u'.') {
    return true;
  }

#ifdef _WIN32
  const DWORD attributes = GetFileAttributesW(path.c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES) {
    return false;
  }

  constexpr DWORD deferredAttributes = FILE_ATTRIBUTE_HIDDEN |
                                       FILE_ATTRIBUTE_SYSTEM |
                                       FILE_ATTRIBUTE_REPARSE_POINT;
  return (attributes & deferredAttributes) != 0;
#else
  return false;
#endif
}

fs::path toPath(const QString &path) {
#ifdef _WIN32
  return {path.toStdWString()};
#else
  return {path.toStdString()};
#endif
}

QString toQString(const fs::path &path) {
#ifdef _WIN32
  return QString::fromStdWString(path.wstring());
#else
  return QString::fromStdString(path.string());
#endif
}

}

QStringList FileSearch::findFiles(const QString& rootDir, const QString& headerFileName) {
  QStringList results;
  const QString target = headerFileName.trimmed();

#ifdef _WIN32
  enablePrivilege(SE_BACKUP_NAME);
  enablePrivilege(SE_RESTORE_NAME);
#endif

  std::error_code ec;
  const fs::path rootPath = toPath(rootDir);
  if (!fs::exists(rootPath, ec) || !fs::is_directory(rootPath, ec)) {
    return results;
  }

  constexpr fs::directory_options opts = fs::directory_options::skip_permission_denied;
  std::unordered_set<std::u16string> visitedDirectories;
  visitedDirectories.insert(visitedKey(rootPath));

  std::vector<fs::path> pendingDirectories;
  pendingDirectories.push_back(rootPath);

  while (!pendingDirectories.empty()) {
    const fs::path currentDirectory = std::move(pendingDirectories.back());
    pendingDirectories.pop_back();

    std::vector<fs::path> childDirectories;
    std::vector<fs::path> deferredChildDirectories;

    for (fs::directory_iterator it(currentDirectory, opts, ec), end; it != end; it.increment(ec)) {
      if (ec) { ec.clear(); continue; }

      const fs::directory_entry &entry = *it;
      if (entry.is_directory(ec)) {
        if (ec) { ec.clear(); continue; }

        const fs::path& childPath = entry.path();
        const auto [_, inserted] = visitedDirectories.insert(visitedKey(childPath));
        if (!inserted) {
          continue;
        }

        if (shouldDeferDirectory(childPath)) {
          deferredChildDirectories.push_back(childPath);
        } else {
          childDirectories.push_back(childPath);
        }
        continue;
      }
      if (ec) { ec.clear(); continue; }

      if (!entry.is_regular_file(ec)) { if (ec) ec.clear(); continue; }
      if (toQString(entry.path().filename()).compare(target, Qt::CaseInsensitive) == 0) {
        const fs::path absolutePath = fs::absolute(entry.path(), ec);
        results << toQString(ec ? entry.path() : absolutePath);
        if (ec) ec.clear();
      }
    }

    if (ec) {
      ec.clear();
    }

    const auto comparePaths = [](const fs::path &a, const fs::path &b) {
      return sortKey(a) < sortKey(b);
    };

    std::ranges::sort(childDirectories, comparePaths);
    std::ranges::sort(deferredChildDirectories, comparePaths);

    for (auto & deferredChildDirectory : std::views::reverse(deferredChildDirectories)) {
      pendingDirectories.push_back(deferredChildDirectory);
    }
    for (auto & childDirectory : std::views::reverse(childDirectories)) {
      pendingDirectories.push_back(childDirectory);
    }
  }

  results.sort(Qt::CaseInsensitive);
  return results;
}
