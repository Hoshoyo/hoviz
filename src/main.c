#include <stdio.h>
#include <float.h>
#define GRAPHICS_MATH_IMPLEMENT
#define HOGL_IMPLEMENT
#include <ho_gl.h>
#include <gm.h>
#include "hoviz.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	hoviz_init(0, 20);

	unsigned int* data = malloc(100*100*4);
	for(int i = 0; i < 100*100; ++i)
		data[i] = 0x550000ff;
	u32 texture = hoviz_texture_from_data((const char*)data, 100, 100);

	while(!hoviz_should_close())
	{
		// Render the 3 axis
		hoviz_render_vec3((vec3){10, 0, 0}, (vec4){1.0f, 0.0f, 0.0f, 0.3f});
		hoviz_render_vec3((vec3){0, 10, 0}, (vec4){0.0f, 1.0f, 0.0f, 0.3f});
		hoviz_render_vec3((vec3){0, 0, 10}, (vec4){0.0f, 0.0f, 1.0f, 0.3f});

		hoviz_render_text((vec2){0,0}, "Hello", 5, hoviz_color_white);

		hoviz_render_2D_quad_textured((vec2){100,100}, 200, 200, texture);
		//hoviz_render_2D_quad((vec2){0,0}, 200, 200, (vec4){1,1,1,0.5f});

		hoviz_flush();
	}

	return 0;
}