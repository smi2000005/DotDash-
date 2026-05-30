#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

int now_timestamp(char *buf, size_t len);
int safe_str_copy(char *dst, size_t dstSize, const char *src);

#endif // UTILS_H
