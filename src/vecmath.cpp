#include "precompiled.h"

inline Vector xmm2vec(__m128 xmm)
{
	Vector vec;
	_mm_storel_pi((__m64*)&vec.x, xmm);
	_mm_store_ss(&vec.z, _mm_movehl_ps(xmm, xmm));
	return vec;
}

inline void xmm2vec(__m128 xmm, vec_t* v)
{
	_mm_storel_pi((__m64*)&v, xmm);
	_mm_store_ss(v + 2, _mm_movehl_ps(xmm, xmm));
}

inline __m128 getPart(__m128 xmm, size_t index)
{
	switch (index) {
	default:
	case 0: return xmm;
	case 1: return _mm_shuffle_ps(xmm, xmm, 1);
	case 2: return _mm_movehl_ps(xmm, xmm);
	}
}

inline __m128 setVec3FloatFrom(__m128 to, __m128 from, size_t index)
{
	if (index == 0) {
		return _mm_move_ss(to, from);
	}
	if (index == 2) {
		return _mm_shuffle_ps(to, from, _MM_SHUFFLE(3, 2, 1, 0));
	}

	from = _mm_move_ss(from, to);
	from = _mm_shuffle_ps(from, to, _MM_SHUFFLE(3, 2, 1, 0));
	return from;
}

inline __m128 dotProduct3D(__m128 v1, __m128 v2)
{
	if (cpuinfo.sse4_1)
		return _mm_dp_ps(v1, v2, 0x71);
	__m128 v = _mm_mul_ps(v1, v2);
	return _mm_add_ps(_mm_movehl_ps(v, v), _mm_hadd_ps(v, v)); // SSE3
}

inline __m128 crossProduct3D(__m128 a, __m128 b)
{
	__m128 tmp1 = _mm_mul_ps(a, _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1)));
	__m128 tmp2 = _mm_mul_ps(b, _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)));
	__m128 m = _mm_sub_ps(tmp1, tmp2);

	return _mm_shuffle_ps(m, m, _MM_SHUFFLE(3, 0, 2, 1));
}

inline __m128 length3D(__m128 v)
{
	return _mm_sqrt_ps(dotProduct3D(v, v));
}

inline __m128 length2D(__m128 v)
{
	v = _mm_mul_ps(v, v);
	return _mm_sqrt_ps(_mm_hadd_ps(v, v)); // hadd = SSE3
}

inline __m128 vec3ToPlane(__m128 origin, __m128 a, __m128 b)
{
	__m128 ab = _mm_sub_ps(b, a);
	__m128 dot = dotProduct3D(ab, ab);

	if (_mm_comineq_ss(dot, _mm_setzero_ps())) {
		__m128 tmp = _mm_div_ss(dotProduct3D(ab, _mm_sub_ps(origin, a)), dot);
		return _mm_add_ps(a, _mm_mul_ps(ab, _mm_shuffle_ps(tmp, tmp, 0)));
	}

	return _mm_setzero_ps();
}

NOINLINE __m128 vec3ToBox(__m128 vec, __m128 lcorner, __m128 hcorner)
{
	enum
	{
		X = 1,
		Y = 2,
		Z = 4,
		XYZ = X | Y | Z
	};

	auto lcmp = _mm_cmple_ps(lcorner, vec);
	auto hcmp = _mm_cmpge_ps(hcorner, vec);
	auto acmp = _mm_and_ps(lcmp, hcmp);

	__m128 point, xcmp;

	switch (_mm_movemask_ps(acmp) & XYZ)
	{
	// vec to corner
	case 0:
		return _mm_or_ps(_mm_and_ps(hcmp, lcorner), _mm_and_ps(lcmp, hcorner));

	// vec to plane
	case X:
	case Y:
	case Z:
		lcmp = _mm_xor_ps(lcmp, acmp);
		hcmp = _mm_xor_ps(hcmp, acmp);
		point = _mm_or_ps(_mm_and_ps(lcmp, hcorner), _mm_and_ps(hcmp, lcorner));
		return vec3ToPlane(vec, _mm_or_ps(point, _mm_and_ps(lcorner, acmp)), _mm_or_ps(point, _mm_and_ps(hcorner, acmp))); // TODO: simplify this
	
	// vec to surface
	case X | Y:
	case X | Z:
	case Y | Z:
		xcmp = _mm_xor_ps(lcmp, hcmp);
		point = _mm_and_ps(_mm_or_ps(_mm_and_ps(lcorner, hcmp), _mm_and_ps(hcorner, lcmp)), xcmp);
		return _mm_or_ps(point, _mm_and_ps(vec, _mm_and_ps(lcmp, hcmp)));

	// inside
	default: // X | Y | Z:
		return vec;
	}
}

