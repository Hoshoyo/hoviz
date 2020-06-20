#pragma once
#include <gm.h>
#include "quaternion.h"

typedef struct
{
	vec3 position;
	r32 near_plane;
	r32 far_plane;
	r32 fov;
	mat4 view_matrix;
	mat4 projection_matrix;
} Camera_Common;

vec3 camera_get_view(const Camera_Common* camera);

typedef struct
{
	Camera_Common c;
	Quaternion rotation;
	Quaternion yrotation;
	r32 move_speed, xrot_speed, yrot_speed;
} Camera;

void camera_quat_init(Camera* camera, vec3 position, r32 near_plane, r32 far_plane, r32 fov);
void camera_quat_set_position(Camera* camera, vec3 position);
void camera_quat_set_near_plane(Camera* camera, r32 nearPlane);
void camera_quat_set_far_plane(Camera* camera, r32 farPlane);
void camera_quat_set_fov(Camera* camera, r32 fov);
void camera_quat_force_matrix_recalculation(Camera* camera);
void camera_quat_rotate(Camera* camera, r32 x_difference, r32 y_difference);
void camera_quat_rotate_x(Camera* camera, r32 x_difference);
void camera_quat_rotate_y(Camera* camera, r32 y_difference);
void camera_quat_move_forward(Camera* camera, r32 amount);
void camera_quat_move_right(Camera* camera, r32 amount);
vec3 camera_quat_get_x_axis(const Camera* camera);
vec3 camera_quat_get_y_axis(const Camera* camera);
vec3 camera_quat_get_z_axis(const Camera* camera);