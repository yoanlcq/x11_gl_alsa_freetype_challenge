#include <dsky/game.h>
#include <dsky/log.h>
#include <dsky/vec.h>
#include <dsky/hope.h>
#include <dsky/res.h>
#include <dsky/gl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define TAG "Game"

static const char *vsrc = 
    "#version 130\n"
    "#pragma debug(on)\n"
    "\n"
    "uniform vec2 u_position;\n"
    "uniform vec2 u_scale;\n"
    "\n"
    "in vec2 a_position;\n"
    "in vec2 a_texcoords;\n"
    "\n"
    "out vec2 v_texcoords;\n"
    "\n"
    "void main() {\n"
    "    v_texcoords = a_texcoords;\n"
    "    gl_Position = vec4(a_position*u_scale + u_position, 0, 1);\n"
    "}\n"
;

static const char *fsrc = 
    "#version 130\n"
    "#pragma debug(on)\n"
    "\n"
    "in vec2 v_texcoords;\n"
    "\n"
    "out vec4 f_color;\n"
    "\n"
    "uniform sampler2D u_texture;\n"
    "\n"
    "void main() {\n"
    "    f_color = texture(u_texture, v_texcoords);\n"
    "}\n"
;

void Game_reshape(Game *g) {
    glViewport(0, 0, g->current_window_size.w, g->current_window_size.h);
}

