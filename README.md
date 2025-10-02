# File Finder

## PURPOSE
"File Finder" is a Qt 6.9 desktop utility that helps C++ and Arduino developers quickly locate files within a chosen directory tree. It provides a simple GUI to enter a filename, select a search root, and view all matching full paths.

## DESCRIPTION
The application presents two inputs: **File Name** for the target filename (e.g., `my-header.h`) and **Search Directory** for the root folder to scan. You can type a directory path directly or click **Browse…** to open a native folder chooser. When you press **Search**, the app recursively scans the directory and all subdirectories, performing a case-insensitive match on the filename and listing each full path in the **Found** textbox.

Built with C++20 and Qt Widgets, the tool is lightweight and responsive. It persists the **last searched directory** using `QSettings`, so your preferred search root is prefilled on the next launch—streamlining repeated lookups and everyday workflows.

## REQUIRED FILES
### On Windows:
* `FileFinder.exe`
* `libgcc_s_seh-1.dll`
* `libstdc++-6.dll`
* `libwinpthread-1.dll`
* `Qt6Widgets.dll`
* `Qt6Core.dll`
* `Qt6Gui.dll`
* `imageformats\qico.dll`
* `platforms\qwindows.dll`
* `styles\qmodernwindowsstyle.dll`

