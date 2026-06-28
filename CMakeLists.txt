# Cross-platform CMake for FileFinder
cmake_minimum_required(VERSION 3.25)
project(FileFinder VERSION 1.5.0 LANGUAGES CXX)

set(FILEFINDER_DISPLAY_NAME "File Finder")
set(FILEFINDER_DEVELOPER_ID "io.github.akevinbailey")
set(FILEFINDER_DEVELOPER_NAME "Andrew Kevin Bailey")
set(FILEFINDER_APPSTREAM_ID "${FILEFINDER_DEVELOPER_ID}.filefinder")
set(FILEFINDER_LINUX_ICON_NAME "${FILEFINDER_APPSTREAM_ID}")

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
        set(CMAKE_PREFIX_PATH "C:/Development/Qt/6.10.3/msvc2022_64/lib/cmake/Qt6")
    endif()
    if (NOT DEFINED Qt6_DIR)
        set(Qt6_DIR "C:/Development/Qt/6.10.3/msvc2022_64")
    endif()
elseif (APPLE)
    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "/Library/Qt/6.10.3/macos/lib/cmake/Qt6")
    endif()
    if (NOT DEFINED Qt6_DIR)
        set(Qt6_DIR "/Library/Qt/6.10.3/macos")
    endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
        message(STATUS "Configuring for Linux x86_64")
        set(QT_PLATFORM_DIR "/home/akbailey/Qt/6.10.3/gcc_64")
        set(FILEFINDER_LINUXDEPLOY_ARCH "x86_64")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
        message(STATUS "Configuring for Linux ARM64 / AArch64")
        set(QT_PLATFORM_DIR "/home/akbailey/Qt/6.10.3/gcc_arm64")
        set(FILEFINDER_LINUXDEPLOY_ARCH "aarch64")
    else()
        message(FATAL_ERROR "Unsupported Linux architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    set(FILEFINDER_QT_CMAKE_DIR "${QT_PLATFORM_DIR}/lib/cmake")
    set(CMAKE_PREFIX_PATH "${QT_PLATFORM_DIR}" CACHE PATH "Qt installation prefix" FORCE)
    set(Qt6_DIR "${FILEFINDER_QT_CMAKE_DIR}/Qt6" CACHE PATH "Qt6 package directory" FORCE)

    foreach(FILEFINDER_QT_PACKAGE IN ITEMS
            Qt6Core
            Qt6CoreTools
            Qt6DBus
            Qt6DBusTools
            Qt6Gui
            Qt6GuiTools
            Qt6Widgets
            Qt6WidgetsTools
    )
        if (EXISTS "${FILEFINDER_QT_CMAKE_DIR}/${FILEFINDER_QT_PACKAGE}")
            set(${FILEFINDER_QT_PACKAGE}_DIR "${FILEFINDER_QT_CMAKE_DIR}/${FILEFINDER_QT_PACKAGE}"
                    CACHE PATH "${FILEFINDER_QT_PACKAGE} package directory" FORCE)
        else()
            unset(${FILEFINDER_QT_PACKAGE}_DIR CACHE)
        endif()
    endforeach()

    # Force the rpath you want, and don't append link directories
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
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

set(FILEFINDER_TARGET_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
if (CMAKE_VS_PLATFORM_NAME)
    set(FILEFINDER_TARGET_PROCESSOR "${CMAKE_VS_PLATFORM_NAME}")
endif()

set(FILEFINDER_COMPILED_FOR "${CMAKE_SYSTEM_NAME} ${FILEFINDER_TARGET_PROCESSOR}")
set(FILEFINDER_COMPILED_ON "${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}")
set(FILEFINDER_COMPILED_WITH "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
string(TIMESTAMP FILEFINDER_BUILD_DATE "%Y-%m-%d")

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "FILEFINDER_DESKTOP_FILE_NAME=\"${FILEFINDER_APPSTREAM_ID}\""
        "FILEFINDER_COMPILED_FOR=\"${FILEFINDER_COMPILED_FOR}\""
        "FILEFINDER_COMPILED_ON=\"${FILEFINDER_COMPILED_ON}\""
        "FILEFINDER_BUILD_DATE=\"${FILEFINDER_BUILD_DATE}\""
        "FILEFINDER_COMPILED_WITH=\"${FILEFINDER_COMPILED_WITH}\""
)

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
            MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
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

# Resolve Qt dirs
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(_qt_arch_dir "${QT_PLATFORM_DIR}")
else()
    get_filename_component(_qt_cmake_dir "${Qt6_DIR}" DIRECTORY)        # .../lib/cmake
    get_filename_component(_qt_lib_dir   "${_qt_cmake_dir}/.." ABSOLUTE) # .../lib
    get_filename_component(_qt_arch_dir  "${_qt_lib_dir}/.." ABSOLUTE)   # .../gcc_64 or .../msvc2022_64
endif()
set(_qt_plugins_dir "${_qt_arch_dir}/plugins")
set(_qt_bin_dir "${_qt_arch_dir}/bin")

# Compatibility with older CLion profiles that still pass -DDEPLOYQT.
if (DEFINED DEPLOYQT)
    set(_filefinder_legacy_deployqt "${DEPLOYQT}")
endif()

# For the Windows and MacOS applications to work, the files must be in certain locations.  The .\Release directory has all the
# files needed to execute the application
if (WIN32)
    # Windows and MacOS needs Unicode
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    target_link_libraries(${PROJECT_NAME} PRIVATE Advapi32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    # Find windeployqt.exe in the Qt bin dir. Use a project-local variable so stale CMake cache entries do not pin an old Qt version.
    find_program(_filefinder_windeployqt windeployqt.exe
            HINTS "${_qt_bin_dir}" "C:/Development/Qt/6.10.3/msvc2022_64/bin"
            NO_CACHE
    )

    if (NOT _filefinder_windeployqt)
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
            -DDEPLOYQT:FILEPATH=${_filefinder_windeployqt}
            -DAPP=$<TARGET_FILE:${PROJECT_NAME}>
            -DCONFIG=$<CONFIG>
            -P "${CMAKE_CURRENT_LIST_DIR}/cmake/run_deployqt_debug.cmake"
            VERBATIM
            COMMENT "deployqt (Debug only)"
    )
elseif (APPLE)
    # Windows and MacOS needs Unicode
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

    # Find macdeployqt in the Qt bin dir. Use a project-local variable so stale CMake cache entries do not pin an old Qt version.
    find_program(_filefinder_macdeployqt macdeployqt
            HINTS "${_qt_bin_dir}" "/Library/Qt/6.10.3/macos/bin"
            NO_CACHE
    )

    if (NOT _filefinder_macdeployqt)
        message(FATAL_ERROR "macdeployqt not found. Please check Qt6_DIR or install Qt tools.")
    endif()

    # Run macdeployqt on the .app bundle itself
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${_filefinder_macdeployqt}" "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>" -verbose=1
            COMMENT "Running macdeployqt on ${PROJECT_NAME}.app"
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_program(LINUXDEPLOY_EXECUTABLE
            NAMES linuxdeploy linuxdeploy-x86_64.AppImage linuxdeploy-aarch64.AppImage linuxdeploy-arm64.AppImage
            HINTS "${QT_PLATFORM_DIR}/bin"
            DOC "Path to linuxdeploy"
    )

    if (NOT LINUXDEPLOY_EXECUTABLE)
        message(FATAL_ERROR "linuxdeploy not found. Expected it in ${QT_PLATFORM_DIR}/bin, or set LINUXDEPLOY_EXECUTABLE.")
    endif()

    find_program(LINUXDEPLOY_QT_PLUGIN
            NAMES linuxdeploy-plugin-qt linuxdeploy-plugin-qt-x86_64.AppImage linuxdeploy-plugin-qt-aarch64.AppImage linuxdeploy-plugin-qt-arm64.AppImage
            HINTS "${QT_PLATFORM_DIR}/bin"
            DOC "Path to linuxdeploy-plugin-qt"
    )

    if (NOT LINUXDEPLOY_QT_PLUGIN)
        message(FATAL_ERROR "linuxdeploy-plugin-qt not found. Expected it in ${QT_PLATFORM_DIR}/bin, or set LINUXDEPLOY_QT_PLUGIN.")
    endif()

    find_program(LINUXDEPLOY_QMAKE
            NAMES qmake6 qmake
            HINTS "${QT_PLATFORM_DIR}/bin"
            DOC "Path to the Qt qmake executable used by linuxdeploy-plugin-qt"
    )

    if (NOT LINUXDEPLOY_QMAKE)
        message(FATAL_ERROR "qmake not found. Expected qmake or qmake6 in ${QT_PLATFORM_DIR}/bin, or set LINUXDEPLOY_QMAKE.")
    endif()

    find_program(CHMOD_EXECUTABLE chmod
            DOC "Path to chmod"
    )

    if (NOT CHMOD_EXECUTABLE)
        message(FATAL_ERROR "chmod not found. It is required to make the AppImage AppRun launcher executable.")
    endif()

    find_program(APPSTREAMCLI_EXECUTABLE appstreamcli
            DOC "Path to appstreamcli, used to validate AppImage AppStream metadata"
    )

    if (NOT APPSTREAMCLI_EXECUTABLE)
        message(FATAL_ERROR "appstreamcli not found. It is required to validate AppImage AppStream metadata.")
    endif()

    set(FILEFINDER_LINUXDEPLOY_EXCLUDED_LIBRARIES
            "libglib-2.0.so*"
            "libgthread-2.0.so*"
    )

    set(FILEFINDER_LINUXDEPLOY_EXCLUDE_ARGS)
    foreach(FILEFINDER_LINUXDEPLOY_EXCLUDED_LIBRARY IN LISTS FILEFINDER_LINUXDEPLOY_EXCLUDED_LIBRARIES)
        list(APPEND FILEFINDER_LINUXDEPLOY_EXCLUDE_ARGS
                --exclude-library "${FILEFINDER_LINUXDEPLOY_EXCLUDED_LIBRARY}"
        )
    endforeach()

    set(FILEFINDER_LINUX_DESKTOP_ID "${FILEFINDER_APPSTREAM_ID}.desktop")
    set(FILEFINDER_APPDIR "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.AppDir")
    set(FILEFINDER_LINUX_SOURCE_DESKTOP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILEFINDER_LINUX_DESKTOP_ID}")
    set(FILEFINDER_LINUX_APPDIR_DESKTOP_FILE "${FILEFINDER_APPDIR}/usr/share/applications/${FILEFINDER_LINUX_DESKTOP_ID}")
    set(FILEFINDER_LINUX_SOURCE_ICON_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILEFINDER_LINUX_ICON_NAME}.png")
    set(FILEFINDER_LINUX_SOURCE_APPSTREAM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILEFINDER_APPSTREAM_ID}.metainfo.xml")
    set(FILEFINDER_LINUX_APPDIR_APPSTREAM_FILE "${FILEFINDER_APPDIR}/usr/share/metainfo/${FILEFINDER_APPSTREAM_ID}.metainfo.xml")
    file(WRITE "${FILEFINDER_LINUX_SOURCE_DESKTOP_FILE}"
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=${FILEFINDER_DISPLAY_NAME}\n"
            "Comment=Find files by name within a selected directory tree\n"
            "Exec=${PROJECT_NAME}\n"
            "Icon=${FILEFINDER_LINUX_ICON_NAME}\n"
            "StartupWMClass=${PROJECT_NAME}\n"
            "Categories=Utility;\n"
    )

    file(WRITE "${FILEFINDER_LINUX_SOURCE_APPSTREAM_FILE}"
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<component type=\"desktop-application\">\n"
            "  <id>${FILEFINDER_LINUX_DESKTOP_ID}</id>\n"
            "  <metadata_license>CC0-1.0</metadata_license>\n"
            "  <project_license>MIT</project_license>\n"
            "  <developer id=\"${FILEFINDER_DEVELOPER_ID}\">\n"
            "    <name>${FILEFINDER_DEVELOPER_NAME}</name>\n"
            "  </developer>\n"
            "  <name>${FILEFINDER_DISPLAY_NAME}</name>\n"
            "  <summary>Find files by name within a selected directory tree</summary>\n"
            "  <description>\n"
            "    <p>${FILEFINDER_DISPLAY_NAME} searches a selected directory tree for a target filename and lists matching full paths.</p>\n"
            "  </description>\n"
            "  <launchable type=\"desktop-id\">${FILEFINDER_LINUX_DESKTOP_ID}</launchable>\n"
            "  <url type=\"homepage\">https://github.com/akevinbailey/FileFinder</url>\n"
            "  <provides>\n"
            "    <binary>${PROJECT_NAME}</binary>\n"
            "  </provides>\n"
            "  <categories>\n"
            "    <category>Utility</category>\n"
            "  </categories>\n"
            "  <releases>\n"
            "    <release version=\"${PROJECT_VERSION}\" date=\"${FILEFINDER_BUILD_DATE}\" />\n"
            "  </releases>\n"
            "  <content_rating type=\"oars-1.1\" />\n"
            "</component>\n"
    )

    set(FILEFINDER_APPIMAGE_NAME "${PROJECT_NAME}.AppImage")
    set(FILEFINDER_APPIMAGE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILEFINDER_APPIMAGE_NAME}")
    set(FILEFINDER_LINUXDEPLOY_QT_PLUGIN_LINK "${CMAKE_CURRENT_BINARY_DIR}/linuxdeploy-plugin-qt")
    set(FILEFINDER_QT_PLUGIN_FILTER_DIR "${CMAKE_CURRENT_BINARY_DIR}/qt-plugins-for-appimage")
    set(FILEFINDER_QMAKE_WRAPPER "${CMAKE_CURRENT_BINARY_DIR}/qmake-for-linuxdeploy")
    set(FILEFINDER_APPRUN "${CMAKE_CURRENT_BINARY_DIR}/AppRun")

    # linuxdeploy-plugin-qt deploys every Qt imageformat plugin it sees. This Qt
    # install has libqtiff.so linked to libtiff.so.5, which is not required by
    # FileFinder and may not exist on the deploy host. Give linuxdeploy a
    # filtered plugin tree instead of modifying the Qt installation.
    set(FILEFINDER_QT_PLUGIN_FILTER_COMMANDS)
    file(GLOB FILEFINDER_QT_PLUGIN_ENTRIES LIST_DIRECTORIES true "${_qt_plugins_dir}/*")
    foreach(FILEFINDER_QT_PLUGIN_ENTRY IN LISTS FILEFINDER_QT_PLUGIN_ENTRIES)
        get_filename_component(FILEFINDER_QT_PLUGIN_ENTRY_NAME "${FILEFINDER_QT_PLUGIN_ENTRY}" NAME)

        if (FILEFINDER_QT_PLUGIN_ENTRY_NAME STREQUAL "imageformats")
            list(APPEND FILEFINDER_QT_PLUGIN_FILTER_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${FILEFINDER_QT_PLUGIN_FILTER_DIR}/imageformats"
            )

            file(GLOB FILEFINDER_QT_IMAGEFORMAT_PLUGIN_ENTRIES "${FILEFINDER_QT_PLUGIN_ENTRY}/*.so*")
            foreach(FILEFINDER_QT_IMAGEFORMAT_PLUGIN_ENTRY IN LISTS FILEFINDER_QT_IMAGEFORMAT_PLUGIN_ENTRIES)
                get_filename_component(FILEFINDER_QT_IMAGEFORMAT_PLUGIN_NAME "${FILEFINDER_QT_IMAGEFORMAT_PLUGIN_ENTRY}" NAME)
                if (NOT FILEFINDER_QT_IMAGEFORMAT_PLUGIN_NAME MATCHES "^libqtiff\\.so")
                    list(APPEND FILEFINDER_QT_PLUGIN_FILTER_COMMANDS
                            COMMAND ${CMAKE_COMMAND} -E create_symlink
                                    "${FILEFINDER_QT_IMAGEFORMAT_PLUGIN_ENTRY}"
                                    "${FILEFINDER_QT_PLUGIN_FILTER_DIR}/imageformats/${FILEFINDER_QT_IMAGEFORMAT_PLUGIN_NAME}"
                    )
                endif()
            endforeach()
        else()
            list(APPEND FILEFINDER_QT_PLUGIN_FILTER_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E create_symlink
                            "${FILEFINDER_QT_PLUGIN_ENTRY}"
                            "${FILEFINDER_QT_PLUGIN_FILTER_DIR}/${FILEFINDER_QT_PLUGIN_ENTRY_NAME}"
            )
        endif()
    endforeach()

    file(WRITE "${FILEFINDER_QMAKE_WRAPPER}"
            "#!/usr/bin/env sh\n"
            "if [ \"$1\" = \"-query\" ] && [ \"$2\" = \"QT_INSTALL_PLUGINS\" ]; then\n"
            "    printf '%s\\n' '${FILEFINDER_QT_PLUGIN_FILTER_DIR}'\n"
            "    exit 0\n"
            "fi\n"
            "if [ \"$1\" = \"-query\" ] && [ \"$#\" -eq 1 ]; then\n"
            "    '${LINUXDEPLOY_QMAKE}' \"$@\" | sed 's|^QT_INSTALL_PLUGINS:.*$|QT_INSTALL_PLUGINS:${FILEFINDER_QT_PLUGIN_FILTER_DIR}|'\n"
            "    exit $?\n"
            "fi\n"
            "exec '${LINUXDEPLOY_QMAKE}' \"$@\"\n"
    )
    file(CHMOD "${FILEFINDER_QMAKE_WRAPPER}"
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

    file(WRITE "${FILEFINDER_APPRUN}" [=[
#!/usr/bin/env sh
set -eu

if [ -z "${APPDIR:-}" ]; then
    this="$0"
    case "${this}" in
        /*) ;;
        *) this="$(pwd)/${this}" ;;
    esac
    APPDIR="$(CDPATH= cd -- "$(dirname -- "${this}")" && pwd)"
fi
export APPDIR

export PATH="${APPDIR}/usr/bin:${PATH:-}"
export LD_LIBRARY_PATH="${APPDIR}/usr/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export QT_PLUGIN_PATH="${APPDIR}/usr/plugins"
export QML2_IMPORT_PATH="${APPDIR}/usr/qml"
export XDG_DATA_DIRS="${APPDIR}/usr/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

if [ -n "${WAYLAND_DISPLAY:-}" ] && [ -z "${QT_QPA_PLATFORM:-}" ]; then
    export QT_QPA_PLATFORM='wayland;xcb'
fi

if [ -z "${QT_QPA_PLATFORMTHEME:-}" ]; then
    case "${XDG_CURRENT_DESKTOP:-}" in
        *GNOME*|*Unity*|*Cinnamon*|*MATE*)
            export QT_QPA_PLATFORMTHEME=gtk3
            ;;
    esac
fi

]=])
    file(APPEND "${FILEFINDER_APPRUN}" "exec \"\${APPDIR}/usr/bin/${PROJECT_NAME}\" \"\$@\"\n")
    file(CHMOD "${FILEFINDER_APPRUN}"
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

    set(FILEFINDER_EXTRA_QT_RUNTIME_COMMANDS)

    set(FILEFINDER_REQUIRED_QT_PLUGIN_FILES
            "${_qt_plugins_dir}/platforms/libqwayland.so"
    )

    set(FILEFINDER_OPTIONAL_QT_PLUGIN_FILES
            "${_qt_plugins_dir}/platformthemes/libqgtk3.so"
    )

    foreach(FILEFINDER_REQUIRED_QT_PLUGIN_FILE IN LISTS FILEFINDER_REQUIRED_QT_PLUGIN_FILES)
        if (NOT EXISTS "${FILEFINDER_REQUIRED_QT_PLUGIN_FILE}")
            message(FATAL_ERROR "Required Qt plugin not found: ${FILEFINDER_REQUIRED_QT_PLUGIN_FILE}")
        endif()
    endforeach()

    foreach(FILEFINDER_QT_PLUGIN_FILE IN LISTS FILEFINDER_REQUIRED_QT_PLUGIN_FILES FILEFINDER_OPTIONAL_QT_PLUGIN_FILES)
        if (EXISTS "${FILEFINDER_QT_PLUGIN_FILE}")
            get_filename_component(FILEFINDER_QT_PLUGIN_DIRECTORY "${FILEFINDER_QT_PLUGIN_FILE}" DIRECTORY)
            get_filename_component(FILEFINDER_QT_PLUGIN_DIRECTORY_NAME "${FILEFINDER_QT_PLUGIN_DIRECTORY}" NAME)
            get_filename_component(FILEFINDER_QT_PLUGIN_NAME "${FILEFINDER_QT_PLUGIN_FILE}" NAME)
            list(APPEND FILEFINDER_EXTRA_QT_RUNTIME_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${FILEFINDER_APPDIR}/usr/plugins/${FILEFINDER_QT_PLUGIN_DIRECTORY_NAME}"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${FILEFINDER_QT_PLUGIN_FILE}"
                            "${FILEFINDER_APPDIR}/usr/plugins/${FILEFINDER_QT_PLUGIN_DIRECTORY_NAME}/${FILEFINDER_QT_PLUGIN_NAME}"
            )
        endif()
    endforeach()

    foreach(FILEFINDER_QT_WAYLAND_PLUGIN_DIRECTORY IN ITEMS
            wayland-decoration-client
            wayland-graphics-integration-client
            wayland-shell-integration
    )
        file(GLOB FILEFINDER_QT_WAYLAND_PLUGIN_FILES
                "${_qt_plugins_dir}/${FILEFINDER_QT_WAYLAND_PLUGIN_DIRECTORY}/*.so"
        )

        if (FILEFINDER_QT_WAYLAND_PLUGIN_FILES)
            list(APPEND FILEFINDER_EXTRA_QT_RUNTIME_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${FILEFINDER_APPDIR}/usr/plugins/${FILEFINDER_QT_WAYLAND_PLUGIN_DIRECTORY}"
            )

            foreach(FILEFINDER_QT_WAYLAND_PLUGIN_FILE IN LISTS FILEFINDER_QT_WAYLAND_PLUGIN_FILES)
                get_filename_component(FILEFINDER_QT_WAYLAND_PLUGIN_NAME "${FILEFINDER_QT_WAYLAND_PLUGIN_FILE}" NAME)
                list(APPEND FILEFINDER_EXTRA_QT_RUNTIME_COMMANDS
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                "${FILEFINDER_QT_WAYLAND_PLUGIN_FILE}"
                                "${FILEFINDER_APPDIR}/usr/plugins/${FILEFINDER_QT_WAYLAND_PLUGIN_DIRECTORY}/${FILEFINDER_QT_WAYLAND_PLUGIN_NAME}"
                )
            endforeach()
        endif()
    endforeach()

    foreach(FILEFINDER_QT_RUNTIME_LIBRARY IN ITEMS
            libQt6WaylandClient.so.6
            libQt6OpenGL.so.6
    )
        set(FILEFINDER_QT_RUNTIME_LIBRARY_PATH "${QT_PLATFORM_DIR}/lib/${FILEFINDER_QT_RUNTIME_LIBRARY}")
        if (EXISTS "${FILEFINDER_QT_RUNTIME_LIBRARY_PATH}")
            get_filename_component(FILEFINDER_QT_RUNTIME_LIBRARY_REALPATH "${FILEFINDER_QT_RUNTIME_LIBRARY_PATH}" REALPATH)
            list(APPEND FILEFINDER_EXTRA_QT_RUNTIME_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${FILEFINDER_QT_RUNTIME_LIBRARY_REALPATH}"
                            "${FILEFINDER_APPDIR}/usr/lib/${FILEFINDER_QT_RUNTIME_LIBRARY}"
            )
        endif()
    endforeach()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${FILEFINDER_APPDIR}"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${FILEFINDER_APPIMAGE_FILE}"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${FILEFINDER_LINUXDEPLOY_QT_PLUGIN_LINK}"
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${LINUXDEPLOY_QT_PLUGIN}" "${FILEFINDER_LINUXDEPLOY_QT_PLUGIN_LINK}"
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${FILEFINDER_QT_PLUGIN_FILTER_DIR}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${FILEFINDER_QT_PLUGIN_FILTER_DIR}"
            ${FILEFINDER_QT_PLUGIN_FILTER_COMMANDS}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${FILEFINDER_APPDIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_SOURCE_DIR}/resources/${PROJECT_NAME}.png"
                    "${FILEFINDER_LINUX_SOURCE_ICON_FILE}"
            COMMAND ${CMAKE_COMMAND} -E env
                    "PATH=${CMAKE_CURRENT_BINARY_DIR}:${QT_PLATFORM_DIR}/bin:$ENV{PATH}"
                    "LD_LIBRARY_PATH=${QT_PLATFORM_DIR}/lib:$ENV{LD_LIBRARY_PATH}"
                    "${LINUXDEPLOY_EXECUTABLE}"
                    --appdir "${FILEFINDER_APPDIR}"
                    --executable "$<TARGET_FILE:${PROJECT_NAME}>"
                    --desktop-file "${FILEFINDER_LINUX_SOURCE_DESKTOP_FILE}"
                    --icon-file "${FILEFINDER_LINUX_SOURCE_ICON_FILE}"
                    ${FILEFINDER_LINUXDEPLOY_EXCLUDE_ARGS}
            COMMAND ${CMAKE_COMMAND} -E env
                    "QMAKE=${FILEFINDER_QMAKE_WRAPPER}"
                    "PATH=${CMAKE_CURRENT_BINARY_DIR}:${QT_PLATFORM_DIR}/bin:$ENV{PATH}"
                    "LD_LIBRARY_PATH=${QT_PLATFORM_DIR}/lib:$ENV{LD_LIBRARY_PATH}"
                    "${FILEFINDER_LINUXDEPLOY_QT_PLUGIN_LINK}"
                    --appdir "${FILEFINDER_APPDIR}"
                    ${FILEFINDER_LINUXDEPLOY_EXCLUDE_ARGS}
            ${FILEFINDER_EXTRA_QT_RUNTIME_COMMANDS}
            COMMAND ${CMAKE_COMMAND} -E rm -f "${FILEFINDER_APPDIR}/usr/plugins/imageformats/libqtiff.so"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${FILEFINDER_APPDIR}/usr/lib/libglib-2.0.so.0"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${FILEFINDER_APPDIR}/usr/lib/libgthread-2.0.so.0"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${FILEFINDER_APPDIR}/AppRun"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FILEFINDER_APPRUN}" "${FILEFINDER_APPDIR}/AppRun"
            COMMAND "${CHMOD_EXECUTABLE}" 755 "${FILEFINDER_APPDIR}/AppRun"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${FILEFINDER_APPDIR}/usr/share/metainfo"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${FILEFINDER_LINUX_SOURCE_APPSTREAM_FILE}"
                    "${FILEFINDER_LINUX_APPDIR_APPSTREAM_FILE}"
            COMMAND ${CMAKE_COMMAND} -E env
                    "ARCH=${FILEFINDER_LINUXDEPLOY_ARCH}"
                    "LDAI_OUTPUT=${FILEFINDER_APPIMAGE_FILE}"
                    "PATH=${CMAKE_CURRENT_BINARY_DIR}:${QT_PLATFORM_DIR}/bin:$ENV{PATH}"
                    "LD_LIBRARY_PATH=${QT_PLATFORM_DIR}/lib:$ENV{LD_LIBRARY_PATH}"
                    "${LINUXDEPLOY_EXECUTABLE}"
                    --appdir "${FILEFINDER_APPDIR}"
                    --desktop-file "${FILEFINDER_LINUX_APPDIR_DESKTOP_FILE}"
                    ${FILEFINDER_LINUXDEPLOY_EXCLUDE_ARGS}
                    --output appimage
            BYPRODUCTS "${FILEFINDER_APPIMAGE_FILE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            VERBATIM
            COMMENT "Building ${FILEFINDER_APPIMAGE_NAME}"
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
