//
// Created by yuzj on 12/16/25.
//

// Enable POSIX extensions
#define _POSIX_C_SOURCE 200809L // NOLINT

#include "mpi_test_utils/constants.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

ssize_t test_sequential_write_nompi(char* file_name, size_t block_size, size_t n_blocks)
{
    // Open file for writing
    FILE* file = fopen(file_name, "wb");
    if (file == NULL)
    {
        perror("Failed to open file for writing");
        return -1;
    }
    // Allocate buffer
    char* buffer = (char*)calloc(block_size, sizeof(char));
    if (buffer == NULL)
    {
        perror("Failed to allocate buffer");
        fclose(file);
        return -1;
    }
    // Write
    // Get start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n_blocks; i++)
    {
        size_t written = fwrite(buffer, sizeof(char), block_size, file);
        if (written != block_size)
        {
            perror("Failed to write data");
            free(buffer);
            fclose(file);
            return -1;
        }
    }
    // Get end time
    clock_gettime(CLOCK_MONOTONIC, &end);
    // Calculate elapsed time in nanosecs
    ssize_t elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
    // Clean up
    free(buffer);
    fclose(file);
    return elapsed_ns;
}

int main(void)
{
    size_t total_bytes = G_SIZE * 4; // 4 GB
    ssize_t sw_time = test_sequential_write_nompi("test", BLOCK_SIZE, total_bytes / BLOCK_SIZE);
    if (sw_time < 0)
    {
        fprintf(stderr, "Sequential write test failed\n");
        return EXIT_FAILURE;
    }
    double sw_bandwidth = (double)total_bytes / (double)sw_time; // bytes per ns
    sw_bandwidth *= 1e3; // convert to MB/s
    printf("Sequential Write Bandwidth: %.6f MB/s\n", sw_bandwidth);
    return EXIT_SUCCESS;
}
