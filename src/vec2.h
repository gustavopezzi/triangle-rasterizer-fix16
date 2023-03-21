#ifndef VEC2_H
#define VEC2_H

#include "../lib/libfixmath/fix16.h"

typedef struct {
  fix16_t x;
  fix16_t y;
} vec2_t;

vec2_t vec2_new(fix16_t x, fix16_t y);
vec2_t vec2_add(vec2_t* a, vec2_t* b);
vec2_t vec2_sub(vec2_t* a, vec2_t* b);
vec2_t vec2_mul(vec2_t* v, fix16_t factor);
vec2_t vec2_div(vec2_t* v, fix16_t factor);
fix16_t vec2_length(vec2_t* v);
fix16_t vec2_dot(vec2_t* a, vec2_t* b);
fix16_t vec2_cross(vec2_t* a, vec2_t* b);
void vec2_normalize(vec2_t* v);
vec2_t vec2_rotate(vec2_t v, vec2_t center, fix16_t angle);

#endif
