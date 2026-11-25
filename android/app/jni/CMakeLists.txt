# The native side of the APK: SDL2 (libSDL2.so) from its sources, then the game,
# whose CMakeLists.txt must make a shared library called `main` (libmain.so: what
# SDLActivity loads and runs) linked with jgame. Gradle gives the two folders.
cmake_minimum_required(VERSION 3.16)
project(jgame_android LANGUAGES C CXX)

if(NOT JGAME_SDL_DIR OR NOT JGAME_GAME_DIR)
	message(FATAL_ERROR "Build this through Gradle (see libjgame/android/README.md)")
endif()

# SDL2's own CMake build: the shared library only
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(${JGAME_SDL_DIR} SDL)

add_subdirectory(${JGAME_GAME_DIR} game)

if(NOT TARGET main)
	message(FATAL_ERROR "${JGAME_GAME_DIR}/CMakeLists.txt has no `main` library: on Android the game "
		"is add_library(main SHARED ...) instead of an executable")
endif()
