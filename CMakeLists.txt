# libjgame with CMake: what an Android build (Gradle drives the NDK through CMake)
# and other CMake projects use. The Makefile stays the way to build it on a
# computer; the two build the same sources.
#
# In a CMake project:
#
#   add_subdirectory(libjgame)
#   target_link_libraries(mygame PRIVATE jgame)
#
# SDL2 is the target SDL2::SDL2 (or SDL2) if the project has one already (an
# Android build adds SDL's sources with add_subdirectory() first), otherwise
# the SDL2 installed on the system; zlib is the system's (the NDK has one).
cmake_minimum_required(VERSION 3.16)
project(jgame VERSION 4.0 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR AND NOT ANDROID)
	set(JGAME_TESTS_DEFAULT ON)
else()
	set(JGAME_TESTS_DEFAULT OFF)
endif()
option(JGAME_BUILD_TESTS "Build the tests in test/ (run them with ctest)" ${JGAME_TESTS_DEFAULT})

file(GLOB JGAME_SOURCES CONFIGURE_DEPENDS
	${PROJECT_SOURCE_DIR}/*.cpp
	${PROJECT_SOURCE_DIR}/audio/*.cpp
	${PROJECT_SOURCE_DIR}/graphics/*.cpp
	${PROJECT_SOURCE_DIR}/input/*.cpp
	${PROJECT_SOURCE_DIR}/streams/*.cpp
	${PROJECT_SOURCE_DIR}/support/*.cpp
	${PROJECT_SOURCE_DIR}/timers/*.cpp)

add_library(jgame STATIC ${JGAME_SOURCES})
add_library(jgame::jgame ALIAS jgame)
target_include_directories(jgame PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(jgame PUBLIC cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(jgame PRIVATE -Wall)
endif()

if(TARGET SDL2::SDL2)
	set(JGAME_SDL SDL2::SDL2)
elseif(TARGET SDL2)
	set(JGAME_SDL SDL2)
else()
	find_package(SDL2 REQUIRED)
	if(TARGET SDL2::SDL2)
		set(JGAME_SDL SDL2::SDL2)
	else()
		# An SDL2 whose CMake files have no targets (old ones): its variables
		target_include_directories(jgame PUBLIC ${SDL2_INCLUDE_DIRS})
		set(JGAME_SDL ${SDL2_LIBRARIES})
	endif()
endif()
find_package(ZLIB REQUIRED)
target_link_libraries(jgame PUBLIC ${JGAME_SDL} ZLIB::ZLIB)

if(JGAME_BUILD_TESTS)
	enable_testing()
	file(GLOB JGAME_TESTS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/test/*.cpp)
	foreach(test_source ${JGAME_TESTS})
		get_filename_component(test_name ${test_source} NAME_WE)
		add_executable(${test_name} ${test_source})
		target_link_libraries(${test_name} PRIVATE jgame)
		add_test(NAME ${test_name} COMMAND ${test_name})
		# No screen and no sound card needed
		set_tests_properties(${test_name} PROPERTIES
			ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy")
	endforeach()
endif()
