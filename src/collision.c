#include "collision.h"
#include <float.h>
#include <light_array.h>
#include "hoviz.h"

// GJK

static void support_list_add(GJK_Support_List* list, vec3 v) {
	list->list[list->current_index] = v;
	list->current_index++;
}

void 
collision_transform_shape(Bounding_Shape* base, mat4* m)
{
	for (int i = 0; i < base->vertex_count; ++i) {
		base->vertices[i] = gm_mat4_multiply_vec3(m, base->vertices[i]);
	}
}

vec3 collision_gjk_support(Bounding_Shape* b1, Bounding_Shape* b2, vec3 direction) 
{
	float max = -FLT_MAX;
	int index = 0;
	for (int i = 0; i < b1->vertex_count; ++i)
	{
		float dot = gm_vec3_dot(b1->vertices[i], direction);
		if (dot > max) 	{
			max = dot;
			index = i;
		}
	}
	int b1_index = index;

	max = -FLT_MAX;
	index = 0;
	for (int i = 0; i < b2->vertex_count; ++i)
	{
		float dot = gm_vec3_dot(b2->vertices[i], gm_vec3_subtract((vec3){0.0f, 0.0f, 0.0f}, direction));
		if (dot > max) {
			max = dot;
			index = i;
		}
	}
	int b2_index = index;

	vec3 result = gm_vec3_subtract(b1->vertices[b1_index], b2->vertices[b2_index]);
	return result;
}

#define GJK_3D
bool gjk_simplex(GJK_Support_List* support_list, vec3* direction)
{
	int num_entries = support_list->current_index;

	if (num_entries == 2) {
		vec3 A = support_list->list[1];
		vec3 B = support_list->list[0];
		vec3 AO = gm_vec3_negative(A);	//(0,0,0) - A
		vec3 AB = gm_vec3_subtract(B, A);
		if (gm_vec3_dot(AB, AO) > 0) {
			*direction = gm_vec3_cross(gm_vec3_cross(AB, AO), AB);
		} else {
			support_list->list[0] = support_list->list[1];
			support_list->current_index--;
			*direction = AO;
		}
	}
	else if (num_entries == 3) {
		vec3 B = support_list->list[0];
		vec3 C = support_list->list[1];
		vec3 A = support_list->list[2];

		vec3 AB = gm_vec3_subtract(B, A);
		vec3 AC = gm_vec3_subtract(C, A);
		vec3 ABC = gm_vec3_cross(AB, AC);	// normal to the triangle
		vec3 AO = gm_vec3_negative(A);

		vec3 edge4 = gm_vec3_cross(AB, ABC);
		vec3 edge1 = gm_vec3_cross(ABC, AC);

		if (gm_vec3_dot(edge1, AO) > 0) {
			if (gm_vec3_dot(AC, AO) > 0) {
				// simplex is A, C

				support_list->list[0] = A;
				support_list->current_index--;
				*direction = gm_vec3_cross(gm_vec3_cross(AC, AO), AC);
			} else {
				if (gm_vec3_dot(AB, AO) > 0) {
					// simplex is A, B

					support_list->list[0] = A;
					support_list->list[1] = B;
					support_list->current_index--;
					*direction = gm_vec3_cross(gm_vec3_cross(AB, AO), AB);
				} else {
					// simplex is A

					support_list->list[0] = A;
					support_list->current_index -= 2;
					*direction = AO;
				}
			}
		} else {
			if (gm_vec3_dot(edge4, AO) > 0) {
				if (gm_vec3_dot(AB, AO) > 0) {
					// simplex is A, B

					support_list->list[0] = A;
					support_list->list[1] = B;
					support_list->current_index--;
					*direction = gm_vec3_cross(gm_vec3_cross(AB, AO), AB);
				} else {
					// simplex is A

					support_list->list[0] = A;
					support_list->current_index -= 2;
					*direction = AO;
				}
			}
			else {
				// for 2D this is enough
#ifndef GJK_3D
				return true;
#else
				if (gm_vec3_dot(ABC, AO) > 0) {
					// simplex is A, B, C

					support_list->list[0] = A;
					support_list->list[1] = B;
					support_list->list[2] = C;
					*direction = ABC;
				} else {
					// simplex is A, C, B
					support_list->list[0] = A;
					support_list->list[1] = C;
					support_list->list[2] = B;
					*direction = gm_vec3_negative(ABC);
				}
#endif
			}
		}
	}
#ifdef GJK_3D
	else if (num_entries == 4) {
		vec3 A = support_list->list[3];
		vec3 B = support_list->list[2];
		vec3 C = support_list->list[1];
		vec3 D = support_list->list[0];

		vec3 AO = gm_vec3_negative(A);
		vec3 AB = gm_vec3_subtract(B, A);
		vec3 AC = gm_vec3_subtract(C, A);
		vec3 AD = gm_vec3_subtract(D, A);

		vec3 ABC = gm_vec3_cross(AB, AC);

		if (gm_vec3_dot(ABC, AO) > 0) {
			// in front of ABC

			support_list->list[0] = C;
			support_list->list[1] = B;
			support_list->list[2] = A;
			support_list->current_index--;
			*direction = ABC;
			return false;
		}

		vec3 ADB = gm_vec3_cross(AD, AB);
		if (gm_vec3_dot(ADB, AO) > 0) {
			// in front of ADB

			support_list->list[0] = B;
			support_list->list[1] = D;
			support_list->list[2] = A;
			support_list->current_index--;
			*direction = ADB;
			return false;
		}

		vec3 ACD = gm_vec3_cross(AC, AD);
		if (gm_vec3_dot(ACD, AO) > 0) {
			// in front of ACD

			support_list->list[0] = D;
			support_list->list[1] = C;
			support_list->list[2] = A;
			support_list->current_index--;
			*direction = ACD;
			return false;
		}

		return true;	// inside the tetrahedron
	}
#endif
	return false;
}

