cmake_minimum_required(VERSION 3.16)
project(TomatoTimer LANGUAGES C CXX RC)

set(CMAKE_MFC_FLAG 2)


set(CMAKE_UNITY_BUILD OFF)

add_definitions(-DUNICODE -D_UNICODE)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")

add_executable(TomatoTimer WIN32
    "${SRC_DIR}/TomatoTimerApp.h"
    "${SRC_DIR}/TomatoTimerApp.cpp"
    "${SRC_DIR}/TomatoTimerDlg.h"
    "${SRC_DIR}/TomatoTimerDlg.cpp"
    "${SRC_DIR}/resource.h"
    "${RES_DIR}/TomatoTimer.rc"
    "sqlite/sqlite3.c"
)

target_compile_features(TomatoTimer PRIVATE cxx_std_17)
target_include_directories(TomatoTimer PRIVATE "${SRC_DIR}" "${RES_DIR}" "sqlite")

# SQLite3 is now compiled directly from source
# find_package(SQLite3 REQUIRED)
# target_link_libraries(TomatoTimer PRIVATE SQLite::SQLite3)
