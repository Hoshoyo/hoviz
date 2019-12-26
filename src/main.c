#include <stdio.h>
#define GRAPHICS_MATH_IMPLEMENT
#define HOGL_IMPLEMENT
#include <ho_gl.h>
#include <gm.h>
#include <GLFW/glfw3.h>
#include "quaternion.h"
#include "collision.h"
#include "hoviz.h"

#define DEGTORAD(degree) ((degree) * (3.141592654f / 180.0f))

static mat4 mat4_rotate(vec3 axis, float angle) {
	mat4 result;
	angle = DEGTORAD(angle);
	float c = cosf(angle);
	float t = 1.0f - c;
	float s = sinf(angle);
	vec3 a = gm_vec3_normalize(axis);

	result.data[0][0] = t * a.x * a.x + c;			result.data[0][1] = t * a.x * a.y + s * a.z;	result.data[0][2] = t * a.x * a.z - s * a.y;	result.data[0][3] = 0;
	result.data[1][0] = t * a.x * a.y - s * a.z;	result.data[1][1] = t * a.y * a.y + c;			result.data[1][2] = t * a.y * a.z + s * a.x;	result.data[1][3] = 0;
	result.data[2][0] = t * a.x * a.z + s * a.y;	result.data[2][1] = t * a.y * a.z - s * a.x;	result.data[2][2] = t * a.z * a.z + c;			result.data[2][3] = 0;
	result.data[3][0] = 0;							result.data[3][1] = 0;							result.data[3][2] = 0;							result.data[3][3] = 1;

	return result;
}

void
transform(vec3* verts, int count, vec3 move) {
	mat4 m = gm_mat4_translate_transposed(move);
	for(int i = 0; i < count; ++i) {
		verts[i] = gm_mat4_multiply_vec3(&m, verts[i]);
	}
}
void
rotate(vec3* verts, int count, vec3 axis, r32 angle) {
	mat4 m = mat4_rotate(axis, angle);
	for(int i = 0; i < count; ++i) {
		verts[i] = gm_mat4_multiply_vec3(&m, verts[i]);
	}
}

void calculate_minkowski(vec3* mink, vec3* v1, vec3* v2, int v1_count, int v2_count) {
	int k = 0;
	for(int i = 0; i < v1_count; ++i) {
		for(int j = 0; j < v2_count; ++j) {
			mink[k] = gm_vec3_subtract(v1[i], v2[j]);
			k++;
		}
	}
}

static void normal_calc(vec3 v1, vec3 v2, vec3 v3) {
    vec3 normal = gm_vec3_cross(gm_vec3_subtract(v2, v1), gm_vec3_subtract(v3, v1));
	hoviz_render_vec3(normal, (vec4){1.0f, 1.0f, 0.0f, 1.0f});
	hoviz_render_triangle(v1, v2, v3, (vec4){1.0f, 0.0f, 0.0f, 1.0f});
}

int global_counter = 0;

