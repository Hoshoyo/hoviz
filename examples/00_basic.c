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

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define FONT_SIZE 26

#define TEXTURE_WIDTH 150
#define TEXTURE_HEIGHT 150

int main(int argc, char** argv) 
{
	hoviz_init("../res/fonts/LiberationMono-Regular.ttf", FONT_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Create a texture from memory data
	u32 data[TEXTURE_HEIGHT][TEXTURE_WIDTH];
	for(s32 y = 0; y < TEXTURE_HEIGHT; ++y)
	{
		for(s32 x = 0; x < TEXTURE_WIDTH; ++x)
		{
			// Format ABGR
			if(y % 32 == 0)
				data[y][x] = 0xff0000ff;
			else if (x % 32 == 0)
				data[y][x] = 0xff00ff00;
			else
				data[y][x] = 0xffff0000;
		}
	}
	u32 texture_id = hoviz_texture_from_data(data, TEXTURE_WIDTH, TEXTURE_HEIGHT);

	while(!hoviz_should_close())
	{
		hoviz_render_vec3((vec3){10,0,0}, hoviz_color_red);
		hoviz_render_vec3((vec3){0,10,0}, hoviz_color_green);
		hoviz_render_vec3((vec3){0,0,10}, hoviz_color_blue);
		
		const vec2 quad_position = (vec2){200, 500};
		hoviz_render_2D_quad_textured(quad_position, TEXTURE_WIDTH, TEXTURE_HEIGHT, texture_id);

		hoviz_flush();
	}

	return 0;
}