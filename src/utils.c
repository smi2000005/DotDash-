#include "utils.h"
#include "../include/errors.h"

#include <time.h>
#include <stdio.h>
#include <string.h>

int now_timestamp(char *buf, size_t len) {
    if (!buf || len < 20) return ERR_ARG;
    time_t t = time(NULL);
    struct tm tm;
    struct tm *tmPtr = localtime(&t);
    if (!tmPtr) return ERR_IO;
    tm = *tmPtr;
    int r = snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    return (r > 0 && (size_t)r < len) ? OK : ERR_IO;
}

int safe_str_copy(char *dst, size_t dstSize, const char *src) {
    if (!dst || !src || dstSize == 0) return ERR_ARG;
    strncpy(dst, src, dstSize - 1);
    dst[dstSize - 1] = '\0';
    return OK;
}
