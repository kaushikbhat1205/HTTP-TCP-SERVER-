cmake_minimum_required(VERSION 3.13)

project(http-server-starter-cpp)

# Enable the C++23 standard
set(CMAKE_CXX_STANDARD 23)

# Set the Vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE /Users/kaushikbhat/vcpkg/scripts/buildsystems/vcpkg.cmake)

# Optionally set compilers (if needed)
# set(CMAKE_C_COMPILER /Library/Developer/CommandLineTools/usr/bin/gcc)
# set(CMAKE_CXX_COMPILER /Library/Developer/CommandLineTools/usr/bin/g++)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(server ${SOURCE_FILES})

target_link_libraries(server PRIVATE Threads::Threads ZLIB::ZLIB)
