//
// Created by yuzj on 12/16/25.
//

#ifndef MPI_TEST_UTILS_FMT_H
#define MPI_TEST_UTILS_FMT_H

#include <stdint.h>

char* format_with_si_64(int64_t value, int precision);
char* format_with_si_u64( uint64_t value, int precision);

char* format_with_comma_64(int64_t value);
char* format_with_comma_u64( uint64_t value);

#endif //MPI_TEST_UTILS_FMT_H