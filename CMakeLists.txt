cmake_minimum_required(VERSION 2.8)
project(par-wb-bench)

find_package(OpenMP)
find_package(Qt4 REQUIRED QtCore QtGui)

include(${QT_USE_FILE})

file(GLOB SRCS *.cpp *.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${OpenMP_CXX_FLAGS}")
link_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(bench ${SRCS})
target_link_libraries(bench ${QT_LIBRARIES} -lgfortran -lblas -llapack -lpardiso412-GNU450-X86-64)
