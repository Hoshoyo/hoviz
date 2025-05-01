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

const char vshader2[] = 
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec2 aOffset; // Per-instance offset\n"

"void main()\n"
"{\n"
    "gl_Position = vec4(aPos + aOffset, 0.0, 1.0);\n"
"}\n";

const char fshader2[] = {
"#version 330 core\n"
"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White lines\n"
"}"
};

int main(int argc, char** argv) {
	hoviz_init(0, 20);

	u32 shader = shader_load_from_buffer(vshader2, fshader2, sizeof(vshader2), sizeof(fshader2));

	#if 1
	float vertices[] = {
		-0.5f, 0.0f,  // Point A
		 0.5f, 0.0f   // Point B
	};
	#else
	float vertices[] = {
		0.0f,  0.5f,  // Top
	   -0.5f, -0.5f,  // Bottom left
		0.5f, -0.5f   // Bottom right
   };
   #endif
	
	unsigned int indices[] = {
		0, 1  // Line between Point A and B
		//0, 1, 2
	};
	
	#if 0
	// Per-instance offsets
	vec2 offsets[] = {
		{ -0.5f,  0.1f },
		{  0.0f,  0.2f },
		{  0.5f,  0.3f },
		{ -0.5f, -0.4f },
		{  0.0f, -0.5f },
		{  0.5f, -0.6f }
	};
	#endif

	const int INSTANCE_COUNT = 1024 * 1024 * 2;
	size_t size = INSTANCE_COUNT * sizeof(vec2);
	vec2* offsets = (vec2*)calloc(INSTANCE_COUNT, sizeof(vec2));
	for(int i = 0; i < 1024; ++i)
	{
		offsets[i].x = 0.1f;
		offsets[i].y = -0.5f + sinf(i * 0.01f);
	}
	
	// Setup VAO/VBO/EBO
	GLuint VAO, VBO, EBO, instanceVBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &instanceVBO);
	
	glBindVertexArray(VAO);
	
	// Vertex buffer (positions)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	// Element buffer (indices)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	// Instance buffer (offsets)
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, size, offsets, GL_STATIC_DRAW);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1); // Advance offset per instance
	
	glBindVertexArray(0);
	
	r64 elapsed = 0;
	r64 sum_time = 0;
	int fps = 0;
	int last_fps = 0;
	char buffer[64] = { 0 };

	while(!hoviz_should_close())
	{
		r64 start_time = hoviz_os_time_us();

		int len = sprintf(buffer, "FPS: %d | Elapsed: %f ms", last_fps, elapsed / 1000.0);
		hoviz_render_text((vec2){0,0}, buffer, len, (vec4){1,1,1,1});

		if (sum_time >= 1000000.0)
		{
			last_fps = fps;
			fps = 0;
			sum_time = 0;
		}

		#if 1
		glClearColor(0.2f,0.2f,0.2f,1.0f);
		// Render the 3 axis
		glUseProgram(shader);
		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_LINES, 2, GL_UNSIGNED_INT, 0, INSTANCE_COUNT); // 6 instances
		//glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0, INSTANCE_COUNT); // 6 instances
		
		hoviz_flush();
		#endif
		
		elapsed = hoviz_os_time_us() - start_time;
		sum_time += elapsed;
		fps++;
	}

	return 0;
}