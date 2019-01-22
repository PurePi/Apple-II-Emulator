cmake_minimum_required(VERSION 3.12)
project(Apple_II_Emulator C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(dependencies/GLFW)
find_package(OpenGL REQUIRED)
include_directories(include)
link_directories(dependencies)

add_executable(Apple_II_Emulator src/main.c src/memory.c include/memory.h src/cpu.c include/cpu.h src/HIF.c include/HIF.h src/cassette.c src/cassette.h src/peripheral.c src/peripheral.h)

target_link_libraries(Apple_II_Emulator glfw ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} jsmn)

add_custom_command(TARGET Apple_II_Emulator
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Apple_II_Emulator> ../$<TARGET_FILE_NAME:Apple_II_Emulator>)