bool collision_gjk_collides(GJK_Support_List* sup_list, Bounding_Shape* b1, Bounding_Shape* b2)
{
	vec3 search_direction = (vec3){1.0f, 0.0f, 0.0f};
	vec3 support = collision_gjk_support(b1, b2, search_direction);

	support_list_add(sup_list, support);

	vec3 opposite_direction = gm_vec3_negative(search_direction);

	int max = 200;
	while (true) {
		if (max <= 0) return true;
		max--;
		vec3 a = collision_gjk_support(b1, b2, opposite_direction);

		float dotval = gm_vec3_dot(a, opposite_direction);
		if (dotval < 0) {
			return false;	// there is no intersection
		}

		support_list_add(sup_list, a);
		if (gjk_simplex(sup_list, &opposite_direction)) {
			return true;
		}
	}
}

// EPA algorithm

typedef struct {
	vec3 a;
    vec3 b;
	vec3 c;
    vec3 normal;
    r32 distance;
	int index;
} Face;

vec3 triangle_centroid(Face f) {
	r32 ox = (f.a.x + f.b.x + f.c.x) / 3.0f;
	r32 oy = (f.a.y + f.b.y + f.c.y) / 3.0f;
	r32 oz = (f.a.z + f.b.z + f.c.z) / 3.0f;
	return (vec3) {ox, oy, oz};
}

static Face face_new(r32* distance, int index, int* out_index, vec3 v1, vec3 v2, vec3 v3) {
    Face f = {0};
    f.a = v1;
    f.b = v2;
    f.c = v3;

    //vec3 normal = gm_vec3_cross(gm_vec3_subtract(v2, v1), gm_vec3_subtract(v3, v1));
	vec3 ba = gm_vec3_subtract(f.b, f.a);
	vec3 ca = gm_vec3_subtract(f.c, f.a);
	vec3 normal = gm_vec3_cross(ba, ca);

	if(gm_vec3_dot(normal, v1) < 0.0f) {
		normal = gm_vec3_negative(normal);
	}

	r32 d = gm_vec3_dot(f.a, normal);
	f.distance = d;
	f.normal = normal;

	if(f.distance < *distance) {
		*distance = f.distance;
		*out_index = index;
	}
	return f;
}

