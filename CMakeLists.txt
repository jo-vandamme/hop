cmake_minimum_required(VERSION 3.1)
project(hop)

set(BUILD_SHARED_LIBS OFF)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Lua REQUIRED)

# Set compilation flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE CXX STANDARD REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -std=c++1y -O3 -Wall -Wextra -Wpedantic -march=native")

#add_definitions(-DDEBUG)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(OPENGL_INCLUDE_DIR)
include_directories(GLEW_INCLUDE_DIRS)
include_directories(GLFW_INCLUDE_DIRS)
include_directories(LUA_INCLUDE_DIR)

include_directories(src)

file(GLOB_RECURSE HOP_HEADERS src/*.h)
file(GLOB_RECURSE HOP_SOURCES src/*.cpp)

add_executable(hop ${HOP_HEADERS} ${HOP_SOURCES})
target_link_libraries(hop ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} glfw ${GLFW_LIBRARIES} ${LUA_LIBRARIES})

install(TARGETS hop DESTINATION bin)
