#include "batcher.h"
#include <stdlib.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"

// https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)

#define BATCH_SIZE (1024 * 64)
#define ARRAY_LENGTH(A) (sizeof(A) / sizeof(*(A)))

typedef struct {
    vec3 position;           // 12
    vec2 text_coord;         // 20
    vec4 color;              // 36
    vec4 clipping;           // 52
    r32  blend_factor;       // 56
    r32  texture_index;      // 60
    r32  red_alpha_override; // 64
} Hobatch_Vertex;

const char vshader[] = 
"#version 330 core\n"
"layout(location = 0) in vec3  v_position;\n"
"layout(location = 1) in vec2  v_text_coords;\n"
"layout(location = 2) in vec4  v_color;\n"
"layout(location = 3) in vec4  v_clipping;\n"
"layout(location = 4) in float v_blend_factor;\n"
"layout(location = 5) in float v_texture_index;\n"
"layout(location = 6) in float v_red_alpha_override;\n"

"out vec2 o_text_coords;\n"
"out vec4 o_color;\n"
"out vec4 clipping;\n"
"out vec2 position;\n"
"out float o_texture_index;\n"
"out float o_blend_factor;\n"
"out float red_alpha_override;\n"

"uniform mat4 u_projection = mat4(1.0);\n"

"void main()\n"
"{\n"
"    gl_Position = u_projection * vec4(v_position.xy, 0.0, 1.0);\n"
"    position = v_position.xy;\n"
"    o_text_coords  = v_text_coords;\n"
"    o_color = v_color;\n"
"    o_texture_index = v_texture_index;\n"
"    clipping = v_clipping;\n"
"    o_blend_factor = v_blend_factor;\n"
"    red_alpha_override = v_red_alpha_override;\n"
"}\n";

const char fshader[] = 
"#version 330 core\n"
"in vec2 o_text_coords;\n"
"in vec4 o_color;\n"
"in float o_texture_index;\n"
"in float o_blend_factor;\n"
"in float red_alpha_override;\n"

"in vec4 clipping;\n"
"in vec2 position;\n"

"out vec4 color;\n"

"uniform sampler2D u_textures[gl_MaxTextureImageUnits];\n"

"void main()\n"
"{\n"

"    if(position.x < clipping.x || position.x > clipping.x + clipping.z) {\n"
"        discard;\n"
"    }\n"
"    if(position.y < clipping.y || position.y > clipping.y + clipping.w) {\n"
"        discard;\n"
"    }\n"
"    int index = int(o_texture_index);\n"
"    vec4 texture_color = texture(u_textures[index], o_text_coords);\n"
"    color = mix(texture_color, o_color, o_blend_factor);\n"
"    color.a = mix(color.a, texture_color.r * o_color.a, red_alpha_override);\n"

//"color = vec4(1.0,1.0,1.0,0.6);\n"

"}\n";

typedef struct {
    vec3 position;
    vec4 color;
} Vertex_3D;

static int
init_lines(Hobatch_Context* ctx, int batch_size) 
{
    ctx->lshader = shader_new_lines();
    glGenVertexArrays(1, &ctx->lines_vao);
	glBindVertexArray(ctx->lines_vao);

    glGenBuffers(1, &ctx->lines_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->lines_vbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(Vertex_3D) * batch_size, 0, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, color));

    ctx->uloc_lines_model = glGetUniformLocation(ctx->lshader, "model_matrix");
    ctx->uloc_lines_view = glGetUniformLocation(ctx->lshader, "view_matrix");
    ctx->uloc_lines_projection = glGetUniformLocation(ctx->lshader, "projection_matrix");

    return 0;
}

static void
render_lines(Hobatch_Context* ctx)
{
    glUseProgram(ctx->lshader);

    glBindVertexArray(ctx->lines_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->lines_vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    mat4 projection = gm_mat4_ortho(0, (r32) ctx->window_width, 0, (r32) ctx->window_height);
    glUniformMatrix4fv(ctx->uniform_loc_projection_matrix, 1, GL_TRUE, (GLfloat *) projection.data);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, 2 * ctx->lines_count);

    ctx->lines_ptr = 0;
    ctx->lines_count = 0;
}

u32
batch_texture_create_from_data(const char* image_data, s32 width, s32 height)
{
    u32 texture_id = 0;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return texture_id;
}

