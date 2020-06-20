#include <gm.h>
#include "font_load.h"
#include "batcher.h"

typedef struct {
  int index;
  vec2 position;
  int width;
  int height;
} Text_Render_Character_Position;

typedef struct {
  int line_count;
  int max_column_count;
  r32 width;
  r32 height;
} Text_Render_Info;

Text_Render_Info text_prerender(Font_Info* font_info, const char* text, int length, int start_index, 
  Text_Render_Character_Position* out_positions, int positions_count);
int text_render(Hobatch_Context* ctx, Font_Info* font_info, const char* text, int length, int start_index, vec2 position, vec4 clipping, vec4 color);