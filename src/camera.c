#include "camera.h"
#include <math.h>
#include "quaternion.h"

void window_get_size(int* width, int* height);

vec3 camera_get_view(const Camera_Common* camera)
{
	mat4 camera_view_matrix = camera->view_matrix;
	vec3 camera_view = (vec3) {-camera_view_matrix.data[2][0],
		-camera_view_matrix.data[2][1], -camera_view_matrix.data[2][2] };
	return gm_vec3_normalize(camera_view);
}

/***********
 * QUATERNION CAMERA
 * *************/

static void camera_recalculate_projection_matrix(Camera_Common* camera)
{
    s32 window_width;
    s32 window_height;
	window_get_size(&window_width, &window_height);
    // ---------------------------------------------------------

	r32 _near = camera->near_plane;
	r32 _far = camera->far_plane;
	r32 top = (r32)fabs(_near) * atanf(gm_radians(camera->fov) / 2.0f);
	r32 bottom = -top;
	r32 right = top * ((r32)window_width / (r32)window_height);
	r32 left = -right;

	mat4 P = (mat4) {
		_near, 0, 0, 0,
			0, _near, 0, 0,
			0, 0, _near + _far, -_near * _far,
			0, 0, 1, 0
	};

	mat4 M = (mat4) {
		2.0f / (right - left), 0, 0, -(right + left) / (right - left),
			0, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
			0, 0, 2.0f / (_far - _near), -(_far + _near) / (_far - _near),
			0, 0, 0, 1
	};

	// Need to transpose when sending to shader
	mat4 MP = gm_mat4_multiply(&M, &P);
	camera->projection_matrix = gm_mat4_scalar_product(-1, &MP);
}

static void camera_quat_recalculate_view_matrix(Camera* camera)
{
	mat4 trans = (mat4) {
		1.0f, 0.0f, 0.0f, -camera->c.position.x,
		0.0f, 1.0f, 0.0f, -camera->c.position.y,
		0.0f, 0.0f, 1.0f, -camera->c.position.z,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	Quaternion f = quaternion_product(&camera->rotation, &camera->yrotation);

	//mat4 rot = quaternion_get_matrix(&camera->rotation);
	//mat4 yrot = quaternion_get_matrix(&camera->yrotation);
	//mat4 tmp = gm_mat4_multiply(&rot, &yrot);
	mat4 rotation = quaternion_get_matrix(&f);
	camera->c.view_matrix = gm_mat4_multiply(&rotation, &trans);
}


void camera_quat_init(Camera* camera, vec3 position, r32 near_plane, r32 far_plane, r32 fov)
{
	camera->c.position = position;
	camera->c.near_plane = near_plane;
	camera->c.far_plane = far_plane;
	camera->c.fov = fov;
	camera->rotation = (Quaternion) { 0.0f, 0.0f, 0.0f, 1.0f };
	camera->yrotation = (Quaternion) { 0.0f, 0.0f, 0.0f, 1.0f };

	camera_quat_recalculate_view_matrix(camera);
	camera_recalculate_projection_matrix(&camera->c);
}

void camera_quat_rotate_x(Camera* camera, r32 x_difference) {
	Quaternion yAxis = quaternion_new((vec3) { 0.0f, 1.0f, 0.0f }, x_difference);
	camera->yrotation = quaternion_product(&yAxis, &camera->yrotation);
	quaternion_normalize(&camera->yrotation);
	camera_quat_recalculate_view_matrix(camera);
}

void camera_quat_rotate_y(Camera* camera, r32 y_difference) {
	vec3 right = quaternion_get_right(&camera->rotation);
	right = gm_vec3_normalize(right);
	Quaternion xAxis = quaternion_new(right, y_difference);
	camera->rotation = quaternion_product(&camera->rotation, &xAxis);
	quaternion_normalize(&camera->rotation);
	camera_quat_recalculate_view_matrix(camera);
}

void camera_quat_rotate(Camera* camera, r32 x_difference, r32 y_difference) {

	Quaternion yAxis = quaternion_new((vec3) { 0.0f, 1.0f, 0.0f }, x_difference);
	camera->yrotation = quaternion_product(&yAxis, &camera->yrotation);
	quaternion_normalize(&camera->yrotation);

	vec3 right = quaternion_get_right(&camera->rotation);
	right = gm_vec3_normalize(right);
	Quaternion xAxis = quaternion_new(right, y_difference);
	camera->rotation = quaternion_product(&camera->rotation, &xAxis);
	quaternion_normalize(&camera->rotation);

	camera_quat_recalculate_view_matrix(camera);
}

void camera_quat_move_forward(Camera* camera, r32 amount) {
	Quaternion f = quaternion_product(&camera->rotation, &camera->yrotation);

	vec3 forward = quaternion_get_forward(&f);
	forward = gm_vec3_scalar_product(amount, gm_vec3_normalize(forward));
	camera->c.position = gm_vec3_add((vec3) { -forward.x, -forward.y, -forward.z }, camera->c.position);

	camera_quat_recalculate_view_matrix(camera);
}

void camera_quat_move_right(Camera* camera, r32 amount) {
	Quaternion f = quaternion_product(&camera->rotation, &camera->yrotation);

	vec3 right = quaternion_get_right(&f);
	right = gm_vec3_scalar_product(amount, gm_vec3_normalize(right));
	camera->c.position = gm_vec3_add((vec3) { right.x, right.y, right.z }, camera->c.position);

	camera_quat_recalculate_view_matrix(camera);
}

void camera_quat_set_position(Camera* camera, vec3 position)
{
	camera->c.position = position;
	camera_quat_recalculate_view_matrix(camera);
}

void camera_quat_set_near_plane(Camera* camera, r32 near_plane)
{
	camera->c.near_plane = near_plane;
	camera_recalculate_projection_matrix(&camera->c);
}

void camera_quat_set_far_plane(Camera* camera, r32 far_plane)
{
	camera->c.far_plane = far_plane;
	camera_recalculate_projection_matrix(&camera->c);
}

void camera_quat_set_fov(Camera* camera, r32 fov)
{
	camera->c.fov = fov;
	camera_recalculate_projection_matrix(&camera->c);
}

void camera_quat_force_matrix_recalculation(Camera* camera)
{
	camera_quat_recalculate_view_matrix(camera);
	camera_recalculate_projection_matrix(&camera->c);
}

vec3 camera_quat_get_x_axis(const Camera* camera)
{
	Quaternion q = quaternion_product(&camera->rotation, &camera->yrotation);
	vec3 right = quaternion_get_right(&q);
	right = gm_vec3_normalize(right);
	return (vec3) {right.x, right.y, right.z};
}

vec3 camera_quat_get_y_axis(const Camera* camera)
{
	Quaternion q = quaternion_product(&camera->rotation, &camera->yrotation);
	vec3 up = quaternion_get_up(&q);
	up = gm_vec3_normalize(up);
	return (vec3) {up.x, up.y, up.z};
}

vec3 camera_quat_get_z_axis(const Camera* camera)
{
	Quaternion q = quaternion_product(&camera->rotation, &camera->yrotation);
	vec3 forward = quaternion_get_forward(&q);
	forward = gm_vec3_normalize(forward);
	return (vec3) {forward.x, forward.y, forward.z};
}