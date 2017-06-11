#include <dsky/font.h>
#include <dsky/hope.h>

// NOTE: I don't like this, it should be per-Game.
static FT_Library library;
static FT_Face face;

static const char *font_path
    = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

void Font_setup() {
    hope(!FT_Init_FreeType(&library));
    hope(!FT_New_Face(library, font_path, 0, &face));
    // NOTE: there's also FT_New_Memory_Face
    // hope(!FT_Set_Char_Size(face, 0, 16*64, 96, 96));
    hope(!FT_Set_Pixel_Sizes(face, 0, 16));
}
void Font_cleanup() {
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}
