#pragma once

#include <stdbool.h>
#include <time.h>
#include <dsky/vec.h>
#include <dsky/gl.h>
#include <dsky/wav.h>

#ifdef __linux__
#include <dsky/platform/x11.h>
#else
#error Unsupported OS
#endif

typedef struct {
    const char *window_title;
    bool center_window; // if true, the field below is ignored
    Vec2i window_position;
    Extent2u window_size;
} GameInitialParams;

typedef struct {
    Vec2f position, velocity;
    bool is_zooming, is_dezooming;
    float zoom;
} View;

typedef struct {
    PlatformGame;
    bool should_quit;
    struct timespec start_time;
    Vec2i current_mouse_position;
    Vec2i current_window_position;
    Vec2f current_bg_position;
    Extent2u current_window_size;
    View view;
    GLuint gl_bg_texture;
    GLuint gl_program;
    GLint gl_loc_texture;
    GLint gl_loc_position;
    GLint gl_loc_scale;
    GLint gl_loc_aspect_ratio;
    GLint gl_loc_view_position;
    GLint gl_loc_view_zoom;
    GLint gl_loc_view_aspect_ratio;
    GLuint gl_vbo;
    PcmWav *bg_wav;
} Game;

typedef PlatformGameEvent GameEvent;

void Game_frame_update(Game *g);
void Game_render_clear(Game *g);
void Game_render_scene(Game *g);
void Game_render_present(Game *g);

Game Game_init(GameInitialParams p);
void Game_deinit(Game g);
bool Game_poll_event(Game *g, GameEvent *e);
GameEvent Game_wait_event(Game *g);
void Game_handle_event(Game *g, GameEvent event);
