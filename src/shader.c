#include "shader.h"

u32 shader_load_from_buffer(const s8* vert_shader, const s8* frag_shader, int vert_length, int frag_length)
{
	GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	GLint compile_status;

	const GLchar* p_v[1] = { vert_shader };
	glShaderSource(vs_id, 1, p_v, &vert_length);

	const GLchar* p_f[1] = { frag_shader };
	glShaderSource(fs_id, 1, p_f, &frag_length);

	glCompileShader(vs_id);
	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(vs_id, sizeof(error_buffer), NULL, error_buffer);
		printf("HELLO WORLD\n");
		printf("shader_load: Error compiling vertex shader: %s\n", error_buffer);
		return -1;
	}

	glCompileShader(fs_id);
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(fs_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		printf("shader_load: Error compiling fragment shader: %s\n", error_buffer);
		return -1;
	}

	GLuint shader_id = glCreateProgram();
	glAttachShader(shader_id, vs_id);
	glAttachShader(shader_id, fs_id);
	glDeleteShader(vs_id);
	glDeleteShader(fs_id);
	glLinkProgram(shader_id);

	glGetProgramiv(shader_id, GL_LINK_STATUS, &compile_status);
	if (compile_status == 0) {
		GLchar error_buffer[512] = { 0 };
		glGetProgramInfoLog(shader_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		printf("shader_load: Error linking program: %s", error_buffer);
		return -1;
	}

	glValidateProgram(shader_id);
	return shader_id;
}

u32 shader_new_lines()
{
    char vshader[] = "#version 330 core\n"
    "layout(location = 0) in vec3 v_vertex;\n"
    "layout(location = 1) in vec4 v_color;\n"
    "out vec4 o_color;\n"
    
    "uniform mat4 model_matrix = mat4(1.0);\n"
    "uniform mat4 view_matrix = mat4(1.0);\n"
    "uniform mat4 projection_matrix = mat4(1.0);\n"

    "void main() {\n"
    "   gl_Position = projection_matrix * view_matrix * model_matrix * vec4(v_vertex, 1.0);\n"
	"	o_color = v_color;\n"
    "}";

    char fshader[] = "#version 330 core\n"
    "in vec4 o_color;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "   color = o_color;\n"
	//"   color = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}";

    return shader_load_from_buffer(vshader, fshader, sizeof(vshader), sizeof(fshader));
}