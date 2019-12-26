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

bool collision_gjk_collides(GJK_Support_List* sup_list, Bounding_Shape* b1, Bounding_Shape* b2);
void collision_transform_shape(Bounding_Shape* base, mat4* m);
Bounding_Shape collision_bounding_shape_new(vec3 size);
//void expanding_polytope_algorithm(vec3* gjk_simplex, Bounding_Shape* s1, Bounding_Shape* s2);
vec3 collision_epa(vec3* simplex, Bounding_Shape* b1, Bounding_Shape* b2);