cmake_minimum_required(VERSION 3.20)
project(Framework_Example)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(DEBUG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --no-gnu-unique -march=native -mtune=native -Wno-return-type -ggdb")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --no-gnu-unique -s -march=native -mtune=native -Wno-return-type -fvisibility=hidden -fvisibility-inlines-hidden")
endif()

set(CMAKE_SKIP_RPATH ON)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/Source/*.cpp")

add_library(Framework_Example SHARED ${SOURCE_FILES})
target_link_libraries(Framework_Example dl SDL2 GL)

if(!DEBUG)
	target_compile_definitions(Framework_Example PUBLIC NDEBUG)
endif()

# Setup Framework
target_include_directories(Framework_Example PUBLIC Dependencies/Framework/Source/Framework)

file(GLOB_RECURSE FRAMEWORK_SOURCE_FILES "${CMAKE_SOURCE_DIR}/Dependencies/Framework/Source/Framework/*.cpp")
target_sources(Framework_Example PUBLIC ${FRAMEWORK_SOURCE_FILES})

target_compile_definitions(Framework_Example PUBLIC FRAMEWORK_ENABLE_HOOKING_DETOUR)
target_compile_definitions(Framework_Example PUBLIC FRAMEWORK_ENABLE_HOOKING_PTRSWAP)
target_compile_definitions(Framework_Example PUBLIC FRAMEWORK_ENABLE_PATTERN_SCANNING)
target_compile_definitions(Framework_Example PUBLIC FRAMEWORK_ENABLE_RETURN_ADDRESS)

# Setup ImGui
target_include_directories(Framework_Example PUBLIC Dependencies/imgui)
# SDL
target_include_directories(Framework_Example PUBLIC /usr/include/SDL2)

file(GLOB IMGUI_SOURCE_FILES "${CMAKE_SOURCE_DIR}/Dependencies/imgui/*.cpp")
list(REMOVE_ITEM IMGUI_SOURCE_FILES "${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_demo.cpp")

list(APPEND IMGUI_SOURCE_FILES "${CMAKE_SOURCE_DIR}/Dependencies/imgui/backends/imgui_impl_opengl3.cpp")
list(APPEND IMGUI_SOURCE_FILES "${CMAKE_SOURCE_DIR}/Dependencies/imgui/backends/imgui_impl_sdl.cpp")
target_sources(Framework_Example PUBLIC ${IMGUI_SOURCE_FILES})

# Setup xorstr
target_include_directories(Framework_Example PUBLIC Dependencies/xorstr/include)
