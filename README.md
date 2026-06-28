# File Finder

## PURPOSE
"File Finder" is a Qt 6.10 desktop utility that helps C++ and Arduino developers quickly locate files within a chosen directory tree. It provides a simple GUI to enter a filename, select a search root, and view all matching full paths.  This application is optimized for fast file searches and low memory usage.

## DESCRIPTION
The application presents two inputs: **File Name** for the target filename (e.g., `my-header.h`) and **Search Directory** for the root folder to scan. You can type a directory path directly or click **Browse…** to open a native folder chooser. When you press **Search**, the app recursively scans the directory and all subdirectories, performing a case-insensitive match on the filename and listing each full path in the **Found** textbox.

Built with C++20 and Qt Widgets, the tool is lightweight and responsive. It persists the **last searched directory** using `QSettings`, so your preferred search root is prefilled on the next launch—streamlining repeated lookups and everyday workflows.

## REQUIRED FILES
Due to the way the Qt works, the files must be in a very specific directory structure.
### On Windows:
Make sure you have the Visual C++ Redistributable installed.  You can download it from the Microsoft website:
[Microsoft Visual C++ Redistributable packages for Visual Studio 2017-2026](https://aka.ms/vc14/vc_redist.x64.exe)

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
On Linux, I am using the AppImage to package the application.  This allows the application to run on any Linux distribution without any additional dependencies.  I have included the icon so you can set up a .desktop file for your distribution.

Directory structure:
* `./icons/FileFinder.ico`
* `./icons/FileFinder.png`
* `./FileFinder.AppImage`

### On MacOS:
On macOS all the files are in the app bundle and no extra steps are required.