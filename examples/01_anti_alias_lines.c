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

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MIN(A, B) (((A) < (B)) ? (A) : (B))

void image_clear(uint32_t* image, int width, int height, uint32_t color)
{
	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x)
		{
			image[y * width + x] = color;
		}
	}
}

void bresenham_line(uint32_t* image, int width, int height, int x0, int y0, int x1, int y1)
{
	float dx = x1 - x0;
	float dy = y1 - y0;
	int D = 2 * dy - dx;
	int y = y0;

	for(int x = x0; x <= x1; ++x)
	{
		image[y * width + x] = 0xffffffff;
		if(D > 0)
		{
			y = y + 1;
			D = D - 2 * dx;
		}
		D += 2 * dy;
	}
}

uint32_t alpha_blend(uint32_t base, uint32_t blended, r32 alpha)
{
	u8 base_r = (u8)((r32)(base & 0xff) * (1.0f - alpha));
	u8 base_g = (u8)((r32)((base >> 8) & 0xff) * (1.0f - alpha));
	u8 base_b = (u8)((r32)((base >> 16) & 0xff) * (1.0f - alpha));

	u8 blended_r = (u8)((r32)(blended & 0xff) * alpha);
	u8 blended_g = (u8)((r32)((blended >> 8) & 0xff) * alpha);
	u8 blended_b = (u8)((r32)((blended >> 16) & 0xff) * alpha);

	return 0xff000000 | (base_r + blended_r) | 
		((base_g << 8) + (blended_g << 8)) |
		((base_b << 16) + (blended_b << 16));
}

void xiaolin_wu_line(uint32_t* image, int width, int height, float x0, float y0, float x1, float y1)
{
	if(fabs(y1 - y0) < fabs(x1 - x0))
	{
		if(x1 < x0)
		{
			float tmp = x0;
			x0 = x1;
			x1 = tmp;
			
			tmp = y0;
			y0 = y1;
			y1 = tmp;
		}
	
		float dx = x1 - x0;
		float dy = y1 - y0;
		float m = (dx != 0) ? dy / dx : 1.0f;

		float overlap = 1.0f - ((x0 + 0.5f) - (int)(x0 + 0.5f));
		float dist_start = y0 - (int)y0;
		image[(int)y0 * width + (int)(x0 + 0.5f)] = alpha_blend(image[(int)y0 * width + (int)(x0 + 0.5f)], 0xff00ffff, (1.0f - dist_start) * overlap);
		image[((int)y0 + 1) * width + (int)(x0 + 0.5f)] = alpha_blend(image[((int)y0 + 1) * width + (int)(x0 + 0.5f)], 0xff00ffff, dist_start * overlap);

		overlap = ((x1 - 0.5f) - (int)(x1 - 0.5f));
		float dist_end = y1 - (int)y1;
		image[(int)y1 * width + (int)(x1 + 0.5f)] = alpha_blend(image[(int)y1 * width + (int)(x1 + 0.5f)], 0xff00ffff, (1.0f - dist_end) * overlap);
		image[((int)y1 + 1) * width + (int)(x1 + 0.5f)] = alpha_blend(image[((int)y1 + 1) * width + (int)(x1 + 0.5f)], 0xff00ffff, dist_end * overlap);
	
		for(int i = 1; i < ((int)(dx + 0.5f)); ++i)
		{
			float x = x0 + i;
			float y = y0 + i * m;
			int ix = (int)x;
			int iy = (int)y;
	
			float distance = y - (float)iy;
	
			image[iy * width + ix] = alpha_blend(image[iy * width + ix], 0xff00ffff, 1.0f - distance);
			image[(iy + 1) * width + ix] = alpha_blend(image[(iy + 1) * width + ix], 0xff00ffff, distance);
		}
	}
	else
	{
		if(y1 < y0)
		{
			float tmp = x0;
			x0 = x1;
			x1 = tmp;
			
			tmp = y0;
			y0 = y1;
			y1 = tmp;
		}
	
		float dx = x1 - x0;
		float dy = y1 - y0;
		float m = (dy != 0) ? dx / dy : 1.0f;

		float overlap = 1.0f - ((y0 + 0.5f) - (int)(y0 + 0.5f));
		float dist_start = y0 - (int)y0;
		image[(int)y0 * width + (int)(x0 + 0.5f)] = alpha_blend(image[(int)y0 * width + (int)(x0 + 0.5f)], 0xff00ffff, (1.0f - dist_start) * overlap);
		image[((int)y0 + 1) * width + (int)(x0 + 0.5f)] = alpha_blend(image[((int)y0 + 1) * width + (int)(x0 + 0.5f)], 0xff00ffff, dist_start * overlap);

		overlap = ((y1 - 0.5f) - (int)(y1 - 0.5f));
		float dist_end = y1 - (int)y1;
		image[(int)(y1 + 0.5f) * width + (int)x1] = alpha_blend(image[(int)(y1 + 0.5f) * width + (int)x1], 0xff00ffff, (1.0f - dist_end) * overlap);
		image[(int)(y1 + 0.5f) * width + (int)x1 + 1] = alpha_blend(image[(int)(y1 + 0.5f) * width + (int)x1 + 1], 0xff00ffff, dist_end * overlap);
	
		for(int i = 1; i < ((int)(dy + 0.5f)); ++i)
		{
			float x = x0 + i * m;
			float y = y0 + i;
			int ix = (int)x;
			int iy = (int)y;
	
			float distance = x - (float)ix;
	
			image[iy * width + ix] = alpha_blend(image[iy * width + ix], 0xff00ffff, 1.0f - distance);
			image[iy * width + (ix + 1)] = alpha_blend(image[iy * width + (ix + 1)], 0xff00ffff, distance);
		}
	}
}

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 480
#define FONT_SIZE 26