void
batch_init(Hobatch_Context* ctx)
{
    init_lines(ctx, 1024);
    ctx->qshader = shader_load_from_buffer(vshader, fshader, sizeof(vshader)-1, sizeof(fshader)-1);

    ctx->uniform_loc_texture_sampler = glGetUniformLocation(ctx->qshader, "u_textures");
    ctx->uniform_loc_projection_matrix = glGetUniformLocation(ctx->qshader, "u_projection");

    glUseProgram(ctx->qshader);

    // Query the max number of texture units
    int texture_units = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    int texture_samplers[128] = {0};
    for(s32 i = 0; i < 128; ++i)
        texture_samplers[i] = i;
    glUniform1iv(ctx->uniform_loc_texture_sampler, texture_units, texture_samplers);
    ctx->max_texture_units = texture_units;

    glGenVertexArrays(1, &ctx->qvao);
    glBindVertexArray(ctx->qvao);

    glGenBuffers(1, &ctx->qvbo);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->qvbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Hobatch_Vertex) * BATCH_SIZE * 4, 0, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Hobatch_Vertex), &((Hobatch_Vertex *) 0)->position);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Hobatch_Vertex), &((Hobatch_Vertex *) 0)->text_coord);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Hobatch_Vertex), &((Hobatch_Vertex *) 0)->color);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Hobatch_Vertex), &((Hobatch_Vertex *) 0)->clipping);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Hobatch_Vertex), &((Hobatch_Vertex *) 0)->blend_factor);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Hobatch_Vertex), &((Hobatch_Vertex *) 0)->texture_index);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Hobatch_Vertex), &((Hobatch_Vertex *) 0)->red_alpha_override);

    u32* indices = (u32 *) calloc(1, BATCH_SIZE * 6 * sizeof(u32));
    for (u32 i = 0, j = 0; i < BATCH_SIZE * 6; i += 6, j += 4)
    {
        indices[i + 0] = j;
        indices[i + 1] = j + 1;
        indices[i + 2] = j + 2;
        indices[i + 3] = j + 2;
        indices[i + 4] = j + 1;
        indices[i + 5] = j + 3;
    }
    glGenBuffers(1, &ctx->qebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->qebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(u32) * BATCH_SIZE, indices, GL_STATIC_DRAW);
    free(indices);
}

void
batch_flush(Hobatch_Context* ctx)
{
    ctx->flush_count++;
    glUseProgram(ctx->qshader);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(ctx->qvao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->qebo);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->qvbo);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    for(int i = 0; i < sizeof(ctx->textures)/sizeof(*ctx->textures); ++i)
    {
        if(ctx->textures[i].valid)
        {
            glBindTextureUnit(ctx->textures[i].unit, ctx->textures[i].id);
        }
    }

    mat4 projection = gm_mat4_ortho(0, (r32) ctx->window_width, 0, (r32) ctx->window_height);
    glUniformMatrix4fv(ctx->uniform_loc_projection_matrix, 1, GL_TRUE, (GLfloat *) projection.data);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);

    glDrawElements(GL_TRIANGLES, 6 * ctx->quad_count, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // reset
    ctx->qmem = 0;
    ctx->qmem_ptr = 0;
    ctx->quad_count = 0;
    ctx->tex_unit_next = 0;
    memset(ctx->textures, 0, sizeof(ctx->textures));

    render_lines(ctx);
}

// bl, br, tl, tr
void
batch_render_quad(Hobatch_Context* ctx, vec3 position, 
    r32 width, r32 height, u32 texture_id, vec4 clipping,
    r32 blend_factor[4], vec4 color[4], vec2 texcoords[4], r32 red_alpha_override)
{
    if(((char*)ctx->qmem_ptr - (char*)ctx->qmem) >= BATCH_SIZE * sizeof(Hobatch_Vertex))
    {
        batch_flush(ctx);
    }

    s32 texture_unit_index = texture_id % ARRAY_LENGTH(ctx->textures);
    s32 texture_unit = -1;

    // If this is a new texture and we already have the max amount, flush it
    if(ctx->tex_unit_next >= ctx->max_texture_units && 
       !(ctx->textures[texture_unit_index].valid && ctx->textures[texture_unit_index].id != texture_id))
    {
        batch_flush(ctx);
    }
    
    // search for a texture unit slot
    while(ctx->textures[texture_unit_index].valid)
    {
        if(ctx->textures[texture_unit_index].id == texture_id)
        {
            texture_unit = ctx->textures[texture_unit_index].unit;
            break;
        }
        texture_unit_index = (texture_unit_index + 1) % ARRAY_LENGTH(ctx->textures);
    }

    // we did not find a slot with this texture already, so grab the next available
    if(texture_unit == -1)
    {
        texture_unit = ctx->tex_unit_next++;
    }
    ctx->textures[texture_unit_index].id = texture_id;
    ctx->textures[texture_unit_index].unit = texture_unit;
    ctx->textures[texture_unit_index].valid = true;

    if(!ctx->qmem)
    {
        glBindBuffer(GL_ARRAY_BUFFER, ctx->qvbo);
        ctx->qmem = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        ctx->qmem_ptr = ctx->qmem;
    }

    // 0 blend factor means full texture no color
    Hobatch_Vertex v[] = 
    {
        {(vec3){position.x, position.y, position.z},                  texcoords[0], color[0], clipping, blend_factor[0], (r32)texture_unit, red_alpha_override},
        {(vec3){position.x + width, position.y, position.z},          texcoords[1], color[1], clipping, blend_factor[1], (r32)texture_unit, red_alpha_override},
        {(vec3){position.x, position.y + height, position.z},         texcoords[2], color[2], clipping, blend_factor[2], (r32)texture_unit, red_alpha_override},
        {(vec3){position.x + width, position.y + height, position.z}, texcoords[3], color[3], clipping, blend_factor[3], (r32)texture_unit, red_alpha_override},
    };
    
    memcpy(ctx->qmem_ptr, v, sizeof(v));
    ctx->qmem_ptr = ((char*)ctx->qmem_ptr) + sizeof(v);
    ctx->quad_count += 1;
}

