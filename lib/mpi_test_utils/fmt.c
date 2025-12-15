#include "mpi_test_utils/constants.h"
#include "mpi_test_utils/fmt.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* format_with_si_64(int64_t value, int precision)
{
    if (value >=0)
    {
        return format_with_si_u64((uint64_t)value, precision);
    }
    char* pos_str = format_with_si_u64((uint64_t)(-value), precision);
    if (pos_str == NULL)
    {
        return NULL;
    }
    // Allocate string for negative value
    size_t len = strlen(pos_str);
    char* result = (char*)malloc((len + 2) * sizeof(char)); // +1 for '-' and +1 for '\0'
    if (result == NULL)
    {
        free(pos_str);
        return NULL;
    }
    snprintf(result, len + 2, "-%s", pos_str);
    free(pos_str);
    return result;
}
char* format_with_si_u64(uint64_t value, int precision)
{
    // Divide by 1024 to get the appropriate SI unit
    const char* units[] = {"", "K", "M", "G", "T", "P", "E"};
    int unit_index = 0;
    double scaled_value = (double)value;
    while (scaled_value >= 1024.0 && unit_index < 6)
    {
        scaled_value /= 1024.0;
        unit_index++;
    }
    // Allocate string for formatted output
    char* result = (char*)malloc(32 * sizeof(char));
    if (result == NULL)
    {
        return NULL;
    }
    snprintf(result, 32, "%.*f %sB", precision, scaled_value, units[unit_index]);
    return result;
}

char* format_with_comma_64(int64_t value)
{
    if (value >=0)
    {
        return format_with_comma_u64((uint64_t)value);
    }
    char* pos_str = format_with_comma_u64((uint64_t)(-value));
    if (pos_str == NULL)
    {
        return NULL;
    }
    // Allocate string for negative value
    size_t len = strlen(pos_str);
    char* result = (char*)malloc((len + 2) * sizeof(char)); // +1 for '-' and +1 for '\0'
    if (result == NULL)
    {
        free(pos_str);
        return NULL;
    }
    snprintf(result, len + 2, "-%s", pos_str);
    free(pos_str);
    return result;
}
char* format_with_comma_u64( uint64_t value)
{
    // Allocate string for formatted output
    char buffer[32];
    snprintf( buffer, 32, "%" PRIu64, value);
    size_t len = strlen(buffer);
    // Calculate length of result string with commas
    size_t comma_count = (len - 1) / 3;
    size_t result_len = len + comma_count + 1; // +1 for '\0'
    char* result = (char*)malloc(result_len * sizeof(char));
    if (result == NULL)
    {
        return NULL;
    }
    // Insert commas
    size_t j = result_len - 1;
    result[j--] = '\0';
    int digit_count = 0;
    for (int i = len - 1; i >= 0; i--)
    {
        result[j--] = buffer[i];
        digit_count++;
        if (digit_count == 3 && i != 0)
        {
            result[j--] = ',';
            digit_count = 0;
        }
    }
    return result;
}

