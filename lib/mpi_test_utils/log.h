/*!
 * @file log.h
 * @brief Log.c is a simple logging library implemented in C99.

 * @section Usage
 * `log.c` and`log.h` should be dropped into an existing project and compiled
 along with it. The library provides 6 function-like macros for logging:
 *
 * ```c
 * log_trace(const char *fmt, ...);
 * log_debug(const char *fmt, ...);
 * log_info(const char *fmt, ...);
 * log_warn(const char *fmt, ...);
 * log_error(const char *fmt, ...);
 * log_fatal(const char *fmt, ...);
 * ```
 *
 * Each function takes a printf format string followed by additional arguments:
 *
 * ```c
 * log_trace("Hello %s", "world")
 * ```
 *
 * Resulting in a line with the given format printed to stderr:
 *
 * ```
 * 20:18:26 TRACE src/main.c:11: Hello world
 * ```
 *
 * @copyright Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

/*!
 * @def LOG_USE_COLOR
 * @brief If the library is compiled with `-DLOG_USE_COLOR` ANSI color escape
 * codes will be used when printing.
 */
#undef LOG_USE_COLOR

#define LOG_VERSION "0.1.0"

#define MAX_CALLBACKS 32

typedef struct {
    /*!
     * @brief `printf`-compatible variadic arguments.
     */
    va_list ap;
    /*!
     * @brief `printf`-compatible format string.
     */
    const char* fmt;
    /*!
     * @brief File name where the logging event is triggered.
     */
    const char* file;
    /*!
     * @brief Time when the logging event is triggered.
     */
    struct tm time;
    void* udata;
    /*!
     * @brief Line number where the logging event is triggered.
     */
    int line;
    /*!
     * @brief Log level. See #LOG_LEVEL for more details.
     */
    int level;
} log_event_t;

typedef void (*log_LogFn)(log_event_t* ev);
typedef void (*log_LockFn)(bool lock, void* udata);

/*!
 * @brief Log levels.
 */
enum LOG_LEVEL {
    /*!
     * @brief Trace level.
     */
    LOG_TRACE,
    /*!
     * @brief Debug level.
     */
    LOG_DEBUG,
    /*!
     * @brief Info level.
     */
    LOG_INFO,
    /*!
     * @brief Warn level.
     */
    LOG_WARN,
    /*!
     * @brief Error level.
     */
    LOG_ERROR,
    /*!
     * @brief Fatal level.
     */
    LOG_FATAL
};

/*!
 * @brief A `printf`-compatible logging function. See Usage for more details.
 */
#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
/*!
 * @brief A `printf`-compatible logging function. See Usage for more details.
 */
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
/*!
 * @brief A `printf`-compatible logging function. See Usage for more details.
 */
#define log_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
/*!
 * @brief A `printf`-compatible logging function. See Usage for more details.
 */
#define log_warn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
/*!
 * @brief A `printf`-compatible logging function. See Usage for more details.
 */
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
/*!
 * @brief A `printf`-compatible logging function. See Usage for more details.
 */
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

/*!
 * @brief Returns the name of the given log level as a string.
 */
const char* log_level_string(int level);

/*!
 * @brief If the log will be written to from multiple threads a lock function
 * can be set.
 *
 * The function is passed the boolean #true if the lock should be acquired or
 * #false if the lock should be released and the given `udata` value.
 */
void log_set_lock(log_LockFn fn, void* udata);
/*!
 * @brief Set the current logging level.
 *
 * All logs below the given level will not be written to `stderr`. By default
 * the level is #LOG_TRACE, such that nothing is ignored.
 * @param level
 */
void log_set_level(int level);
/*!
 * @brief Enable quiet-mode.
 *
 * While this mode is enabled the library will not output anything to `stderr`,
 * but will continue to write to files and callbacks if any are set.
 * @param enable
 */
void log_set_quiet(bool enable);
/*!
* @brief Add one or more callback functions which are called with the log data.

* A callback function is passed a `log_event_t` structure containing the `line`
number, `filename`, `fmt` string, `va` printf va\_list, `level` and the given
`udata`.
*/
int log_add_callback(log_LogFn fn, void* udata, int level);

/*!
 * @brief Add one or more file pointers where the log will be written.
 *
 * The data written to the file output is of the following format:
 *
 * ```
 * 2047-03-11 20:18:26 TRACE src/main.c:11: Hello world
 * ```
 *
 * Any messages below the given `level` are ignored. If the library failed to
 * add a file pointer a value less-than-zero is returned.
 */
int log_add_fp(FILE* fp, int level);

void log_log(int level, const char* file, int line, const char* fmt, ...);

#endif
