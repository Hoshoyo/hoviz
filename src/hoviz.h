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
} HoViz_Input_State;

int  hoviz_init_3D();
void hoviz_flush();
void hoviz_render_vec3(vec3 v, vec4 color);
void hoviz_render_vec3_from_start(vec3 start, vec3 v, vec4 color);
void hoviz_render_line(vec3 start, vec3 end, vec4 color);
void hoviz_render_triangle(vec3 v1, vec3 v2, vec3 v3, vec4 color);
void hoviz_render_point(vec3 p, vec4 color);
int  hoviz_should_close();
void hoviz_camera_reset();
extern HoViz_Input_State hoviz_input_state;

r64 os_time_us();