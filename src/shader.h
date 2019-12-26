#pragma once
#include <ho_gl.h>
#include <gm.h>

u32 shader_new_lines();
u32 shader_load_from_buffer(const s8* vert_shader, const s8* frag_shader, int vert_length, int frag_length);