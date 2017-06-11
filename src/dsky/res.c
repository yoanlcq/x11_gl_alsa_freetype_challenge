#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dsky/res.h>
#include <dsky/hope.h>
#include <unistd.h>

#define TAG "Res"

static char exe_dir[4096] = "";

FILE* Res_open(const char *name) {
    // PERF: Should be done once instead at some point
    if(!exe_dir[0]) {
        ssize_t bytes_read = readlink("/proc/self/exe", exe_dir, sizeof exe_dir);
        hope(bytes_read > 0);
        exe_dir[bytes_read] = '\0';
        *strrchr(exe_dir, '/') = '\0';
        logi("Exe dir is `%s`\n", exe_dir);
    }

    const char *const mid[6] = {
        "res",
        "../res",
        "../../res",
        "",
        "..",
        "../..",
    };
    for(int i=0 ; i<6 ; ++i) {
        char *path;
        asprintf(&path, "%s/%s/%s", exe_dir, mid[i], name);
        FILE *f = fopen(path, "rb");
        free(path);
        if(f)
            return f;
    }
    return NULL;
}

Iov FILE_load(FILE *f) {
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    void* data = malloc(size);
    hope(data);
    size = fread(data, 1, size, f); // Don't panic if there's less data
    fclose(f);
    return (Iov){ .data=data, .size=size };
}

Iov Res_load(const char *name) {
    FILE *f = Res_open(name);
    if(!f)
        return (Iov){0};
    return FILE_load(f);
}
