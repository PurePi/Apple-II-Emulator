cmake_minimum_required(VERSION 3.12)
project(Apple_II_Emulator C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(Dependencies/GLFW)
find_package(OpenGL REQUIRED)
include_directories(include)

add_executable(Apple_II_Emulator src/main.c src/memory.c include/memory.h src/cpu.c include/cpu.h src/HIF.c include/HIF.h)

target_link_libraries(Apple_II_Emulator glfw ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES})