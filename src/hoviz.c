#include "hoviz.h"
#include <gm.h>
#include <light_array.h>
#include <ho_gl.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "camera.h"
#include "input.h"
#include <time.h>

#if defined(__linux__)
#include <unistd.h>

void os_usleep(u64 microseconds)
{
	usleep(microseconds);
}

r64 os_time_us()
{
	struct timespec t_spec;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec);
	u64 res = t_spec.tv_nsec + 1000000000 * t_spec.tv_sec;
	return (r64)res / 1000.0;
}
#else
static r64 perf_frequency;
static void os_set_query_frequency() {
	LARGE_INTEGER li = { 0 };
	QueryPerformanceFrequency(&li);
	perf_frequency = (r64)(li.QuadPart);
}
r64 os_time_us() {
	static initialized = false;
	if (!initialized) {
		os_set_query_frequency();
		initialized = true;
	}

	LARGE_INTEGER li = { 0 };
	QueryPerformanceCounter(&li);
	return ((r64)(li.QuadPart) / perf_frequency) * 1000000.0;
}
#endif

#define CAMERA_ROTATE_VELOCITY_X 0.1f
#define CAMERA_ROTATE_VELOCITY_Y 0.2f
#define CAMERA_VELOCITY 0.5f

static void camera_update(Camera* camera, r64 delta_time);

#define offset_of(S, F) &((S*)0)->F

HoViz_Input_State hoviz_input_state;

typedef struct {
    GLFWwindow* window;

    u32 shader;

    u32 lines_vao;
    u32 lines_vbo;
    void* lines_ptr;
    int   lines_count;

    u32 triangles_vao;
    u32 triangles_vbo;
    void* triangles_ptr;
    int triangles_count;

    u32 points_vao;
    u32 points_vbo;
    void* points_ptr;
    int points_count;

    u32 uloc_model;
    u32 uloc_view;
    u32 uloc_projection;
    Camera camera;

    int fps;
    r64 elapsed;
    r64 last_frame_start;
} HoViz_Context;

typedef struct {
    vec3 position;
    vec4 color;
} HoViz_Vertex_3D;

static HoViz_Context ctx;

void window_get_size(int* width, int* height)
{
	glfwGetFramebufferSize(ctx.window, width, height);
}

