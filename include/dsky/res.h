#pragma once

typedef struct {
    void *data;
    size_t size;
} Iov;

FILE* Res_open(const char *name);
Iov Res_load(const char *name);
Iov FILE_load(FILE *f);
