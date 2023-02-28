cmake_minimum_required(VERSION 3.21)

project(apac LANGUAGES C VERSION 0.1.2)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

unset(CMAKE_CXX_FLAGS_RELEASE)
unset(CMAKE_CXX_FLAGS_DEBUG)

set(APAC_IS_COMPILING_FOR_HOST ON)

unset(CMAKE_C_FLAGS)

add_executable(apac)
add_subdirectory(modules)

target_compile_options(apac PRIVATE -Wall -Wno-uninitialized -pipe -Wno-ignored-attributes
	$<$<CONFIG:Release>:-Ofast;-fstack-protector-strong;-fvisibility=hidden>
	$<$<CONFIG:Debug>:-Og;>)

target_link_options(apac PRIVATE $<$<CONFIG:Debug>:-rdynamic;>)

if (APAC_IS_COMPILING_FOR_HOST)
	target_compile_options(apac PRIVATE -march=native)
endif()

target_compile_definitions(apac PRIVATE $<$<CONFIG:Debug>:APAC_IS_UNDER_DEBUG=1>)
target_compile_definitions(apac PRIVATE _GNU_SOURCE)


if (CMAKE_C_COMPILER_ID STREQUAL Clang)
	target_compile_options(apac PRIVATE $<$<CONFIG:Debug>:-gdwarf-4;-fsanitize=address>)
    	if (LINUX)
		target_link_options(apac PRIVATE $<$<CONFIG:Debug>:-lasan>)
	endif()

elseif(CMAKE_C_COMPILER_ID STREQUAL GNU)
	target_compile_options(apac PRIVATE $<$<CONFIG:Debug>:-ggdb;-grecord-gcc-switches>)
endif()

add_subdirectory(OpenCL-Headers)
add_subdirectory(source)

find_package(Threads REQUIRED)
target_link_libraries(apac PRIVATE m Threads::Threads OpenCL::Headers)

install(TARGETS apac DESTINATION ${CMAKE_SOURCE_DIR}/runtime)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/default/ DESTINATION ${CMAKE_SOURCE_DIR}/runtime)
if (WIN32)
elseif(UNIX)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/tools/unix/ DESTINATION ${CMAKE_SOURCE_DIR}/runtime) 
endif()

target_include_directories(apac PRIVATE ${CMAKE_SOURCE_DIR}/source ${CMAKE_SOURCE_DIR}/res)

