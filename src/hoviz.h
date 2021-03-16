#pragma once
#include <gm.h>

typedef struct {
    vec2 position;
    int state;
    int mods;
} Mouse_Button;

typedef struct {
    int mods;
    int key_state[1024];    // 1 = down, 0 = up
    int key_event[1024];
    Mouse_Button mouse_buttons[6];
    vec2 mouse_position;
    vec2 last_mouse_position;
    bool last_mouse_pos_valid;
} HoViz_Input_State;

int  hoviz_init(const char* font_filename, int font_size);
void hoviz_flush();
void hoviz_render_vec3(vec3 v, vec4 color);
void hoviz_render_vec3_from_start(vec3 start, vec3 v, vec4 color);
void hoviz_render_line(vec3 start, vec3 end, vec4 color);
void hoviz_render_triangle(vec3 v1, vec3 v2, vec3 v3, vec4 color);
void hoviz_render_point(vec3 p, vec4 color);
int  hoviz_should_close();
void hoviz_camera_reset();
u32  hoviz_texture_from_data(const char* data, int width, int height);
u32  hoviz_texture_from_file(const char* filename, int* out_width, int* out_height, int* channels);

void hoviz_render_2D_quad_textured(vec2 position, r32 width, r32 height, u32 texture_id);
void hoviz_render_2D_quad(vec2 position, r32 width, r32 height, vec4 color);
void hoviz_render_2D_box(vec2 bl, vec2 tr, vec4 color);
void hoviz_render_2D_line(vec2 start, vec2 end, vec4 color);
void hoviz_render_text(vec2 position, const char* text, int length, vec4 color);
void hoviz_window_get_size(int* width, int* height);

void hoviz_set_3D_camera_speed(r32 movespeed, r32 xrot_speed, r32 yrot_speed);

extern HoViz_Input_State hoviz_input_state;

r64 hoviz_os_time_us();

extern vec4 hoviz_color_red;
extern vec4 hoviz_color_green;
extern vec4 hoviz_color_blue;
extern vec4 hoviz_color_white;
extern vec4 hoviz_color_black;
extern vec4 hoviz_color_magenta;
extern vec4 hoviz_color_yellow;
extern vec4 hoviz_color_cyan;
extern vec4 hoviz_color_gray;