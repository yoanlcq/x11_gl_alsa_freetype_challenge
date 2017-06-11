#pragma once

#include <dsky/log.h>

// The hope macro is a kind of assert for things which can legitimately
// fail at anytime but I'm too lazy to handle at the moment.
// NOTE: __builtin_trap is GCC-specific
#define hope(pred) \
do { \
    if(pred) \
        break; \
    loge_("%s:%u:%s: Hoped that `%s` would be true.\n", __FILE__, __LINE__, __func__, #pred); \
    __builtin_trap(); \
} while(0);
