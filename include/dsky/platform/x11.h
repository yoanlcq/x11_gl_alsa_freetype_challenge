#pragma once

#include <X11/Xlib.h>
#include <dsky/gl.h>

typedef struct {
    Display *x_display;
    Window x_window;
    int x_screen;
    GLXContext glx_context;
    GLXWindow glx_window;
    bool is_vsync;
} PlatformGame;

typedef XEvent PlatformGameEvent;