#define SUBIMAGE_WIDTH 256
#define SUBIMAGE_HEIGHT 256
#define SUBIMAGE_X 100
#define SUBIMAGE_Y 100

static uint32_t image[WINDOW_WIDTH][WINDOW_HEIGHT];

int main(int argc, char** argv) 
{
	hoviz_init("../res/fonts/LiberationMono-Regular.ttf", FONT_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT);
	
	image_clear((uint32_t*)image, WINDOW_WIDTH, WINDOW_HEIGHT, 0xff555555);
	u32 texture = hoviz_texture_from_data((const char*)image, WINDOW_WIDTH, WINDOW_HEIGHT);

	u32 subpixels[SUBIMAGE_HEIGHT][SUBIMAGE_WIDTH] = {0};

	while(!hoviz_should_close())
	{
		{
			vec2 cursor = hoviz_input_state.mouse_position;
			int x0 = SUBIMAGE_WIDTH / 2;
			int y0 = SUBIMAGE_HEIGHT / 2;
			int x1 = MAX(0, MIN((s32)cursor.x - SUBIMAGE_X, SUBIMAGE_WIDTH - 2));
			int y1 = MAX(0, MIN(WINDOW_HEIGHT - (s32)cursor.y - SUBIMAGE_Y, SUBIMAGE_HEIGHT - 2));

			image_clear((uint32_t*)subpixels, SUBIMAGE_WIDTH, SUBIMAGE_HEIGHT, 0xff666666);
			xiaolin_wu_line((uint32_t*)subpixels, SUBIMAGE_WIDTH, SUBIMAGE_HEIGHT, x0, y0, x1, y1);
			
			hoviz_texture_update(texture, SUBIMAGE_X, SUBIMAGE_Y, SUBIMAGE_WIDTH, SUBIMAGE_HEIGHT, subpixels);
		}

		hoviz_texture_update(texture, SUBIMAGE_X, SUBIMAGE_Y, SUBIMAGE_WIDTH, SUBIMAGE_HEIGHT, subpixels);
		hoviz_render_2D_quad_textured((vec2){0,0}, WINDOW_WIDTH, WINDOW_HEIGHT, texture);

		hoviz_flush();
	}

	return 0;
}