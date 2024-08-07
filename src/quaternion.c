#include "quaternion.h"

Quaternion quaternion_new(vec3 axis, r32 angle)
{
    if (gm_vec3_length(axis) != 0.0f)
        axis = gm_vec3_normalize(axis);
    r32 sang = sinf(gm_radians(angle) / 2.0f);

    Quaternion quat;
    quat.w = cosf(gm_radians(angle / 2.0f));
    quat.x = axis.x * sang;
    quat.y = axis.y * sang;
    quat.z = axis.z * sang;

    return quat;
}

vec3 quaternion_get_right(const Quaternion* quat) {
	mat4 m = quaternion_get_matrix(quat);
	return 
	#if !defined(__cplusplus)
	(vec3) 
	#endif
	{ m.data[0][0], m.data[0][1], m.data[0][2] };
}

vec3 quaternion_get_up(const Quaternion* quat) {
	mat4 m = quaternion_get_matrix(quat);
	return 
	#if !defined(__cplusplus)
	(vec3) 
	#endif
	{ m.data[1][0], m.data[1][1], m.data[1][2] };
}

vec3 quaternion_get_forward(const Quaternion* quat) {
	mat4 m = quaternion_get_matrix(quat);
	return 
	#if !defined(__cplusplus)
	(vec3) 
	#endif
	{ m.data[2][0], m.data[2][1], m.data[2][2] };
}

Quaternion quaternion_inverse(const Quaternion* q)
{
	Quaternion result;
	result.x = -q->x;
	result.y = -q->y;
	result.z = -q->z;
	result.w = q->w;
	return result;
}

mat4 quaternion_get_matrix(const Quaternion* quat)
{
    mat4 result;

	result.data[0][0] = 1.0f - 2.0f * quat->y * quat->y - 2.0f * quat->z * quat->z;
	result.data[1][0] = 2.0f * quat->x * quat->y + 2.0f * quat->w * quat->z;
	result.data[2][0] = 2.0f * quat->x * quat->z - 2.0f * quat->w * quat->y;
	result.data[3][0] = 0.0f;

	result.data[0][1] = 2.0f * quat->x * quat->y - 2.0f * quat->w * quat->z;
	result.data[1][1] = 1.0f - (2.0f * quat->x * quat->x) - (2.0f * quat->z * quat->z);
	result.data[2][1] = 2.0f * quat->y * quat->z + 2.0f * quat->w * quat->x;
	result.data[3][1] = 0.0f;

	result.data[0][2] = 2.0f * quat->x * quat->z + 2.0f * quat->w * quat->y;
	result.data[1][2] = 2.0f * quat->y * quat->z - 2.0f * quat->w * quat->x;
	result.data[2][2] = 1.0f - (2.0f * quat->x * quat->x) - (2.0f * quat->y * quat->y);
	result.data[3][2] = 0.0f;

	result.data[0][3] = 0.0f;
	result.data[1][3] = 0.0f;
	result.data[2][3] = 0.0f;
	result.data[3][3] = 1.0f;

    return result;
}

Quaternion quaternion_product(const Quaternion* q1, const Quaternion* q2)
{
    Quaternion res;

    res.w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;
    res.x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
    res.y = q1->w * q2->y + q1->y * q2->w + q1->z * q2->x - q1->x * q2->z;
    res.z = q1->w * q2->z + q1->z * q2->w + q1->x * q2->y - q1->y * q2->x;

    return res;
}

Quaternion quaternion_normalize(const Quaternion* q)
{
	r32 len = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
	return 
	#if !defined(__cplusplus)
	(Quaternion)
	#endif
	{q->x / len, q->y / len, q->z / len, q->w / len};
}