int main(int argc, char** argv) {
	hoviz_init_3D();

	vec3 vecs[] = {
		(vec3){0, 0, 0}, (vec3){1, 0, 0},
		(vec3){0, 0, 1}, (vec3){1, 0, 1},
		(vec3){0, 1, 0}, (vec3){1, 1, 0},
		(vec3){0, 1, 1}, (vec3){1, 1, 1}
	};
	transform(vecs, 8, (vec3){3.3f, 0.0f, -1.0f});
	rotate(vecs, 8, (vec3){0, 1, 0}, 45);

	vec3 vecs2[] = {
		(vec3){4, 0, 0}, (vec3){6, 0, 0},
		(vec3){4, 0, 2}, (vec3){6, 0, 2},
		(vec3){4, 2, 0}, (vec3){6, 2, 0},
		(vec3){4, 2, 2}, (vec3){6, 2, 2}
	};

	vec3 minkowski[64] = {0};
	float velocity = 0.1f;

	Bounding_Shape b1 = {0};
	Bounding_Shape b2 = {0};
	b1.vertex_count = 8;
	b2.vertex_count = 8;
	b1.vertices = vecs;
	b2.vertices = vecs2;
	

	while(!hoviz_should_close())
	{
		// Render the 3 axis
		hoviz_render_vec3((vec3){10, 0, 0}, (vec4){1.0f, 0.0f, 0.0f, 0.3f});
		hoviz_render_vec3((vec3){0, 10, 0}, (vec4){0.0f, 1.0f, 0.0f, 0.3f});
		hoviz_render_vec3((vec3){0, 0, 10}, (vec4){0.0f, 0.0f, 1.0f, 0.3f});
		hoviz_render_point((vec3){0, 0, 0}, (vec4){0.0f, 1.0f, 0.0f, 1.0f});
		
		#if 1
		calculate_minkowski(minkowski, vecs, vecs2, 8, 8);
		if(hoviz_input_state.key_state[GLFW_KEY_LEFT_SHIFT]) {
			velocity = 0.01f;
		} else {
			velocity = 0.1f;
		}
		if(hoviz_input_state.key_state[GLFW_KEY_LEFT]) {
			transform(vecs, 8, (vec3){-velocity, 0.0f, 0.0f});
		}
		if(hoviz_input_state.key_state[GLFW_KEY_RIGHT]) {
			transform(vecs, 8, (vec3){velocity, 0.0f, 0.0f});
		}
		if(hoviz_input_state.key_state[GLFW_KEY_UP]) {
			transform(vecs, 8, (vec3){0.0f, 0.0f, -velocity});
		}
		if(hoviz_input_state.key_state[GLFW_KEY_DOWN]) {
			transform(vecs, 8, (vec3){0.0f, 0.0f, velocity});
		}
		if(hoviz_input_state.key_state[GLFW_KEY_R]) {
			transform(vecs, 8, (vec3){0.0f, velocity, 0.0f});
		}
		if(hoviz_input_state.key_state[GLFW_KEY_F]) {
			transform(vecs, 8, (vec3){0.0f, -velocity, 0.0f});
		}
		if(hoviz_input_state.key_event[GLFW_KEY_X]) {
			global_counter++;
			hoviz_input_state.key_event[GLFW_KEY_X] = 0;
			printf("Foo1\n");
		}
		if(hoviz_input_state.key_event[GLFW_KEY_Z]) {
			global_counter--;
			if(global_counter < 0) global_counter = 0;
			hoviz_input_state.key_event[GLFW_KEY_Z] = 0;
			printf("Foo2\n");
		}

		// Render the shapes
		for(int i = 0; i < 8; ++i) {
			hoviz_render_point(vecs[i], (vec4){1.0f, 1.0f, 0.0f, 1.0f});
			hoviz_render_point(vecs2[i], (vec4){0.0f, 1.0f, 1.0f, 1.0f});
		}
		for(int i = 0; i < 64; ++i) {
			hoviz_render_point(minkowski[i], (vec4){1.0f, 1.0f, 1.0f, 1.0f});
		}

		GJK_Support_List sup_list = {0};
		if(collision_gjk_collides(&sup_list, &b1, &b2)) {
			assert(sup_list.current_index == 4);
			//expanding_polytope_algorithm(sup_list.list, &b1, &b2);
			collision_epa(sup_list.list, &b1, &b2);
			// Render the GJK simplex
			hoviz_render_triangle(sup_list.list[0], sup_list.list[1], sup_list.list[2], (vec4){0.4f, 0.45f, 1.0f, 0.4f});
			hoviz_render_triangle(sup_list.list[0], sup_list.list[1], sup_list.list[3], (vec4){0.4f, 0.48f, 1.0f, 0.4f});
			hoviz_render_triangle(sup_list.list[0], sup_list.list[2], sup_list.list[3], (vec4){0.4f, 0.57f, 1.0f, 0.4f});
			hoviz_render_triangle(sup_list.list[1], sup_list.list[2], sup_list.list[3], (vec4){0.4f, 0.60f, 1.0f, 0.4f});
		}
		#endif

		hoviz_flush();
	}

	return 0;
}