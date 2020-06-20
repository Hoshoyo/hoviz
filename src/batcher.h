#pragma once
#include <ho_gl.h>
#include <gm.h>

typedef struct {
    bool valid;
    u32 id;
    s32 unit;
} Hobatch_Texture;

typedef struct {
    GLuint qshader;
    GLuint qvao, qvbo, qebo;

    void* qmem, *qmem_ptr;
    u32 uniform_loc_texture_sampler;
    u32 uniform_loc_projection_matrix;

    s32 quad_count;

    r32 window_width, window_height;

    Hobatch_Texture textures[128];
    s32 tex_unit_next;

    s32 flush_count;
    s32 max_texture_units;

    // Lines
    GLuint lines_vao, lines_vbo;
    GLuint lshader;
    u32 uloc_lines_model;
    u32 uloc_lines_view;
    u32 uloc_lines_projection;

    void* lines_ptr;
    int lines_count;
} Hobatch_Context;

u32 batch_texture_create_from_data(const char* image_data, s32 width, s32 height);

void batch_init(Hobatch_Context* ctx);
void batch_flush(Hobatch_Context* ctx);
void batch_render_quad(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, u32 texture_id, vec4 clipping, r32 blend_factor[4], vec4 color[4], vec2 texcoords[4], r32 red_alpha_override);
void batch_render_quad_textured_clipped(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, u32 texture_id, vec4 clipping);
void batch_render_quad_textured(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, u32 texture_id);
void batch_render_quad_color(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, vec4 color[4]);
void batch_render_quad_color_solid(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, vec4 color);
void batch_render_quad_color_clipped(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, vec4 color[4], vec4 clipping);
void batch_render_quad_color_solid_clipped(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, vec4 color, vec4 clipping);

vec4 batch_render_color_from_hex(uint32_t hex);

void batch_render_line(Hobatch_Context* ctx, vec3 start, vec3 end, vec4 color);