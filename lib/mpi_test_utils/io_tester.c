//
// Created by yuzj on 12/16/25.
//
// Enable POSIX extensions
#define _POSIX_C_SOURCE 200809L // NOLINT

#include "mpi_test_utils/io_tester.h"
#include "mpi_test_utils/pcg_basic.h"

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

ssize_t test_sequential_write_nompi(const char* file_name, size_t block_size, size_t n_blocks)
{
    // Open file for writing
    FILE* file = fopen(file_name, "wbe");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return -1;
    }
    // Allocate buffer
    char* buffer = (char*)calloc(block_size, sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        fclose(file);
        return -1;
    }
    // Write
    // Get start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n_blocks; i++) {
        size_t written = fwrite(buffer, sizeof(char), block_size, file);
        if (written != block_size) {
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
ssize_t test_sequential_read_nompi(const char* file_name, size_t block_size, size_t n_blocks)
{
    // Open file for reading
    FILE* file = fopen(file_name, "rbe");
    if (file == NULL) {
        perror("Failed to open file for reading");
        return -1;
    }
    // Allocate buffer
    char* buffer = (char*)calloc(block_size, sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        fclose(file);
        return -1;
    }
    // Read
    // Get start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n_blocks; i++) {
        size_t read = fread(buffer, sizeof(char), block_size, file);
        if (read != block_size) {
            perror("Failed to read data");
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
ssize_t test_random_read_nompi(const char* file_name, size_t block_size, size_t n_blocks, size_t n_reads)
{
    pcg32_random_t* rng = malloc(sizeof(pcg32_random_t));
    pcg32_srandom_r(rng, (uint64_t)time(NULL), (uint64_t)(uintptr_t)rng);
    // Open file for reading
    FILE* file = fopen(file_name, "rbe");
    if (file == NULL) {
        perror("Failed to open file for reading");
        free(rng);
        return -1;
    }
    // Allocate buffer
    char* buffer = (char*)calloc(block_size, sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        fclose(file);
        free(rng);
        return -1;
    }
    // Read
    // Get start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n_reads; i++) {
        size_t rand_block_idx = pcg32_boundedrand_r(rng, (uint32_t)n_blocks);
        size_t offset = rand_block_idx * block_size;
        if (rand_block_idx != (n_blocks - 1)) {
            offset += pcg32_boundedrand_r(rng, block_size);
        }
        if (fseek(file, offset, SEEK_SET) != 0) {
            perror("Failed to seek to position");
            free(buffer);
            fclose(file);
            free(rng);
            return -1;
        }
        size_t read = fread(buffer, sizeof(char), block_size, file);
        if (read != block_size) {
            perror("Failed to read data");
            free(buffer);
            fclose(file);
            free(rng);
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
    free(rng);
    return elapsed_ns;
}
ssize_t test_sequential_write_libaio(const char* file_name, size_t block_size, size_t n_blocks)
{
    int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Failed to open file for writing");
        return -1;
    }

    struct aiocb** cbs = calloc(n_blocks, sizeof(struct aiocb*));
    if (cbs == NULL) {
        perror("Failed to allocate aiocb array");
        close(fd);
        return -1;
    }

    for (size_t i = 0; i < n_blocks; i++) {
        cbs[i] = calloc(1, sizeof(struct aiocb));
        if (cbs[i] == NULL) {
            perror("Failed to allocate aiocb");
            for (size_t j = 0; j < i; j++) {
                free((void*)cbs[j]->aio_buf);
                free(cbs[j]);
            }
            free(cbs);
            close(fd);
            return -1;
        }

        char* buffer = calloc(block_size, sizeof(char));
        if (buffer == NULL) {
            perror("Failed to allocate buffer");
            for (size_t j = 0; j < i; j++) {
                free((void*)cbs[j]->aio_buf);
                free(cbs[j]);
            }
            free(cbs[i]);
            free(cbs);
            close(fd);
            return -1;
        }

        /* Initialize aiocb (calloc already zeroes it) */
        cbs[i]->aio_fildes = fd;
        cbs[i]->aio_buf = buffer;
        cbs[i]->aio_nbytes = block_size;
        // cbs[i]->aio_offset = (off_t)(i * block_size); /* explicit non-overlapping offsets */
        // cbs[i]->aio_sigevent.sigev_notify = SIGEV_NONE;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    struct sigevent sig = { 0, 0 };

    // Write
    for (size_t i = 0; i < n_blocks; i++) {
        if (aio_write(cbs[i]) == -1) {
            perror("Failed to submit AIO write");
            for (size_t j = 0; j <= i; j++) {
                free((void*)cbs[j]->aio_buf);
                free(cbs[j]);
            }
            free(cbs);
            close(fd);
            return -1;
        }
    }

    /* Check per-request results */
    for (size_t i = 0; i < n_blocks; i++) {
        int err = EINPROGRESS;
        while (err == EINPROGRESS) {
            err = aio_error(cbs[i]);
        }

        if (err != 0) {
            fprintf(stderr, "AIO write error for request %zu: %s\n", i, strerror(err));
        } else {
            ssize_t ret = aio_return(cbs[i]);
            if (ret != (ssize_t)block_size) {
                fprintf(stderr, "AIO write returned %zd bytes for request %zu (expected %zu)\n", ret, i, block_size);
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    ssize_t elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

    /* Clean up all allocations */
    for (size_t i = 0; i < n_blocks; i++) {
        if (cbs[i]) {
            free((void*)cbs[i]->aio_buf);
            free(cbs[i]);
        }
    }
    free(cbs);
    close(fd);
    return elapsed_ns;
}