void
batch_render_quad_textured_clipped(Hobatch_Context* ctx, vec3 position, 
    r32 width, r32 height, u32 texture_id, vec4 clipping)
{
    r32 blend_factor[4] = {0,0,0,0};
    vec2 texcoords[4] = 
    {
        (vec2){0.0f, 0.0f},
        (vec2){1.0f, 0.0f},
        (vec2){0.0f, 1.0f},
        (vec2){1.0f, 1.0f}
    };
    vec4 color[4] = {0};
    batch_render_quad(ctx, position, width, height, texture_id, clipping, blend_factor, color, texcoords, 0.0f);
}

void
batch_render_quad_textured(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, u32 texture_id)
{
    vec4 clipping = (vec4){0,0,FLT_MAX,FLT_MAX};
    batch_render_quad_textured_clipped(ctx, position, width, height, texture_id, clipping);
}

void
batch_render_quad_color(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, vec4 color[4])
{
    vec4 clipping = (vec4){0,0,FLT_MAX,FLT_MAX};
    vec2 texcoords[4] = 
    {
        (vec2){0.0f, 0.0f},
        (vec2){1.0f, 0.0f},
        (vec2){0.0f, 1.0f},
        (vec2){1.0f, 1.0f}
    };
    r32 blend_factor[4] = {1,1,1,1};
    batch_render_quad(ctx, position, width, height, 0, clipping, blend_factor, color, texcoords, 0.0f);
}

void
batch_render_quad_color_solid(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, vec4 color)
{
    vec4 clipping = (vec4){0,0,FLT_MAX,FLT_MAX};
    r32 blend_factor[4] = {1,1,1,1};
    vec2 texcoords[4] = 
    {
        (vec2){0.0f, 0.0f},
        (vec2){1.0f, 0.0f},
        (vec2){0.0f, 1.0f},
        (vec2){1.0f, 1.0f}
    };
    vec4 c[] = {color, color, color, color};
    batch_render_quad(ctx, position, width, height, 0, clipping, blend_factor, c, texcoords, 0.0f);
}

void
batch_render_quad_color_clipped(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, vec4 color[4], vec4 clipping)
{
    r32 blend_factor[4] = {1,1,1,1};
    vec2 texcoords[4] = 
    {
        (vec2){0.0f, 0.0f},
        (vec2){1.0f, 0.0f},
        (vec2){0.0f, 1.0f},
        (vec2){1.0f, 1.0f}
    };
    batch_render_quad(ctx, position, width, height, 0, clipping, blend_factor, color, texcoords, 0.0f);
}

void
batch_render_quad_color_solid_clipped(Hobatch_Context* ctx, vec3 position, r32 width, r32 height, vec4 color, vec4 clipping)
{
    r32 blend_factor[4] = {1,1,1,1};
    vec4 c[] = {color, color, color, color};
    vec2 texcoords[4] = 
    {
        (vec2){0.0f, 0.0f},
        (vec2){1.0f, 0.0f},
        (vec2){0.0f, 1.0f},
        (vec2){1.0f, 1.0f}
    };
    batch_render_quad(ctx, position, width, height, 0, clipping, blend_factor, c, texcoords, 0.0f);
}


vec4
batch_render_color_from_hex(uint32_t hex)
{
    return (vec4){
        (float)((hex >> 24) & 0xff) / 255.0f,
        (float)((hex >> 16) & 0xff) / 255.0f,
        (float)((hex >> 8) & 0xff) / 255.0f,
        (float)((hex) & 0xff) / 255.0f,
    };
}

/*
    Lines
*/

void
batch_render_line(Hobatch_Context* ctx, vec3 start, vec3 end, vec4 color)
{
    if(ctx->lines_ptr == 0) {
        glBindVertexArray(ctx->lines_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ctx->lines_vbo);
        ctx->lines_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    }

    Vertex_3D* lines = ((Vertex_3D*)ctx->lines_ptr) + (2 * ctx->lines_count);

    lines[0].position = start;
    lines[0].color = color;

    lines[1].position = end;
    lines[1].color = color;

    ctx->lines_count++;
}