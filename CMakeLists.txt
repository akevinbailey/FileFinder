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
    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "C:/Development/Qt/6.9.3/msvc2022_64")
    endif()
    if (NOT DEFINED Qt6_DIR)
        set(Qt6_DIR "C:/Development/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6")
    endif()
    if (NOT DEFINED DEPLOY_QT)
        set(DEPLOY_QT "C:/Development/Qt/6.9.3/msvc2022_64/bin/windeployqt6.exe")
    endif()
else() # Linux
    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "/home/akbailey/Qt/6.9.3/gcc_64")
    endif()
    if (NOT DEFINED Qt6_DIR)
        set(Qt6_DIR "/home/akbailey/Qt/6.9.3/gcc_64/lib/cmake/Qt6")
    endif()
    if (NOT DEFINED DEPLOY_QT)
        set(DEPLOY_QT "") # not used on Linux
    endif()
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

# Windows defines (Unicode)
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
endif()

# -------- Optional deploy steps --------
# Windows: run windeployqt after build (dynamic Qt)
if (WIN32 AND DEPLOY_QT AND EXISTS "${DEPLOY_QT}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
            COMMAND "${DEPLOY_QT}" --release --no-translations --no-compiler-runtime
            "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Deploying Qt runtime with windeployqt"
    )
endif()

# Linux: help the built app find Qt libs when Qt is in a custom prefix
# (Adjust/remove if you prefer to set LD_LIBRARY_PATH externally.)
if (UNIX AND NOT APPLE)
    get_filename_component(_qt_cmake_dir "${Qt6_DIR}" DIRECTORY)
    get_filename_component(_qt_lib_dir   "${_qt_cmake_dir}/lib" ABSOLUTE)
    set(CMAKE_BUILD_RPATH "${_qt_lib_dir}")
    set(CMAKE_INSTALL_RPATH "${_qt_lib_dir}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
endif()
