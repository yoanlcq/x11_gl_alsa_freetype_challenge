#include <stdio.h>
#include <stdlib.h>
#include <dsky/res.h>
#include <dsky/hope.h>

FILE* Res_open(const char *name) {
    char *path;
    asprintf(&path, "res/%s", name); // XXX doesn't search far
    FILE *f = fopen(path, "rb");
    free(path);
    return f;
}

void* FILE_load(FILE *f) {
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    void* data = malloc(size);
    hope(data);
    fread(data, 1, size, f);
    fclose(f);
    return data;
}

void* Res_load(const char *name) {
    FILE *f = Res_open(name);
    if(!f)
        return NULL;
    return FILE_load(f);
}
