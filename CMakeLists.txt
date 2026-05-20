cmake_minimum_required(VERSION 3.20)
project(OECONOMICS CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Fetch Raylib
include(FetchContent)
FetchContent_Declare(raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG master
)
FetchContent_MakeAvailable(raylib)

add_executable(oeconomics
    src/main.cpp
    src/systems.cpp
    src/simulation.cpp
    src/gui_engine.cpp
)

# Use pthread se necessário (Linux)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(oeconomics
    PRIVATE raylib
    PRIVATE Threads::Threads
)
