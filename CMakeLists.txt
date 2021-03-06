cmake_minimum_required(VERSION 2.8)
project(par-wb-bench)

find_package(OpenMP)
find_package(Qt4 REQUIRED QtCore QtGui)
find_package(Boost COMPONENTS graph REQUIRED)

include(${QT_USE_FILE})

find_package(Git REQUIRED)

if (EXISTS ${CMAKE_SOURCE_DIR}/parpenet/src)
    message(STATUS "parpenet submodule already exists")
else (EXISTS ${CMAKE_SOURCE_DIR}/parpenet/src)
    message(STATUS "preparing parpenet submodule")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule init
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout -t origin/bench-both 
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/parpenet/)
endif (EXISTS ${CMAKE_SOURCE_DIR}/parpenet/src)

set(BENCH_SRCS main.cpp graph.cpp graph.h vp-tree.h)

link_directories(${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${OpenMP_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")

add_executable(bench ${BENCH_SRCS})
target_link_libraries(bench ${QT_LIBRARIES} -lgfortran -lblas -llapack)

add_executable(dump_matrizes dump_matrices.cpp graph.cpp vp-tree.h)
target_link_libraries(dump_matrizes ${QT_LIBRARIES})

add_subdirectory(parpenet/src)
