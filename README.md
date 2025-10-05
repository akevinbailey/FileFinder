# File Finder

## PURPOSE
"File Finder" is a Qt 6.9 desktop utility that helps C++ and Arduino developers quickly locate files within a chosen directory tree. It provides a simple GUI to enter a filename, select a search root, and view all matching full paths.  This application is optimized for fast file searches and low memory usage.

## DESCRIPTION
The application presents two inputs: **File Name** for the target filename (e.g., `my-header.h`) and **Search Directory** for the root folder to scan. You can type a directory path directly or click **Browse…** to open a native folder chooser. When you press **Search**, the app recursively scans the directory and all subdirectories, performing a case-insensitive match on the filename and listing each full path in the **Found** textbox.

Built with C++20 and Qt Widgets, the tool is lightweight and responsive. It persists the **last searched directory** using `QSettings`, so your preferred search root is prefilled on the next launch—streamlining repeated lookups and everyday workflows.

## REQUIRED FILES
Due to the way the Qt works, the files must be in a very specific directory structure.
### On Windows:
Make sure you have the Visual C++ Redistributable installed.  You can download it from the Microsoft website:
[Microsoft Visual C++ Redistributable packages for Visual Studio 2015-2022](https://aka.ms/vs/17/release/vc_redist.x64.exe)

Directory structure:
* `.\icons\FileFinder.ico`
* `.\icons\FileFinder.png`
* `.\imageformats\qico.dll`
* `.\platforms\qwindows.dll`
* `.\styles\qmodernwindowsstyle.dll`
* `.\FileFinder.exe`
* `.\Qt6Core.dll`
* `.\Qt6Gui.dll`
* `.\Qt6Widgets.dll`

### On Linux:
I am not using Wayland, because it is a pain in the ass to get Qt6 working on Gnome desktop.  Therefore, I am only using XCB.  Make sure you have the following packages installed:

`sudo apt-get install libxcb1 libxkbcommon-x11-0 libxrender1 libxext6 libfontconfig1 libxcb-cursor0 libxcb-xinerama0 libxcb-render0 libxcb-shape0 libxcb-xfixes0`

Directory structure:
* `./bin/icons/FileFinder.ico`
* `./bin/icons/FileFinder.png`
* `./bin/imageformats/libqico.so`
* `./bin/platforms/libqxcb.so`
* `./bin/platformthemes/libqgtk3.so`
* `./bin/platformthemes/libqxdgdesktopportal.so`
* `./bin/FileFinder`
* `./bin/qt.conf`
* `./lib/libicudata.so.73`
* `./lib/libicui18n.so.73`
* `./lib/libicuuc.so.73`
* `./lib/libQt6Core.so.6`
* `./lib/libQt6DBus.so.6`
* `./lib/libQt6Gui.so.6`
* `./lib/libQt6OpenGL.so.6`
* `./lib/libQt6Widgets.so.6`
* `./lib/libQt6XcbQpa.so.6`