ALIGN16 struct {int a, b; float c, d;} fl_180 = {0x7fffffff, 0x7fffffff, 180.0, 180.0};

float getViewAngleToOrigin(entvars_t* viewer, Vector origin)
{
	ALIGN16 Vector target_angle;
	vectorAnglesFixed(origin - (viewer->origin + viewer->view_ofs), target_angle);

	__m128 f7f_180 = _mm_load_ps((float *)&fl_180);
	__m128 target = _mm_load_ps(target_angle);
	__m128 v_angle = _mm_loadu_ps(viewer->v_angle);
	__m128 f180 = __m128();
	f180 = _mm_movehl_ps(f180, f7f_180);
	target = _mm_sub_ps(target, v_angle);
	__m128 f180_2 = f180;
	target = _mm_and_ps(target, f7f_180);
	f180 = _mm_sub_ps(f180, target);
	f180 = _mm_and_ps(f180, f7f_180);
	f180_2 = _mm_sub_ps(f180_2, f180);

	f180_2 = _mm_mul_ps(f180_2, f180_2);
	return _mm_cvtss_f32(length2D(f180_2));
}

bool vec3InBox(__m128 vec, __m128 lcorner, __m128 hcorner)
{
	lcorner = _mm_cmple_ps(lcorner, vec);
	hcorner = _mm_cmpge_ps(hcorner, vec);
	lcorner = _mm_xor_ps(lcorner, hcorner);
	return !(_mm_movemask_epi8(_mm_castps_si128(lcorner)) & 0xFFF);
}

float vec3DistToBox(__m128 vec, __m128 lcorner, __m128 hcorner)
{
	__m128 point = vec3ToBox(vec, lcorner, hcorner);
	return _mm_cvtss_f32(length3D(_mm_sub_ps(point, vec)));
}

/* External */

bool vectorCompare(const vec_t *v1, const vec_t *v2)
{
	__m128 cmp = _mm_cmpneq_ps(_mm_loadu_ps(v1), _mm_loadu_ps(v2));
	return !(_mm_movemask_epi8(_mm_castps_si128(cmp)) & 0xFFF);
}

float getAngleDiff(float a, float b)
{
	return 180.0 - abs(180.0 - abs(a - b));
}

float length(Vector& v)
{
	return _mm_cvtss_f32(length3D(_mm_loadu_ps(v)));
}

float length2D(const vec_t * v)
{
	__m128 mult = _mm_mul_ps(_mm_loadu_ps(v), _mm_loadu_ps(v));
	return _mm_cvtss_f32(_mm_sqrt_ps(_mm_hadd_ps(mult, mult))); // hadd = SSE3
}

float dotProduct(Vector& v1, Vector& v2)
{
	return _mm_cvtss_f32(dotProduct3D(_mm_loadu_ps(v1), _mm_loadu_ps(v2)));
}

void crossProduct(const vec_t *v1, const vec_t *v2, vec_t *cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void vectorAnglesFixed(const float* forward, float* angles)
{
	float length, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0) {
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else {
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		length = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], length) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
