#pragma once

#include <stdio.h>
#define logi(...) printf("[I]["TAG"] " __VA_ARGS__)
#define logw(...) printf("[W]["TAG"] " __VA_ARGS__)
#define loge(...) printf("[E]["TAG"] " __VA_ARGS__)
#define loge_(...) printf("[E]" __VA_ARGS__)
#define logd(...) printf("[D]["TAG"] " __VA_ARGS__)
#define logv(...) printf("[V]["TAG"] " __VA_ARGS__)
