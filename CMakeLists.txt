# Set minimum cmake version
cmake_minimum_required(VERSION 3.15)
# Set a variable that contains the project name
set(PROJECT_NAME prime_numbers)
# Set project name and language definition
project(${PROJECT_NAME} VERSION 1.0 LANGUAGES CXX)
# Define the executable and the source files required to build it
add_executable(${PROJECT_NAME} src/main.cpp)
# Set C++ language standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
# Link with Windows system libraries (needed for WinAPI threading)
target_link_libraries(${PROJECT_NAME} PRIVATE kernel32)
