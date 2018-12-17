cmake_minimum_required(VERSION 3.12)
project(analyse2)

set(CMAKE_CXX_STANDARD 14)
set(INC_DIR /usr/include/llvm-7)
set(LINK_DIR /usr/lib/llvm-7/lib)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(llvm)

add_executable(analyse2 main.cpp head.h SearchDirtyDest.cpp SearchDirtyDest.h test.h test.cpp utils.h utils.cpp Analyse.h Analyse.cpp Environment.h Environment.cpp)

target_link_libraries(analyse2 llvm)