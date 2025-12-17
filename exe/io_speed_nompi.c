//
// Created by yuzj on 12/16/25.
//

// Enable POSIX extensions
#define _POSIX_C_SOURCE 200809L // NOLINT

#include "mpi_test_utils/constants.h"
#include "mpi_test_utils/io_tester.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(void)
{
    size_t total_bytes = G_SIZE * 4; // 4 GB
    ssize_t sw_time = test_sequential_write_nompi("test", BLOCK_SIZE, total_bytes / BLOCK_SIZE);
    if (sw_time < 0) {
        fprintf(stderr, "Sequential write test failed\n");
        return EXIT_FAILURE;
    }
    double sw_bandwidth = (double)total_bytes / (double)sw_time * 1e3; // bytes per ns convert to MB/s
    printf("Sequential Write Bandwidth: %.6f MB/s\n", sw_bandwidth);

    ssize_t sr_time = test_sequential_read_nompi("test", BLOCK_SIZE, total_bytes / BLOCK_SIZE);
    if (sr_time < 0) {
        fprintf(stderr, "Sequential read test failed\n");
        return EXIT_FAILURE;
    }
    double sr_bandwidth = (double)total_bytes / (double)sr_time * 1e3; // bytes per ns convert to MB/s
    printf("Sequential Read Bandwidth: %.6f MB/s\n", sr_bandwidth);

    ssize_t rr_time = test_random_read_nompi("test", BLOCK_SIZE, total_bytes / BLOCK_SIZE, total_bytes / BLOCK_SIZE);
    if (rr_time < 0) {
        fprintf(stderr, "Random read test failed\n");
        return EXIT_FAILURE;
    }
    double rr_bandwidth = (double)total_bytes / (double)rr_time * 1e3; // bytes per ns convert to MB/s
    printf("Random Read Bandwidth: %.6f MB/s\n", rr_bandwidth);

    unlink("test");
    return EXIT_SUCCESS;
}
