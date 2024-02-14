cmake_minimum_required(VERSION 3.8)

cmake_policy(SET CMP0074 NEW)

if(NOT CMAKE_BUILD_TYPE)
	message("Defaulting build type type to Release...")
	set(CMAKE_BUILD_TYPE Release)
endif()

project("similarity")

option(KERNIGHAN "KERNIGHAN" OFF)

set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_DEBUG_LIBS OFF)

if(CONFIRM_BOOST_COMPONENTS)
	find_package(Boost REQUIRED COMPONENTS chrono system)
else()
	message("Bypassing check for boost components (broken on recent Windows builds)")
	message("Note that a header-only install of boost will result in link failures")
	find_package(Boost REQUIRED)
endif()

set(COMMON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../common/cpp/include")

add_executable(similarity
	"main.cpp"
	"${COMMON_INCLUDE_DIR}/profile.hpp"
	"${COMMON_INCLUDE_DIR}/profile_config.hpp"
	"${COMMON_INCLUDE_DIR}/collector/json.hpp"
)

target_include_directories(similarity PUBLIC ${COMMON_INCLUDE_DIR})

if(Boost_FOUND)
	message(Boost_INCLUDE_DIRS="${Boost_INCLUDE_DIRS}")
	message(Boost_LIBRARY_DIRS="${Boost_LIBRARY_DIRS}")
	message(boost_LIBRARY_SEARCH_DIRS_RELEASE="${boost_LIBRARY_SEARCH_DIRS_RELEASE}")
	message(_boost_LIBRARY_SEARCH_DIRS_RELEASE="${_boost_LIBRARY_SEARCH_DIRS_RELEASE}")
	target_include_directories(similarity PUBLIC ${Boost_INCLUDE_DIRS})
	target_link_directories(similarity PUBLIC ${Boost_LIBRARY_DIRS})
else()
	if(MSVC)
		message(FATAL ERROR "Boost installation not found")
	else()
		message(WARNING "Boost installation not found")
		message(WARNING "Proceeding with assumption boost is in system paths...")
	endif()
endif()

if(MSVC)
	message("Boost libraries are assumed to auto-link...")
else()	
	target_link_libraries(similarity boost_chrono boost_system boost_filesystem)
endif()

target_compile_definitions(similarity PUBLIC BOOST_ERROR_CODE_HEADER_ONLY)

if(KERNIGHAN)
	target_compile_definitions(similarity PUBLIC USE_KERNIGHAN_BIT_COUNT_ALGORITHM)
endif()

if(MSVC)
	target_compile_definitions(similarity PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

message(CMAKE_CXX_FLAGS_RELEASE="${CMAKE_CXX_FLAGS_RELEASE}")
message(CMAKE_CXX_FLAGS_DEBUG="${CMAKE_CXX_FLAGS_DEBUG}")

if(MSVC)
	string(REGEX REPLACE "/O[^ ]*[ ]*" "" DEOPTIMIZED_CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_CXX_FLAGS_RELEASE "${DEOPTIMIZED_CMAKE_CXX_FLAGS_RELEASE} /O2 /Oy /DNDEBUG")
else()
	string(REGEX REPLACE "-O[^ ]*[ ]*" "" DEOPTIMIZED_CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_CXX_FLAGS_RELEASE "${DEOPTIMIZED_CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

message(CMAKE_CXX_FLAGS_RELEASE="${CMAKE_CXX_FLAGS_RELEASE}")
message(CMAKE_CXX_FLAGS_DEBUG="${CMAKE_CXX_FLAGS_DEBUG}")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET similarity PROPERTY CXX_STANDARD 20)
endif()

