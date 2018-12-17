cmake_minimum_required(VERSION 3.12)
set(PROJECT_NAME DirtyPath)
project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader)

add_executable(${PROJECT_NAME} main.cpp head.h SearchDirtyDest.cpp SearchDirtyDest.h test.h test.cpp utils.h utils.cpp Analyse.h Analyse.cpp Environment.h Environment.cpp)

target_link_libraries(${PROJECT_NAME} ${llvm_libs})