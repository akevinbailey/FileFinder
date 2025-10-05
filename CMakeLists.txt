# Cross-platform CMake for FileFinder (x64 Windows + x64 Linux)
cmake_minimum_required(VERSION 4.0)
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
    set(CMAKE_PREFIX_PATH "C:/Development/Qt/6.9.3/msvc2022_64")
    set(Qt6_DIR "C:/Development/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6")
    set(DEPLOY_QT "C:/Development/Qt/6.9.3/msvc2022_64/bin/windeployqt6.exe")
else() # Linux
    set(CMAKE_PREFIX_PATH "/home/akbailey/Qt/6.9.3/gcc_64")
    set(Qt6_DIR "/home/akbailey/Qt/6.9.3/gcc_64/lib/cmake/Qt6")
    set(DEPLOY_QT "") # not used on Linux

    # Use the INSTALL_RPATH for the build too (prevents CMake from appending SDK lib dirs)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    # Force the rpath you want, and don't append link directories
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)
    # Do not skip rpaths
    set(CMAKE_SKIP_BUILD_RPATH   OFF)
    set(CMAKE_SKIP_INSTALL_RPATH OFF)
endif()

# Find Qt
find_package(Qt6 6.9.3 REQUIRED COMPONENTS Widgets)

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

# GUI subsystem on Windows (no console)
set(_WIN_GUI_FLAG "")
if (WIN32)
    set(_WIN_GUI_FLAG WIN32)
endif()

add_executable(${PROJECT_NAME} ${_WIN_GUI_FLAG}
        ${APP_SOURCES}
        ${APP_HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

# Nice warnings on GCC/Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

############################# Copy libraries to their required directories #############################

# Add icons
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        # Set up the plugin directories
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icons"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/resources/FileFinder.ico" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icons"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/resources/FileFinder.png" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icons"
)

# Resolve Qt dirs from Qt6_DIR
get_filename_component(_qt_cmake_dir "${Qt6_DIR}" DIRECTORY)        # .../lib/cmake
get_filename_component(_qt_lib_dir   "${_qt_cmake_dir}/.." ABSOLUTE) # .../lib
get_filename_component(_qt_arch_dir  "${_qt_lib_dir}/.." ABSOLUTE)   # .../gcc_64 or .../msvc2022_64
set(_qt_plugins_dir "${_qt_arch_dir}/plugins")

# For the Windows application to work, the files must be in certain locations.  The .\Release directory has all the
# files needed to execute the application
if (WIN32)
    # Windows needs Unicode
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

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

    # We must execute an external cmake script to get the extra debug files due to the way cmake handles multi-config generators
    set(WINDEPLOYQT "C:/Development/Qt/6.9.3/msvc2022_64/bin/windeployqt6.exe")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -DWINDEPLOYQT:FILEPATH=${WINDEPLOYQT}
            -DAPP=$<TARGET_FILE:${PROJECT_NAME}>
            -DCONFIG=$<CONFIG>
            -P "${CMAKE_CURRENT_LIST_DIR}/cmake/run_windeployqt_debug.cmake"
            VERBATIM
            COMMENT "windeployqt (Debug only)"
    )
endif()

# For the Linux application to work, the files must be in certain locations.  The ./bin directory has application and
# platform plugin libraries and the ./lib has all the Qt libraries.  The ./bin and ./lib must be at the same directory
# level for the application to work.
if (UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Debug/bin"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Release/bin"

            LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Debug/lib"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Release/lib"

            ARCHIVE_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/Debug/lib"
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/Release/lib"
    )

    # Make the exe look in its own folder for shared libs
    set_target_properties(${PROJECT_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "$ORIGIN/../lib"
            INSTALL_RPATH_USE_LINK_PATH FALSE
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            # Set up the plugin directories
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platformthemes"
            # Copy the imageformats plugin
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_qt_plugins_dir}/imageformats/libqico.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats/"
            # Copy the platform plugin
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_qt_plugins_dir}/platforms/libqxcb.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/"
            # Copy the platformthemes plugin
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_qt_plugins_dir}/platformthemes/libqgtk3.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platformthemes/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_qt_plugins_dir}/platformthemes/libqxdgdesktopportal.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platformthemes/"
    )

    # Helper: copy the REAL file a SONAME symlink points to, saving it under the SONAME
    function(copy_soname_into_bindir SONAME)
        get_filename_component(_real "${_qt_lib_dir}/${SONAME}" REALPATH)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_real}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../lib/${SONAME}"
        )
    endfunction()
    # Copy Qt libs by SONAME (these names are what the loader looks for) because the are sym-links
    copy_soname_into_bindir("libQt6Core.so.6")
    copy_soname_into_bindir("libQt6Gui.so.6")
    copy_soname_into_bindir("libQt6Widgets.so.6")
    copy_soname_into_bindir("libQt6DBus.so.6")
    copy_soname_into_bindir("libQt6OpenGL.so.6")
    # If you keep X11 fallback:
    copy_soname_into_bindir("libQt6XcbQpa.so.6")
    # ICU (Qt is typically built against ICU on the official kits)
    copy_soname_into_bindir("libicui18n.so.73")
    copy_soname_into_bindir("libicuuc.so.73")
    copy_soname_into_bindir("libicudata.so.73")

    # Tell Qt where to find plugins, relative to the exe
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" "[Paths]\nPlugins=./platforms\n")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/qt.conf"
    )
endif()