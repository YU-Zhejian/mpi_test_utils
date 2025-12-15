cmake_minimum_required(VERSION 3.17)

project(mpi_test_utils LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(MPI REQUIRED COMPONENTS C)

set(LINK_LIBS MPI::MPI_C)

include_directories("${CMAKE_CURRENT_LIST_DIR}/lib")

file(GLOB LIB_SOURCES "lib/mpi_test_utils/*.c")
add_library(mpi_test_utils SHARED ${LIB_SOURCES})
target_link_libraries(mpi_test_utils PUBLIC ${LINK_LIBS})

add_executable(io_speed_nompi exe/io_speed_nompi.c)
target_link_libraries( io_speed_nompi PRIVATE mpi_test_utils )

add_executable(clock_difference exe/clock_difference.c)
target_link_libraries( clock_difference PRIVATE mpi_test_utils )