static int 
init_points(int batch_size, u32 shader) {
    glGenVertexArrays(1, &ctx.points_vao);
	glBindVertexArray(ctx.points_vao);

    glGenBuffers(1, &ctx.points_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(HoViz_Vertex_3D) * batch_size, 0, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(HoViz_Vertex_3D), offset_of(HoViz_Vertex_3D, position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(HoViz_Vertex_3D), offset_of(HoViz_Vertex_3D, color));

    ctx.uloc_model = glGetUniformLocation(shader, "model_matrix");
    ctx.uloc_view = glGetUniformLocation(shader, "view_matrix");
    ctx.uloc_projection = glGetUniformLocation(shader, "projection_matrix");

    return 0;
}

static int 
init_triangles(int batch_size, u32 shader) {
    glGenVertexArrays(1, &ctx.triangles_vao);
	glBindVertexArray(ctx.triangles_vao);

    glGenBuffers(1, &ctx.triangles_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.triangles_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(HoViz_Vertex_3D) * batch_size, 0, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(HoViz_Vertex_3D), offset_of(HoViz_Vertex_3D, position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(HoViz_Vertex_3D), offset_of(HoViz_Vertex_3D, color));

    ctx.uloc_model = glGetUniformLocation(shader, "model_matrix");
    ctx.uloc_view = glGetUniformLocation(shader, "view_matrix");
    ctx.uloc_projection = glGetUniformLocation(shader, "projection_matrix");

    return 0;
}

static int
init_lines(int batch_size, u32 shader) {
    glGenVertexArrays(1, &ctx.lines_vao);
	glBindVertexArray(ctx.lines_vao);

    glGenBuffers(1, &ctx.lines_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.lines_vbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(HoViz_Vertex_3D) * batch_size, 0, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(HoViz_Vertex_3D), offset_of(HoViz_Vertex_3D, position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(HoViz_Vertex_3D), offset_of(HoViz_Vertex_3D, color));

    ctx.uloc_model = glGetUniformLocation(shader, "model_matrix");
    ctx.uloc_view = glGetUniformLocation(shader, "view_matrix");
    ctx.uloc_projection = glGetUniformLocation(shader, "projection_matrix");

    return 0;
}

int hoviz_should_close() {
    return glfwWindowShouldClose(ctx.window);
}

int 
hoviz_init_3D()
{
    if(glfwInit() == -1) {
        printf("Error: glfw could not initialize\n");
        return -1;
    }

    ctx.window = glfwCreateWindow(640, 480, "HoViz", 0, 0);
    if (!ctx.window) {
        printf("Error: glfw could not create window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(ctx.window);

    glfwSwapInterval(1);

    input_set_callbacks(ctx.window);

    if(hogl_init_gl_extensions() == -1) {
        printf("Error: ho_gl could not initialize extensions\n");
        return -1;
    }

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int size = 1024 * 1024;
    ctx.shader = shader_new_lines();

    init_lines(size, ctx.shader);
    init_triangles(size, ctx.shader);
    init_points(size, ctx.shader);

    camera_quat_init(&ctx.camera, (vec3){3.0f, 3.0f, 10.0f}, -0.01f, -1000.0f, 90.0f);

    ctx.last_frame_start = os_time_us();

    return 0;
}

void
hoviz_render_line(vec3 start, vec3 end, vec4 color)
{
    if(ctx.lines_ptr == 0) {
        glBindVertexArray(ctx.lines_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ctx.lines_vbo);
        ctx.lines_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(ctx.lines_ptr);
    }

    HoViz_Vertex_3D* lines = ((HoViz_Vertex_3D*)ctx.lines_ptr) + (2 * ctx.lines_count);

    lines[0].position = start;
    lines[0].color = color;

    lines[1].position = end;
    lines[1].color = color;

    ctx.lines_count++;
}

void
hoviz_render_vec3_from_start(vec3 start, vec3 v, vec4 color)
{
    if(ctx.lines_ptr == 0) {
        glBindVertexArray(ctx.lines_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ctx.lines_vbo);
        ctx.lines_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(ctx.lines_ptr);
    }

    HoViz_Vertex_3D* lines = ((HoViz_Vertex_3D*)ctx.lines_ptr) + (2 * ctx.lines_count);

    lines[0].position = start;
    lines[0].color = color;

    lines[1].position = gm_vec3_add(v, start);
    lines[1].color = color;

    ctx.lines_count++;
}

void
hoviz_render_vec3(vec3 v, vec4 color)
{
    if(ctx.lines_ptr == 0) {
        glBindVertexArray(ctx.lines_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ctx.lines_vbo);
        ctx.lines_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(ctx.lines_ptr);
    }

    HoViz_Vertex_3D* lines = ((HoViz_Vertex_3D*)ctx.lines_ptr) + (2 * ctx.lines_count);

    lines[0].position = (vec3){0, 0, 0};
    lines[0].color = color;

    lines[1].position = v;
    lines[1].color = color;

    ctx.lines_count++;
}

void
hoviz_render_triangle(vec3 v1, vec3 v2, vec3 v3, vec4 color)
{
    if(ctx.triangles_ptr == 0) {
        glBindVertexArray(ctx.triangles_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ctx.triangles_vbo);
        ctx.triangles_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(ctx.triangles_ptr);
    }

    HoViz_Vertex_3D* triangles = ((HoViz_Vertex_3D*)ctx.triangles_ptr) + (3 * ctx.triangles_count);

    triangles[0].position = v1;
    triangles[0].color = color;

    triangles[1].position = v2;
    triangles[1].color = color;

    triangles[2].position = v3;
    triangles[2].color = color;

    ctx.triangles_count++;
}

void
hoviz_render_point(vec3 p, vec4 color) {
    if(ctx.points_ptr == 0) {
        glBindVertexArray(ctx.points_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ctx.points_vbo);
        ctx.points_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(ctx.points_ptr);
    }

    HoViz_Vertex_3D* points = ((HoViz_Vertex_3D*)ctx.points_ptr) + (ctx.points_count);

    points[0].position = p;
    points[0].color = color;

    ctx.points_count++;
}

static void
render_triangles() {
    glUseProgram(ctx.shader);

    glBindVertexArray(ctx.triangles_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.triangles_vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glUniformMatrix4fv(ctx.uloc_view, 1, GL_TRUE, (GLfloat*)ctx.camera.c.view_matrix.data);
    glUniformMatrix4fv(ctx.uloc_projection, 1, GL_TRUE, (GLfloat*)ctx.camera.c.projection_matrix.data);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 3 * ctx.triangles_count);

    ctx.triangles_ptr = 0;
    ctx.triangles_count = 0;
}

static void
render_lines() {
    glUseProgram(ctx.shader);

    glBindVertexArray(ctx.lines_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.lines_vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glUniformMatrix4fv(ctx.uloc_view, 1, GL_TRUE, (GLfloat*)ctx.camera.c.view_matrix.data);
    glUniformMatrix4fv(ctx.uloc_projection, 1, GL_TRUE, (GLfloat*)ctx.camera.c.projection_matrix.data);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, 2 * ctx.lines_count);

    ctx.lines_ptr = 0;
    ctx.lines_count = 0;
}

static void
render_points() {
    glUseProgram(ctx.shader);

    glBindVertexArray(ctx.points_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.points_vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glUniformMatrix4fv(ctx.uloc_view, 1, GL_TRUE, (GLfloat*)ctx.camera.c.view_matrix.data);
    glUniformMatrix4fv(ctx.uloc_projection, 1, GL_TRUE, (GLfloat*)ctx.camera.c.projection_matrix.data);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, ctx.points_count);

    ctx.points_ptr = 0;
    ctx.points_count = 0;
}

void
hoviz_flush() 
{
    camera_update(&ctx.camera, 1.0 / 60.0);

    // Setup uniforms
    
    camera_quat_force_matrix_recalculation(&ctx.camera);

    render_triangles();
    render_lines();
    render_points();

    r64 end_time = os_time_us();
    r64 elapsed_us = (end_time - ctx.last_frame_start);
    ctx.elapsed += (elapsed_us / 1000.0);
    ctx.fps++;

    if(ctx.elapsed >= 1000.0) {
        printf("FPS: %d (average frame time: %fms)\n", ctx.fps, ctx.elapsed / (r64)ctx.fps);
        ctx.elapsed = 0.0;
        ctx.fps = 0;
    }

#if 0
    u64 sleep_time_us = (u64)(((1.0 / 60.0) * 1000000) - elapsed_us);
    if((r64)sleep_time_us < (1.0 / 60.0) * 1000000.0) {
        os_usleep(sleep_time_us);
    }

    ctx.elapsed += ((r64)sleep_time_us / 1000.0);
#endif

    ctx.last_frame_start = os_time_us();

    // Show this frame
    glfwSwapBuffers(ctx.window);

    memset(hoviz_input_state.key_event, 0, sizeof(hoviz_input_state.key_event));
    // Poll events for next frame
    glfwPollEvents();

    // clear next frame
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    int width, height;
    window_get_size(&width, &height);
    glViewport(0, 0, width, height);
}


// CAMERA

static void handle_mouse_change(bool move, r64 x, r64 y)
{
    static bool reset_rotate = true;
    static bool reset_move = true;
    static r64 x_pos_old_rotate, y_pos_old_rotate;
    static r64 x_pos_old_move, y_pos_old_move;
    
    if(move) {
        if (!reset_rotate)
        {
            r64 x_difference = x - x_pos_old_rotate;
            r64 y_difference = -y - y_pos_old_rotate;

            Camera* camera = &ctx.camera;
            camera_quat_rotate_x(camera, (r32)x_difference * CAMERA_ROTATE_VELOCITY_X);
            camera_quat_rotate_y(camera, (r32)y_difference * CAMERA_ROTATE_VELOCITY_Y);
        }

        reset_rotate = false;
        x_pos_old_rotate = x;
        y_pos_old_rotate = -y;
    } else {
        reset_rotate = true;
    }
}

static void camera_update(Camera* camera, r64 delta_time)
{
    float CAMERA_QUAT_MOVEMENT_SPEED_MODIFIER = 10.0f;
    int* key_states = hoviz_input_state.key_state;

    if(hoviz_input_state.mods & GLFW_MOD_SHIFT) {
        CAMERA_QUAT_MOVEMENT_SPEED_MODIFIER /= 5.0f;
    }

    if (key_states['W'] || key_states['w']){
        camera_quat_move_forward(camera, CAMERA_QUAT_MOVEMENT_SPEED_MODIFIER * CAMERA_VELOCITY * (r32)delta_time);
    }
    else if (key_states['S'] || key_states['s']) {
        camera_quat_move_forward(camera, -CAMERA_QUAT_MOVEMENT_SPEED_MODIFIER * CAMERA_VELOCITY * (r32)delta_time);
    }
    else if (key_states['A'] || key_states['a']) {
        camera_quat_move_right(camera, -CAMERA_QUAT_MOVEMENT_SPEED_MODIFIER * CAMERA_VELOCITY * (r32)delta_time);
    }
    else if (key_states['D'] || key_states['d']) {
        camera_quat_move_right(camera, CAMERA_QUAT_MOVEMENT_SPEED_MODIFIER * CAMERA_VELOCITY * (r32)delta_time);
    }
    handle_mouse_change(hoviz_input_state.mouse_buttons[1].state, hoviz_input_state.mouse_position.x, hoviz_input_state.mouse_position.y);
}

void hoviz_camera_reset() {
    camera_quat_init(&ctx.camera, (vec3){3.0f, 3.0f, 10.0f}, -0.01f, -1000.0f, 90.0f);
}