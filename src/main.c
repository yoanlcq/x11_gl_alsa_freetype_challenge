#include <dsky/game.h>

// TODO
// - Actually center window;
// - Actually have Vsync;
// - Prevent resizing;

int main() {
    GameInitialParams p = {
        .window_title = "Dracosky",
        .window_position = { .x = (1366-854)/2, .y = (768-20-480)/2 },
        .window_size = { .w=854, .h=480 }
    };
    Game g = Game_init(p);
    do {
        GameEvent event = {0};
        while(Game_poll_event(&g, &event)) {
            Game_handle_event(&g, event);
        }
        Game_frame_update(&g);
        Game_render_clear(&g);
        Game_render_scene(&g);
        Game_render_present(&g);
    } while(!g.should_quit);

    Game_deinit(g);
    return 0;
}
