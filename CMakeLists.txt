# Cross-platform CMake for FileFinder (x64 Windows + x64 Linux)
cmake_minimum_required(VERSION 3.25)
project(FileFinder LANGUAGES CXX)

# C++20 + Qt autogen
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ------- Qt locations (override from CLI/GUI if desired) -------
if (WIN32)
    # Vulkan is not needed on Windows
    set(CMAKE_DISABLE_FIND_PACKAGE_WrapVulkanHeaders ON)
    # Defaults for Windows (Visual Studio)
    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "C:/Development/Qt/6.10.0/msvc2022_64/lib/cmake/Qt6")
    endif()
    if (NOT DEFINED Qt6_DIR)
        set(Qt6_DIR "C:/Development/Qt/6.10.0/msvc2022_64")
    endif()
elseif (APPLE)
    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "/Library/Qt/6.10.0/macos/lib/cmake/Qt6")
    endif()
    if (NOT DEFINED Qt6_DIR)
        set(Qt6_DIR "/Library/Qt/6.10.0/macos")
    endif()
else() # Linux
    # To build on Linux, use the Qt6 provided by the distro:
    # Ubuntu / Debian (amd64, arm64, etc.):
    #   sudo apt-get install -y build-essential cmake ninja-build qt6-base-dev qt6-base-dev-tools
    #   sudo apt-get install -y libqt6widgets6
    #   sudo apt-get install -y xwayland libxcb1 libxcb-render0 libxcb-shape0 libxcb-xfixes0 libx11-6 libxext6 libxrender1 fontconfig libxkbcommon-x11-0 libxcb-cursor0 libxkbcommon-dev
    # Fedora / RHEL / Rocky / Alma (x86_64, aarch64):
    #   sudo dnf install -y cmake ninja-build @development-tools qt6-qtbase-devel
    #   sudo dnf install -y qt6-qtbase-gui xorg-x11-server-Xwayland libxkbcommon-x11 libX11 libXext libXrender xcb-util-cursor
    # openSUSE
    #   sudo zypper install -y cmake ninja gcc-c++ libqt6-qtbase-devel
    #   sudo zypper install -y libqt6-qtbase-platformtheme libqt6-qtbase-gui xwayland xcb-util-cursor
    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt6")
    endif()
    if (NOT DEFINED Qt6_DIR)
        set(Qt6_DIR "/usr/lib/x86_64-linux-gnu/qt6")
    endif()
    # Force the rpath you want, and don't append link directories
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Source layout
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "${SRC_DIR}/*.h")

# QRC (processed via AUTORCC if present)
set(APP_QRC "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.qrc")
if (EXISTS "${APP_QRC}")
    list(APPEND APP_SOURCES "${APP_QRC}")
endif()

# Windows: add .rc icon if present
set(APP_RC "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.rc")
if (WIN32 AND EXISTS "${APP_RC}")
    list(APPEND APP_SOURCES "${APP_RC}")
endif()

# macOS: add .icns icon into the bundle if present
set(APP_ICON_MAC "${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}.icns")
if (APPLE AND EXISTS "${APP_ICON_MAC}")
    # Put icon into the bundle Resources
    set_source_files_properties(${APP_ICON_MAC}
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND APP_SOURCES "${APP_ICON_MAC}")
endif()

# GUI subsystem on Windows (no console)
set(_WIN_GUI_FLAG "")
if (WIN32)
    set(_WIN_GUI_FLAG WIN32)
endif()

# GUI subsystem on Apple (no console)
set(_MAC_BUNDLE_FLAG "")
if (APPLE)
    set(_MAC_BUNDLE_FLAG MACOSX_BUNDLE)
endif()

add_executable(${PROJECT_NAME} ${_WIN_GUI_FLAG} ${_MAC_BUNDLE_FLAG}
        ${APP_SOURCES}
        ${APP_HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

# Enable useful warnings
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

############################# Copy libraries to their required directories #############################

# macOS bundle properties (name, icon, id, etc.)
if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME          "${PROJECT_NAME}"
            MACOSX_BUNDLE_GUI_IDENTIFIER       "com.example.${PROJECT_NAME}"
            MACOSX_BUNDLE_BUNDLE_VERSION       "1.0.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
    )
    if (EXISTS "${APP_ICON_MAC}")
        set_target_properties(${PROJECT_NAME} PROPERTIES
                MACOSX_BUNDLE_ICON_FILE "${PROJECT_NAME}.icns")
    endif()
endif()

# Add icons
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        # Set up the plugin directories
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icons"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/resources/${PROJECT_NAME}.ico" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icons"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/resources/${PROJECT_NAME}.png" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icons"
)

# Resolve Qt dirs from Qt6_DIR
get_filename_component(_qt_cmake_dir "${Qt6_DIR}" DIRECTORY)        # .../lib/cmake
get_filename_component(_qt_lib_dir   "${_qt_cmake_dir}/.." ABSOLUTE) # .../lib
get_filename_component(_qt_arch_dir  "${_qt_lib_dir}/.." ABSOLUTE)   # .../gcc_64 or .../msvc2022_64
set(_qt_plugins_dir "${_qt_arch_dir}/plugins")

# For the Windows and MacOS applications to work, the files must be in certain locations.  The .\Release directory has all the
# files needed to execute the application
if (WIN32)
    # Windows and MacOS needs Unicode
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    # Find windeployqt.exe in the Qt bin dir
    find_program(DEPLOYQT windeployqt.exe
            HINTS "${Qt6_DIR}/bin" "C:/Development/Qt/6.10.0/msvc2022_64/bin"
    )

    if (NOT DEPLOYQT)
        message(FATAL_ERROR "windeployqt.exe not found. Please check Qt6_DIR or install Qt tools.")
    endif()

    # To reduce the file sizes on Window, we manually copy only the necessary libraries instead of using windeployqt.exe
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            # Set up the plugin directories
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/styles"
            # Copy the imageformats plugin
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_qt_plugins_dir}/imageformats/qico.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats/"
            # Copy the platform plugin
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_qt_plugins_dir}/platforms/qwindows.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/"
            # Copy the styles plugin
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_qt_plugins_dir}/styles/qmodernwindowsstyle.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/styles/"
            # Setup libs
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt6::Core>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt6::Gui>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt6::Widgets>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    # We must execute an external cmake script to call windeployqt.exe and get the extra debug files due to the way cmake handles multi-config generators on Windows
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -DDEPLOYQT:FILEPATH=${DEPLOYQT}
            -DAPP=$<TARGET_FILE:${PROJECT_NAME}>
            -DCONFIG=$<CONFIG>
            -P "${CMAKE_CURRENT_LIST_DIR}/cmake/run_deployqt_debug.cmake"
            VERBATIM
            COMMENT "deployqt (Debug only)"
    )
elseif (APPLE)
    # Windows and MacOS needs Unicode
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

    # Find macdeployqt in the Qt bin dir
    find_program(DEPLOYQT macdeployqt
            HINTS "${Qt6_DIR}/bin" "/Library/Qt/6.10.0/macos/bin"
    )

    if (NOT DEPLOYQT)
        message(FATAL_ERROR "macdeployqt not found. Please check Qt6_DIR or install Qt tools.")
    endif()

    # Run macdeployqt on the .app bundle itself
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${DEPLOYQT}" "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>" -verbose=1
            COMMENT "Running macdeployqt on ${PROJECT_NAME}.app"
    )
else()
    # Linux: no special deployment, rely on system Qt
endif()