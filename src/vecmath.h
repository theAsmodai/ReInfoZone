#pragma once

bool vectorCompare(const vec_t *v1, const vec_t *v2);
float getAngleDiff(float a, float b);
float length(Vector& v);
float length2D(const vec_t* v);
float dotProduct(Vector& v1, Vector& v2);
void crossProduct(const vec_t *v1, const vec_t *v2, vec_t *cross);
void vectorAnglesFixed(const float* forward, float* angles);

float getViewAngleToOrigin(entvars_t* viewer, Vector origin);
bool vec3InBox(__m128 vec, __m128 lcorner, __m128 hcorner);
float vec3DistToBox(__m128 vec, __m128 lcorner, __m128 hcorner);
Vector getIntersection(Vector start, Vector end, Vector p1, Vector p2, Vector p3);
void getViewIntersection(edict_t* viewer, Vector lcorner, Vector hcorner);
