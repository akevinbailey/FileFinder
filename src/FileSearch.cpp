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
#include <QDirIterator>
#include <QFileInfo>

QStringList FileSearch::findFiles(const QString& rootDir, const QString& headerFileName) {
  QStringList results;
  const QString target = headerFileName.trimmed();

  QDirIterator it(rootDir, QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const QString filePath = it.next();
    const QFileInfo fi(filePath);
    // Compare base filename with case-insensitive check
    if (fi.fileName().compare(target, Qt::CaseInsensitive) == 0) {
      results << fi.absoluteFilePath();
    }
  }
  results.sort(Qt::CaseInsensitive);
  return results;
}