Quaternion quaternion_slerp(const Quaternion* _q1, const Quaternion* _q2, r32 t)
{
    Quaternion q1 = *_q1, q2 = *_q2, qm;

	// Calculate angle between them.
	r32 cos_half_theta = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
    if (cos_half_theta < 0)
    {
        q2.w = -q2.w;
        q2.x = -q2.x;
        q2.y = -q2.y;
        q2.z = q2.z;
        cos_half_theta = -cos_half_theta;
    }

	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (fabsf(cos_half_theta) >= 1.0)
    {
		qm.w = q1.w;
        qm.x = q1.x;
        qm.y = q1.y;
        qm.z = q1.z;
		return qm;
	}

	// Calculate temporary values.
	r32 half_theta = acosf(cos_half_theta);
	r32 sin_half_theta = sqrtf(1.0f - cos_half_theta*cos_half_theta);

	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	if (fabsf(sin_half_theta) < 0.001)
    {
		qm.w = (q1.w * 0.5f + q2.w * 0.5f);
		qm.x = (q1.x * 0.5f + q2.x * 0.5f);
		qm.y = (q1.y * 0.5f + q2.y * 0.5f);
		qm.z = (q1.z * 0.5f + q2.z * 0.5f);
		return qm;
	}

	r32 ratio_a = sinf((1 - t) * half_theta) / sin_half_theta;
	r32 ratio_b = sinf(t * half_theta) / sin_half_theta; 

	// Calculate Quaternion
	qm.w = (q1.w * ratio_a + q2.w * ratio_b);
	qm.x = (q1.x * ratio_a + q2.x * ratio_b);
	qm.y = (q1.y * ratio_a + q2.y * ratio_b);
	qm.z = (q1.z * ratio_a + q2.z * ratio_b);
	return qm;
}

Quaternion quaternion_nlerp(const Quaternion* _q1, const Quaternion* _q2, r32 t)
{
    Quaternion q1 = *_q1, q2 = *_q2, qm;
	Quaternion result;
	r32 dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
	r32 blendI = 1.0f - t;
	if (dot < 0) {
		result.w = blendI * q1.w + t * -q2.w;
		result.x = blendI * q1.x + t * -q2.x;
		result.y = blendI * q1.y + t * -q2.y;
		result.z = blendI * q1.z + t * -q2.z;
	} else {
		result.w = blendI * q1.w + t * q2.w;
		result.x = blendI * q1.x + t * q2.x;
		result.y = blendI * q1.y + t * q2.y;
		result.z = blendI * q1.z + t * q2.z;
	}

	return quaternion_normalize(&result);
}

// This function will only work properly if:
// the matrix is orthogonal
// the matrix is special orthogonal which gives additional condition: det(matrix)= +1
//
// when dealing with transform matrices, usually this should be true unless there is a scale
// (not sure if any scale or only for non-symetrical scales)
Quaternion quaternion_from_matrix(const mat4* m)
{
	Quaternion q = {0};
	mat4 a = *m;

	r32 trace = a.data[0][0] + a.data[1][1] + a.data[2][2]; // I removed + 1.0f; see discussion with Ethan

	if (trace > 0) { // I changed M_EPSILON to 0
		float s = 0.5f / sqrtf(trace+ 1.0f);
		q.w = 0.25f / s;
		q.x = ( a.data[2][1] - a.data[1][2] ) * s;
		q.y = ( a.data[0][2] - a.data[2][0] ) * s;
		q.z = ( a.data[1][0] - a.data[0][1] ) * s;
	} else {
		if ( a.data[0][0] > a.data[1][1] && a.data[0][0] > a.data[2][2] ) {
		float s = 2.0f * sqrtf( 1.0f + a.data[0][0] - a.data[1][1] - a.data[2][2]);
		q.w = (a.data[2][1] - a.data[1][2] ) / s;
		q.x = 0.25f * s;
		q.y = (a.data[0][1] + a.data[1][0] ) / s;
		q.z = (a.data[0][2] + a.data[2][0] ) / s;
		} else if (a.data[1][1] > a.data[2][2]) {
		float s = 2.0f * sqrtf( 1.0f + a.data[1][1] - a.data[0][0] - a.data[2][2]);
		q.w = (a.data[0][2] - a.data[2][0] ) / s;
		q.x = (a.data[0][1] + a.data[1][0] ) / s;
		q.y = 0.25f * s;
		q.z = (a.data[1][2] + a.data[2][1] ) / s;
		} else {
		float s = 2.0f * sqrtf( 1.0f + a.data[2][2] - a.data[0][0] - a.data[1][1] );
		q.w = (a.data[1][0] - a.data[0][1] ) / s;
		q.x = (a.data[0][2] + a.data[2][0] ) / s;
		q.y = (a.data[1][2] + a.data[2][1] ) / s;
		q.z = 0.25f * s;
		}
	}

	return q;
}