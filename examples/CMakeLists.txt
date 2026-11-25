# The example as a CMake project: on a computer the program `minimal`, on
# Android the libmain.so of an APK (libjgame/android builds it with
# ./gradlew assembleDebug -Pjgame.game=../examples)
cmake_minimum_required(VERSION 3.16)
project(jgame_minimal LANGUAGES CXX)

if(NOT TARGET jgame)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. jgame)
endif()

if(ANDROID)
	set(MINIMAL_TARGET main)
	add_library(main SHARED minimal.cpp)
else()
	set(MINIMAL_TARGET minimal)
	add_executable(minimal minimal.cpp)
endif()
target_link_libraries(${MINIMAL_TARGET} PRIVATE jgame)
