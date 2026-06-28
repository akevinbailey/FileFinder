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

#include <QApplication>
#include <QGuiApplication>
#include <QString>

#include "MainWindow.h"

namespace {
constexpr auto kApplicationName = "FileFinder";
constexpr auto kApplicationDisplayName = "File Finder";
#ifndef FILEFINDER_DESKTOP_FILE_NAME
#define FILEFINDER_DESKTOP_FILE_NAME "io.github.akevinbailey.filefinder"
#endif
constexpr auto kDesktopFileName = FILEFINDER_DESKTOP_FILE_NAME;
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCoreApplication::setOrganizationName("Andrew Kevin Bailey");
  app.setProperty("copyright_date", QStringLiteral("2026"));
  QCoreApplication::setApplicationName(kApplicationName);
  QCoreApplication::setApplicationVersion("1.5.0");
  QGuiApplication::setApplicationDisplayName(kApplicationDisplayName);
  QGuiApplication::setDesktopFileName(kDesktopFileName);
  app.setProperty("homepage", QStringLiteral("https://github.com/akevinbailey/FileFinder"));

  QIcon icon;
  icon.addFile(":/icons/FileFinder.ico");
  icon.addFile(":/icons/FileFinder.png");
  QGuiApplication::setWindowIcon(icon);

  MainWindow w;
  w.setWindowIcon(icon);
  w.show();
  return app.exec();
}
