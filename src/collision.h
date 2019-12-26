#pragma once
#include <gm.h>

typedef struct {
	vec3 list[4];
	int current_index;
} GJK_Support_List;

typedef struct {
	s32   vertex_count;
	vec3* vertices;
} Bounding_Shape;

extern vec4 hoviz_red;
extern vec4 hoviz_green;
extern vec4 hoviz_blue;

bool collision_gjk_collides(GJK_Support_List* sup_list, Bounding_Shape* b1, Bounding_Shape* b2);
void collision_transform_shape(Bounding_Shape* base, mat4* m);
vec3 collision_epa(vec3* simplex, Bounding_Shape* b1, Bounding_Shape* b2);