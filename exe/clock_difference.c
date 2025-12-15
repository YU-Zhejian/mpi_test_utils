

// POSIX source
#define _POSIX_C_SOURCE 200809L // NOLINT

#include "mpi_test_utils/fmt.h"
#include "mpi_test_utils/log.h"

#include <mpi.h>

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>


int main() {
    MPI_Init( NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size == 1)
    {
        if (rank == 0) {
            log_error("%s", "Only one process detected. Clock difference test requires at least two processes.");
        }
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    if (rank == 0)
    {
        log_info( "%s", "Starting clock difference measurement across MPI processes...");
    }
    // Synchronize all processes
    MPI_Barrier(MPI_COMM_WORLD);

    // Get timestamp on each node
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    int64_t clock_ns = ts.tv_sec * 1000000000 + ts.tv_nsec;


    if (rank == 0)
    {
        log_info( "%s", "Collecting clock difference measurement across MPI processes...");
    }

    // Collect all timestamps
    int64_t *all_times_ns = calloc( size, sizeof(int64_t));
    MPI_Allgather(&clock_ns, 1, MPI_INT64_T, all_times_ns, 1, MPI_INT64_T, MPI_COMM_WORLD);
    if (rank != 0 )
    {
        free(all_times_ns);
        MPI_Finalize();
        return EXIT_SUCCESS;
    }
    if (rank == 0)
    {
        log_info( "%s", "Analyzing clock difference measurement across MPI processes...");
    }
    // Calculate all pair-wise differences
    int64_t* all_time_diff = calloc(size * size, sizeof(int64_t));
    int64_t maxdiff = INT64_MIN;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            int this_idx = i * size + j;
            all_time_diff[this_idx]= all_times_ns[i] - all_times_ns[j];
            if (all_time_diff[this_idx] > maxdiff)
            {
                maxdiff = all_time_diff[this_idx];
            }
        }
    }
    // Write results into a CSV
    FILE* csv_file = fopen("clock_differences.csv", "w");
    if (csv_file == NULL)
    {
        perror("Failed to open CSV file for writing");
        free(all_time_diff);
        free(all_times_ns);
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    // Write header
    fprintf(csv_file, "Process");
    for (int j = 0; j < size; j++)
    {
        fprintf(csv_file, ",P%d", j);
    }
    fprintf(csv_file, "\n");
    // Write data
    for (int i = 0; i < size; i++)
    {
        fprintf(csv_file, "P%d", i);
        for (int j = 0; j < size; j++)
        {
            int this_idx = i * size + j;
            fprintf(csv_file, ",%ld", all_time_diff[this_idx]);
        }
        fprintf(csv_file, "\n");
    }
    fclose(csv_file);
    printf("Clock difference matrix written to clock_differences.csv\n");
    char* maxdiff_str = format_with_comma_u64(maxdiff);
    printf("Maximum clock difference observed: %s ns\n", maxdiff_str);
    free(maxdiff_str);
    free(all_time_diff);
    free(all_times_ns);
    log_info("%s", "Done!");
    MPI_Finalize();
    return EXIT_SUCCESS;
}
