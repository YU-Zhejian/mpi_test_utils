
// POSIX code
#define _POSIX_C_SOURCE 200809L // NOLINT

#include "mpi_test_utils/log.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    log_LogFn fn;
    void* udata;
    int level;
} Callback;

static struct {
    void* udata;
    log_LockFn lock;
    int level;
    bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} L;

static const char* level_strings[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

#ifdef LOG_USE_COLOR
static const char* level_colors[] = { "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m" };
#endif

static void stdout_callback(log_event_t* ev)
{
    char buf[16] = { 0 };
    buf[strftime(buf, 16, "%H:%M:%S", &ev->time)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", buf, level_colors[ev->level],
        level_strings[ev->level], ev->file, ev->line);
#else
    fprintf(ev->udata, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file, ev->line);
#endif
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void file_callback(log_event_t* ev)
{
    char buf[64] = { 0 };
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ev->time)] = '\0';
    fprintf(ev->udata, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file, ev->line);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void lock(void)
{
    if (L.lock) {
        L.lock(true, L.udata);
    }
}

static void unlock(void)
{
    if (L.lock) {
        L.lock(false, L.udata);
    }
}

const char* log_level_string(int level) { return level_strings[level]; }

void log_set_lock(log_LockFn fn, void* udata)
{
    L.lock = fn;
    L.udata = udata;
}

void log_set_level(int level) { L.level = level; }

void log_set_quiet(bool enable) { L.quiet = enable; }

int log_add_callback(log_LogFn fn, void* udata, int level)
{
    int i;
    for (i = 0; i < MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
            L.callbacks[i].fn = fn;
            L.callbacks[i].udata = udata;
            L.callbacks[i].level = level;
            // Original code:
            // = (Callback) { fn, udata, level };
            // Microsoft Visual Studio 2010 raised C2059 here.
            return 0;
        }
    }
    return -1;
}

int log_add_fp(FILE* fp, int level) { return log_add_callback(file_callback, fp, level); }

static void init_event(log_event_t* ev, void* udata)
{
    time_t t = time(NULL);
#ifdef _MSC_BUILD
    localtime_s(&ev->time, &t);
#else
    localtime_r(&t, &ev->time); // FIXME: Non-portable.
#endif
    ev->udata = udata;
}

void log_log(int level, const char* file, int line, const char* fmt, ...)
{
    int i;
    log_event_t* ev = calloc(1, sizeof(log_event_t));
    ev->file = file;
    ev->line = line;
    ev->fmt = fmt;

    lock();

    if (!L.quiet && level >= L.level) {
        init_event(ev, stderr);
        va_start(ev->ap, fmt);
        stdout_callback(ev);
        va_end(ev->ap);
    }

    for (i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback* cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(ev, cb->udata);
            va_start(ev->ap, fmt);
            cb->fn(ev);
            va_end(ev->ap);
        }
    }
    free(ev);

    unlock();
}
