cmake_minimum_required(VERSION 4.0)
project(FileFinder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Development/Qt/6.9.2/mingw_64")
set(Qt6_DIR "C:/Development/Qt/6.9.2/mingw_64/lib/cmake/Qt6")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Adjust if your Qt is not in a default location
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Source / include layout
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "${SRC_DIR}/*.h")

# Resources (QRC)
set(APP_RESOURCES "${CMAKE_SOURCE_DIR}/resources/app.qrc")
qt_add_resources(APP_QRC ${APP_RESOURCES})

add_executable(${PROJECT_NAME} WIN32
        ${APP_SOURCES}
        ${APP_HEADERS}
        ${APP_QRC}
)

target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/resources/appicon.rc)
target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
        COMMAND "C:/Development/Qt/6.9.2/mingw_64/bin/windeployqt.exe"
        --no-translations
        --no-compiler-runtime
        "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt runtime with windeployqt"
)

## The --relese directives puts all the static libraries in the build folder.  Once built, you must delete the
## cmake-build-debug-mingw-64 and cmake-build-release-mingw-64 directories to update the CMakeLists.txt or rebuild
## the application.

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
#        COMMAND "C:/Development/Qt/6.9.2/mingw_64/bin/windeployqt.exe"
#        --release
#        "$<TARGET_FILE:${PROJECT_NAME}>"
#        COMMENT "Deploying Qt runtime with windeployqt"
#)