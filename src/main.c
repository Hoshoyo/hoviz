#include <stdio.h>
#include <float.h>
#define GRAPHICS_MATH_IMPLEMENT
#define HOGL_IMPLEMENT
#define STB_IMAGE_IMPLEMENTATION
#include <ho_gl.h>
#include <gm.h>
#include <stb_image.h>
#include "hoviz.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	hoviz_init(0, 20);

	//int width, height, channels;
	//u32 texture = hoviz_texture_from_file("test.png", &width, &height, &channels);

	while(!hoviz_should_close())
	{
		// Render the 3 axis
		hoviz_render_vec3((vec3){10, 0, 0}, (vec4){1.0f, 0.0f, 0.0f, 0.3f});
		hoviz_render_vec3((vec3){0, 10, 0}, (vec4){0.0f, 1.0f, 0.0f, 0.3f});
		hoviz_render_vec3((vec3){0, 0, 10}, (vec4){0.0f, 0.0f, 1.0f, 0.3f});

		hoviz_render_2D_line((vec2){0,0}, (vec2){100,100}, (vec4){1,1,1,1});
		hoviz_render_2D_box((vec2){0,0}, (vec2){100,100}, (vec4){1,1,1,1});
		hoviz_render_2D_box((vec2){1,1}, (vec2){99,99}, (vec4){0,0,0,1});

		hoviz_flush();
	}

	return 0;
}