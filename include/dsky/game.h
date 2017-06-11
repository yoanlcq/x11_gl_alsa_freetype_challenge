#pragma once

#include <stdbool.h>
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
    Vec2i window_position;
    Extent2u window_size;
} GameInitialParams;

typedef struct {
    PlatformGame;
    bool should_quit;
    GLuint gl_bg_texture;
    GLuint gl_program;
    GLint gl_loc_texture;
    GLint gl_loc_position;
    GLint gl_loc_scale;
    GLuint gl_vbo;
    PcmWav *bg_wav;
    Vec2f current_bg_position, current_bg_velocity;
    Vec2i current_mouse_position;
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