void Game_frame_update(Game *g) {

    Vec2f factor = { .x=-0.02f, .y=-0.02f };
    Vec2f world = {
        .x = ((g->current_mouse_position.x / (float) g->current_window_size.w) - 0.5f) * 2,
        .y = (0.5f - (g->current_mouse_position.y / (float) g->current_window_size.h)) * 2
    };
    // printf("mosx: %i\n", g->current_mouse_position.x);
    // printf("sizw: %u\n", g->current_window_size.w);
    g->current_bg_velocity.x = world.x*factor.x;
    g->current_bg_velocity.y = world.y*factor.y;
    const Vec2f max_vel = { .x=0.01f, .y=0.01f };
    if(fabsf(g->current_bg_velocity.x) <= max_vel.x)
        g->current_bg_velocity.x = 0;
    if(fabsf(g->current_bg_velocity.y) <= max_vel.y)
        g->current_bg_velocity.y = 0;
    //printf("velx: %f\n", g->current_bg_velocity.x);

    g->current_bg_position.x += g->current_bg_velocity.x;
    g->current_bg_position.y += g->current_bg_velocity.y;
    g->current_bg_position.x = clampf(g->current_bg_position.x, -1.25f, 1.25f);
    g->current_bg_position.y = clampf(g->current_bg_position.y, -1.0f, 1.0f);
    //printf("posx: %f\n", g->current_bg_position.x);
}
void Game_render_clear(Game *g) {
    (void)g;
    glClearColor(1,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Game_render_scene(Game *g) {
    glUseProgram(g->gl_program);
    glBindBuffer(GL_ARRAY_BUFFER, g->gl_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(4*sizeof(Vec2f)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    const uint32_t texunit = 0;
    glActiveTexture(GL_TEXTURE0 + texunit);
    glBindTexture(GL_TEXTURE_2D, g->gl_bg_texture);
    glUniform1i(g->gl_loc_texture, texunit);
    glUniform2f(g->gl_loc_scale, 2.5f, 2.f);
    glUniform2fv(g->gl_loc_position, 1, (const GLfloat*)&g->current_bg_position);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
void Game_render_present(Game *g) {
    if(!g->is_vsync) {
        struct timespec t = {0}, rem;
        t.tv_nsec = 16666666;
        nanosleep(&t, &rem);
    }
    glXSwapBuffers(g->x_display, g->glx_window);
}

static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
static bool has_GLX_ARB_create_context = false;
static bool has_GLX_ARB_create_context_profile = false;
static bool has_GLX_EXT_swap_control = false;
static bool has_GLX_EXT_swap_control_tear = false;
static const char *glx_extensions;
static Atom WM_DELETE_WINDOW = None;

Game Game_init(GameInitialParams p) {

    Game g = {0};
    g.should_quit = false;
    g.x_display = XOpenDisplay(NULL);
    hope(g.x_display);
    g.x_screen = DefaultScreen(g.x_display);

    int glx_error_base = 0, glx_event_base = 0;
    hope(glXQueryExtension(g.x_display, &glx_error_base, &glx_event_base));

    int glx_major = 0, glx_minor = 0;
    hope(glXQueryVersion(g.x_display, &glx_major, &glx_minor));
    hope(glx_major >= 1);
    hope(glx_major == 1 ? glx_minor >= 4 : true);

    // NOTE: I don't set an X11 custom error handler when creating the
    // OpenGL context, and I'm fine with that right now.

    // PERF: strstr() every time
    glx_extensions = glXQueryExtensionsString(g.x_display, g.x_screen);
    has_GLX_ARB_create_context = !!strstr(glx_extensions, "GLX_ARB_create_context");
    has_GLX_ARB_create_context_profile = !!strstr(glx_extensions, "GLX_ARB_create_context_profile");
    has_GLX_EXT_swap_control = !!strstr(glx_extensions, "GLX_EXT_swap_control");
    has_GLX_EXT_swap_control_tear = !!strstr(glx_extensions, "GLX_EXT_swap_control_tear");

    hope(has_GLX_ARB_create_context);
    hope(has_GLX_ARB_create_context_profile); // NOTE: because of laziness

    glXCreateContextAttribsARB = 
        (PFNGLXCREATECONTEXTATTRIBSARBPROC) 
        glXGetProcAddressARB((const GLubyte*) "glXCreateContextAttribsARB");
    hope(glXCreateContextAttribsARB);

    if(has_GLX_EXT_swap_control) {
        glXSwapIntervalEXT =
            (PFNGLXSWAPINTERVALEXTPROC)
            glXGetProcAddressARB((const GLubyte*) "glXSwapIntervalEXT");
        hope(glXSwapIntervalEXT);
    }

    int fbattribs[] = {
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_SAMPLE_BUFFERS, 1,
        GLX_SAMPLES, 4,
        None
    };
    GLXFBConfig fbconfig;
    {
        int fbconfigs_count = 0;
        GLXFBConfig *fbconfigs = glXChooseFBConfig(
            g.x_display, g.x_screen, fbattribs, &fbconfigs_count
        );
        hope(fbconfigs);
        hope(fbconfigs_count > 0);
        fbconfig = fbconfigs[0];
        XFree(fbconfigs);
    }

    XVisualInfo *visual_info = glXGetVisualFromFBConfig(g.x_display, fbconfig);
    hope(visual_info);

    XSetWindowAttributes swa = {0};
    Colormap cmap;
    swa.colormap = cmap = XCreateColormap(g.x_display, RootWindow(g.x_display, visual_info->screen), visual_info->visual, AllocNone);
    swa.background_pixmap = None;
    swa.border_pixel = 0;
    swa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
                     ButtonReleaseMask | EnterWindowMask |
                     LeaveWindowMask | PointerMotionMask | 
                     Button1MotionMask |
                     Button2MotionMask | Button3MotionMask |
                     Button4MotionMask | Button5MotionMask |
                     ButtonMotionMask | KeymapStateMask |
                     ExposureMask | VisibilityChangeMask | 
                     StructureNotifyMask | /* ResizeRedirectMask | */
                     SubstructureNotifyMask | SubstructureRedirectMask |
                     FocusChangeMask | PropertyChangeMask |
                     ColormapChangeMask | OwnerGrabButtonMask;

    // https://specifications.freedesktop.org/wm-spec/latest/
    // Atom _NET_WM_PID   = XInternAtom(g.x_display, "_NET_WM_PID", False);
    // Atom _NET_WM_PING  = XInternAtom(g.x_display, "_NET_WM_PING", False);
    // Atom _NET_WORKAREA = XInternAtom(g.x_display, "_NET_WORKAREA", False);
    // Atom _NET_DESKTOP_VIEWPORT = XInternAtom(g.x_display, "_NET_DESKTOP_VIEWPORT", False);
    // Atom _NET_DESKTOP_GEOMETRY = XInternAtom(g.x_display, "_NET_DESKTOP_GEOMETRY", False);
    // Atom _NET_WM_ICON  = XInternAtom(g.x_display, "_NET_WM_ICON", False);
    // Atom _NET_FRAME_EXTENTS  = XInternAtom(g.x_display, "_NET_FRAME_EXTENTS", False);

    if(p.center_window) {
        
        /*
        Atom req_type = 0, actual_type_return = 0;
        int actual_format_return;
        unsigned long nitems_return, bytes_after_return;
        */

        /*
        struct {
            int32_t x, y, w, h;
        } *workarea;
        hope(XGetWindowProperty(
            g.x_display, RootWindow(g.x_display, g.x_screen),
            _NET_WORKAREA, 0, 4, False,
            req_type, &actual_type_return, &actual_format_return, 
            &nitems_return, &bytes_after_return, 
            (unsigned char**)&workarea
        ) == Success);
        logi("(%i,%i,%i,%i)\n", workarea->x, workarea->y, workarea->w, workarea->h);
        */
        /*
        Vec2i *dpos;
        Extent2u *dsiz;
        hope(XGetWindowProperty(
            g.x_display, RootWindow(g.x_display, g.x_screen),
            _NET_DESKTOP_VIEWPORT, 0, 2, False,
            req_type, &actual_type_return, &actual_format_return, 
            &nitems_return, &bytes_after_return, 
            (unsigned char**)&dpos
        ) == Success);
        hope(XGetWindowProperty(
            g.x_display, RootWindow(g.x_display, g.x_screen),
            _NET_DESKTOP_GEOMETRY, 0, 2, False,
            req_type, &actual_type_return, &actual_format_return, 
            &nitems_return, &bytes_after_return, 
            (unsigned char**)&dsiz
        ) == Success);
        logi("(%i,%i,%i,%i)\n", dpos->x, dpos->y, dsiz->w, dsiz->h);
        */

        // NOTE: The taskbar is not part of the computation
        Extent2u d = {
            .w = DisplayWidth(g.x_display, g.x_screen),
            .h = DisplayHeight(g.x_display, g.x_screen)
            // .w = workarea->w,
            // .h = workarea->h
            // .w = dsiz->w,
            // .h = dsiz->h
        };
        p.window_position = (Vec2i) {
            .x = d.w/2 - p.window_size.w/2,
            .y = d.h/2 - p.window_size.h/2,
        };

        // XFree(dpos);
        // XFree(dsiz);
        // XFree(workarea);
    }

    g.x_window = XCreateWindow(
        g.x_display, RootWindow(g.x_display, visual_info->screen),
        p.window_position.x, p.window_position.y,
        p.window_size.w, p.window_size.h,
        0, visual_info->depth, 
        InputOutput, visual_info->visual,
        CWBorderPixel | CWColormap | CWEventMask, &swa
    );

    XFree(visual_info);

    XSizeHints hints = {0};
    hints.flags  = PPosition | PSize;
    hints.x      = p.window_position.x;
    hints.y      = p.window_position.y;
    hints.width  = p.window_size.w;
    hints.height = p.window_size.h;
    XSetNormalHints(g.x_display, g.x_window, &hints);

    XStoreName(g.x_display, g.x_window, p.window_title);
    
    WM_DELETE_WINDOW = XInternAtom(g.x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(g.x_display, g.x_window, &WM_DELETE_WINDOW, 1);

    XColor cursor_bg = {0}, cursor_fg = {0}, dummy;
    // NOTE: view /etc/X11/rgb.txt
    XLookupColor(g.x_display, cmap, "gold", &dummy, &cursor_bg);
    XLookupColor(g.x_display, cmap, "brown", &dummy, &cursor_fg);
    Vec2i cursor_hotspot = {0};
    Pixmap cursor_pixmap, cursor_mask;
#include <X11/bitmaps/left_ptr>
    cursor_pixmap = XCreateBitmapFromData(
        g.x_display, RootWindow(g.x_display, g.x_screen), 
        (void*)left_ptr_bits, left_ptr_width, left_ptr_height
    );
#include <X11/bitmaps/left_ptrmsk>
    cursor_mask = XCreateBitmapFromData(
        g.x_display, RootWindow(g.x_display, g.x_screen), 
        (void*)left_ptrmsk_bits, left_ptrmsk_width, left_ptrmsk_height
    );
    Cursor cursor = XCreatePixmapCursor(g.x_display, cursor_pixmap, cursor_mask, &cursor_fg, &cursor_bg, cursor_hotspot.x, cursor_hotspot.y);
    XDefineCursor(g.x_display, g.x_window, cursor);

    XFreePixmap(g.x_display, cursor_mask);
    XFreePixmap(g.x_display, cursor_pixmap);

    int attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        None
    };
    g.glx_context =
        glXCreateContextAttribsARB(g.x_display, fbconfig, NULL, True, attribs);
    hope(g.glx_context);
    logi("GLX context is direct : %i\n", 
        glXIsDirect(g.x_display, g.glx_context)
    );
    g.glx_window = glXCreateWindow(g.x_display, fbconfig, g.x_window, NULL);
    hope(g.glx_window);
    hope(glXMakeContextCurrent(g.x_display, g.glx_window, g.glx_window, g.glx_context));

    g.is_vsync = true;
    if(has_GLX_EXT_swap_control_tear)
        glXSwapIntervalEXT(g.x_display, g.glx_window, -1);
    else if(has_GLX_EXT_swap_control)
        glXSwapIntervalEXT(g.x_display, g.glx_window, 1);
    else
        g.is_vsync = false;

    XMapRaised(g.x_display, g.x_window);
    XSync(g.x_display, False);

    logi("# OpenGL settings\n");
    logi("Vendor: %s\n", glGetString(GL_VENDOR));
    logi("Renderer: %s\n", glGetString(GL_RENDERER));
    logi("Version: %s\n", glGetString(GL_VERSION));
    logi("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    Iov rgba_iov = Res_load("bg_1024x1024_rgba.data");
    hope(rgba_iov.size);
    Rgba32 *data = rgba_iov.data;
    Extent2u extent = { .w=1024, .h=1024 };
    g.gl_bg_texture = GLTexture_from_power_of_two_rgba32(data, extent);
    free(data);
    hope(g.gl_bg_texture);

    g.gl_program = GLProgram_quick_load(vsrc, fsrc);
    hope(g.gl_program);

    g.gl_loc_texture = glGetUniformLocation(g.gl_program, "u_texture");
    g.gl_loc_position = glGetUniformLocation(g.gl_program, "u_position");
    g.gl_loc_scale = glGetUniformLocation(g.gl_program, "u_scale");
    hope(g.gl_loc_texture != -1);
    hope(g.gl_loc_position != -1);
    hope(g.gl_loc_scale != -1);

    glBindAttribLocation(g.gl_program, 0, "a_position");
    glBindAttribLocation(g.gl_program, 1, "a_texcoords");


    const Vec2f vpositions[] = {
        { .x=-1, .y=-1 },
        { .x=-1, .y=+1 },
        { .x=+1, .y=-1 },
        { .x=+1, .y=+1 }
    };

    const Extent2(float) texcoord_bounds = {
        .w=1020.f/1024.f,
        .h=407.f/1024.f,
    };
    const TexUv vtexcoords[] = {
        { .u=0                , .v=texcoord_bounds.h },
        { .u=0                , .v=0 },
        { .u=texcoord_bounds.w, .v=texcoord_bounds.h },
        { .u=texcoord_bounds.w, .v=0 }
    };

    g.gl_vbo = 0;
    glGenBuffers(1, &g.gl_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g.gl_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof vpositions + sizeof vtexcoords, NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vpositions, vpositions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof vpositions, sizeof vtexcoords, vtexcoords);

    Iov wavfiledata = Res_load("bg.wav");
    hope(wavfiledata.size);
    g.bg_wav = wavfiledata.data;
    PcmWav_convert_endianness(g.bg_wav);
    hope(PcmWav_is_valid(g.bg_wav));
    PcmWav_log(g.bg_wav, "Loaded `bg.wav`:");
    PcmWav_play_once(g.bg_wav);

    return g;
}


void Game_deinit(Game g) {
    free(g.bg_wav);
    glDeleteBuffers(1, &g.gl_vbo);
    glDeleteProgram(g.gl_program);
    glXMakeContextCurrent(g.x_display, None, None, NULL);
    glXDestroyWindow(g.x_display, g.glx_window);
    glXDestroyContext(g.x_display, g.glx_context);
    XDestroyWindow(g.x_display, g.x_window);
    XCloseDisplay(g.x_display);
}

bool Game_poll_event(Game *g, GameEvent *e) {
    assert(e);
    int event_count = XPending(g->x_display);
    if(event_count > 0)
        XNextEvent(g->x_display, e);
    return !!event_count;
}
GameEvent Game_wait_event(Game *g) {
    XEvent event = {0};
    XNextEvent(g->x_display, &event);
    return event;
}

static void Game_handle_KeyPress(Game *g, XKeyEvent *e) {
    (void)g;
    KeySym keysym = XLookupKeysym(e, 0);
    hope(keysym != NoSymbol);
    //logi("Pressed: keycode=%i, keysym=%lu\n", e->keycode, keysym);
}
static void Game_handle_KeyRelease(Game *g, XKeyEvent *e) {
    (void)g;
    KeySym keysym = XLookupKeysym(e, 0);
    hope(keysym != NoSymbol);
    //logi("Released: keycode=%i, keysym=%lu\n", e->keycode, keysym);
}
static void Game_handle_ButtonPress(Game *g, XButtonEvent *e) {
    (void)g;
    (void)e;
    /*
    const char *name = "";
    switch(e->button) {
    case 1: name = "Left mouse button"; break;
    case 2: name = "Middle mouse button"; break;
    case 3: name = "Right mouse button"; break;
    }
    logi("Pressed: %s\n", name);
    */
}
static void Game_handle_ButtonRelease(Game *g, XButtonEvent *e) {
    (void)g;
    (void)e;
    /*
    const char *name = "";
    switch(e->button) {
    case 1: name = "Left mouse button"; break;
    case 2: name = "Middle mouse button"; break;
    case 3: name = "Right mouse button"; break;
    }
    logi("Released: %s\n", name);
    */
}
static void Game_handle_MotionNotify(Game *g, XMotionEvent *e) {
    g->current_mouse_position.x = e->x;
    g->current_mouse_position.y = e->y;
    /*
    logi("Mouse at (%d, %d) (absolute: (%d, %d))\n", 
        e->x, e->y, e->x_root, e->y_root
    );
    */
}
static void Game_handle_FocusIn(Game *g, XFocusChangeEvent *e) {
    (void)g;
    (void)e;
    //logi("We gained focus\n");
}
static void Game_handle_FocusOut(Game *g, XFocusChangeEvent *e) {
    (void)g;
    (void)e;
    //logi("We lost focus\n");
}
static void Game_handle_EnterNotify(Game *g, XCrossingEvent *e) {
    g->current_mouse_position.x = e->x;
    g->current_mouse_position.y = e->y;
    //logi("Mouse is in\n");
}
static void Game_handle_LeaveNotify(Game *g, XCrossingEvent *e) {
    (void)g;
    (void)e;
    //logi("Mouse is out\n");
}
static void Game_handle_ConfigureNotify(Game *g, XConfigureEvent *e) {
    (void)g;
    g->current_window_position.x = e->x;
    g->current_window_position.y = e->y;
    g->current_window_size.w = e->width;
    g->current_window_size.h = e->height;
    Game_reshape(g);
    /*
    logi("Window moved to (%i, %i) and resized to (%i, %i)\n", 
        e->x, e->y, e->width, e->height
    );
    */
}

void Game_handle_event(Game *g, GameEvent event) {
    switch(event.type) {
    case KeyPress:
        Game_handle_KeyPress(g, (void*)&event);
        break;
    case KeyRelease:
        Game_handle_KeyRelease(g, (void*)&event);
        break;
    case ButtonPress:
        Game_handle_ButtonPress(g, (void*)&event);
        break;
    case ButtonRelease:
        Game_handle_ButtonRelease(g, (void*)&event);
        break;
    case MotionNotify:
        Game_handle_MotionNotify(g, (void*)&event);
        break;
    case EnterNotify:
        Game_handle_EnterNotify(g, (void*)&event);
        break;
    case LeaveNotify:
        Game_handle_LeaveNotify(g, (void*)&event);
        break;
    case FocusIn:
        Game_handle_FocusIn(g, (void*)&event);
        break;
    case FocusOut:
        Game_handle_FocusOut(g, (void*)&event);
        break;
    case ClientMessage:
        if((Atom) event.xclient.data.l[0] == WM_DELETE_WINDOW) {
            logi("Received quit event\n");
            g->should_quit = true;
        }
        break;
    case ConfigureNotify:
        Game_handle_ConfigureNotify(g, (void*)&event);
        break;
    default:
        //logi("Unknown event %#x\n", event.type);
        return;
    }
}
