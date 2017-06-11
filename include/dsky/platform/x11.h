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
    XKeyEvent x11_previous_key_release_event;

    Atom WM_DELETE_WINDOW;
    Atom _NET_WM_PID;
    Atom _NET_WM_PING;
    Atom _NET_WORKAREA;
    Atom _NET_DESKTOP_VIEWPORT;
    Atom _NET_DESKTOP_GEOMETRY;
    Atom _NET_WM_ICON;
    Atom _NET_FRAME_EXTENTS;
    Atom _NET_WM_STATE;
    Atom _NET_WM_STATE_FULLSCREEN;

} PlatformGame;

typedef XEvent PlatformGameEvent;

