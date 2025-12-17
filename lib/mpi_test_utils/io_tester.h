//
// Created by yuzj on 12/16/25.
//
#ifndef MPI_TEST_UTILS_IO_TESTER_H
#define MPI_TEST_UTILS_IO_TESTER_H 1

// Enable POSIX extensions
#define _POSIX_C_SOURCE 200809L // NOLINT

#include <stddef.h>
#include <stdio.h>

ssize_t test_sequential_write_nompi(const char* file_name, size_t block_size, size_t n_blocks);
ssize_t test_sequential_read_nompi(const char* file_name, size_t block_size, size_t n_blocks);
ssize_t test_random_read_nompi(const char* file_name, size_t block_size, size_t n_blocks, size_t n_reads);
ssize_t test_sequential_write_libaio(const char* file_name, size_t block_size, size_t n_blocks);
#endif // MPI_TEST_UTILS_IO_TESTER_H
