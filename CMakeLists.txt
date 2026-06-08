cmake_minimum_required(VERSION 3.22)

cmake_policy(SET CMP0091 NEW)

if(WIN32)
	set(VCPKG_TARGET_TRIPLET "x64-windows-static")
	set(VCPKG_HOST_TRIPLET "x64-windows-static")
elseif (LINUX)
	set(VCPKG_TARGET_TRIPLET "x64-linux")
endif()

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

set(VCPKG_FEATURE_FLAGS versions)

project(so5_lor_two_thread_demo_ru LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(sobjectizer CONFIG REQUIRED)

add_subdirectory(two_thread_demo)
add_subdirectory(two_thread_demo_2)
add_subdirectory(two_thread_demo_3)