vec3 collision_epa(vec3* simplex, Bounding_Shape* b1, Bounding_Shape* b2) {
    // Simplex faces
    // 0 1 2
    // 0 1 3
    // 1 2 3
    // 0 2 3

	r32 distance = FLT_MAX;

	int index = -1;
	Face faces[4] = {0};

	//while(1) 
	{
		faces[0] = face_new(&distance, 0, &index, simplex[0], simplex[1], simplex[2]);
		faces[1] = face_new(&distance, 1, &index, simplex[0], simplex[1], simplex[3]);
		faces[2] = face_new(&distance, 2, &index, simplex[1], simplex[2], simplex[3]);
		faces[3] = face_new(&distance, 3, &index, simplex[0], simplex[2], simplex[3]);

		// closest face is the face at index

		// Find the new support in the normal direction of the closest face
		vec3 p = collision_gjk_support(b1, b2, faces[index].normal);
		hoviz_render_point(p, (vec4){1.0f, 0.0f, 1.0f, 1.0f});

		if(gm_vec3_dot(p, faces[index].normal) - faces[index].distance < 0.001f) 
		{
			vec3 penetration = gm_vec3_scalar_product(faces[index].distance, gm_vec3_normalize(faces[index].normal));
			hoviz_render_vec3(penetration, (vec4){0.3f, 0.3f, 1.0f, 1.0f});
			return penetration;
		}

		// Expand polytope
		distance = FLT_MAX;
		index = -1;

		r32 f1 = gm_vec3_dot(faces[0].normal, p);
		r32 f2 = gm_vec3_dot(faces[1].normal, p);
		r32 f3 = gm_vec3_dot(faces[2].normal, p);
		r32 f4 = gm_vec3_dot(faces[3].normal, p);

		vec4 color = (vec4) {1.0f, 0.0f, 0.0f, 1.0f};
		for(int i = 0; i < 4; ++i) {
			vec3 centroid = triangle_centroid(faces[i]);
			r32 dot = gm_vec3_dot(faces[i].normal, p);
			if(dot >= 0.0f) {
				hoviz_render_vec3_from_start(centroid, faces[i].normal, color);
			} else {
				hoviz_render_vec3_from_start(centroid, faces[i].normal, (vec4){1.0f, 1.0f, 0.3f, 0.9f});
			}

			hoviz_render_vec3_from_start(faces[i].a, faces[i].b, (vec4){1.0f, 1.0f, 1.0f, 1.0f});
			hoviz_render_vec3_from_start(faces[i].b, faces[i].c, (vec4){1.0f, 1.0f, 1.0f, 1.0f});
			hoviz_render_vec3_from_start(faces[i].c, faces[i].a, (vec4){1.0f, 1.0f, 1.0f, 1.0f});
		}

		int xx = 0;
	}
}

#if 0
typedef struct {
	vec3 a;
    vec3 b;
	vec3 c;
    vec3 normal;
    r32 distance;
    s32 index;
} Face;
static Face find_closest_face(vec3* simplex) {
    Face closest = {0};
    closest.distance = FLT_MAX;

    for(int i = 0; i < array_length(simplex); ++i) {
		int j = (i + 1) % array_length(simplex);
		int k = (i + 2) % array_length(simplex);

        vec3 a = simplex[i];
        vec3 b = simplex[j];
		vec3 c = simplex[k];

		vec3 ba = gm_vec3_subtract(b, a);
		vec3 ca = gm_vec3_subtract(c, a);
		vec3 n = gm_vec3_normalize(gm_vec3_cross(ba, ca));

		vec3 ao = a;

        r32 d = gm_vec3_dot(n, a);

		if(gm_vec3_dot(ao, n) > 0.0f){
            n = gm_vec3_negative(n);
			d = gm_vec3_dot(n, a);
		}

        if(d < closest.distance) {
            closest.distance = d;
            closest.normal = n;
            closest.a = a;
            closest.b = b;
			closest.c = c;
            closest.index = k;
        }
    }
    return closest;
}

extern int global_counter;
void expanding_polytope_algorithm(vec3* gjk_simplex, Bounding_Shape* s1, Bounding_Shape* s2) 
{
	vec3* simplex = array_new(vec3);
	array_push(simplex, gjk_simplex[0]);
	array_push(simplex, gjk_simplex[1]);
	array_push(simplex, gjk_simplex[2]);

	for(int ii = 0; ii < 10; ++ii)
	{
        Face face = find_closest_face(simplex);
		vec3 p = collision_gjk_support(s1, s2, face.normal);
        
        if(global_counter == ii) {
            for(int jj = 0; jj < array_length(simplex); ++jj) {
                hoviz_render_vec3_from_start(simplex[jj], simplex[(jj + 1) % array_length(simplex)], (vec4){1.0f, 0.0f, 0.0f, 1.0f});
            }
            hoviz_render_triangle(face.a, face.b, face.c, (vec4){1.0f, 1.0f, 0.0f, 0.4f});

            hoviz_render_vec3(face.normal, (vec4){1.0f, 0.0f, 1.0f, 1.0f});
            hoviz_render_vec3(p, (vec4){1.0f, 1.0f, 0.0f, 1.0f});
        }

        r32 d = gm_vec3_dot(p, face.normal);

        r32 TOLERANCE = 0.0001f;

        if(d - face.distance < TOLERANCE) {
            vec3 normal = face.normal;
            r32 depth = d;

			//printf("{%f %f %f}, distance: %f\n", normal.x, normal.y, normal.z, depth);

            normal = gm_vec3_scalar_product(depth, gm_vec3_normalize(normal));

			array_free(simplex);
            return;
        } else {
            array_insert(simplex, p, face.index);
        }
    }
}
#endif