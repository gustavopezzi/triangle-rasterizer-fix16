#include <math.h>
#include "vec2.h"

vec2_t vec2_new(fix16_t x, fix16_t y) {
  vec2_t result = { x, y };
  return result;
}

vec2_t vec2_add(vec2_t* a, vec2_t* b) {
  vec2_t result = {
    .x = fix16_add(a->x, b->x),
    .y = fix16_add(a->y, b->y)
  };
  return result;
}

vec2_t vec2_sub(vec2_t* a, vec2_t* b) {
  vec2_t result = {
    .x = fix16_sub(a->x, b->x),
    .y = fix16_sub(a->y, b->y)
  };
  return result;
}

vec2_t vec2_mul(vec2_t* v, fix16_t factor) {
  vec2_t result = {
    .x = fix16_mul(v->x, factor),
    .y = fix16_mul(v->y, factor)
  };
  return result;
}

vec2_t vec2_div(vec2_t* v, fix16_t factor) {
  vec2_t result = {
    .x = fix16_div(v->x, factor),
    .y = fix16_div(v->y, factor)
  };
  return result;
}

fix16_t vec2_length(vec2_t* v) {
  return sqrt(v->x * v->x + v->y * v->y);
}

fix16_t vec2_dot(vec2_t* a, vec2_t* b) {
  return fix16_add(fix16_mul(a->x, b->x), fix16_mul(a->y, b->y));
}

fix16_t vec2_cross(vec2_t* a, vec2_t* b) {
  return fix16_sub(fix16_mul(a->x, b->y), fix16_mul(a->y, b->x));
}

void vec2_normalize(vec2_t* v) {
  fix16_t length = vec2_length(v);
  v->x = fix16_div(v->x, length);
  v->y = fix16_div(v->y, length);
}

vec2_t vec2_rotate(vec2_t v, vec2_t center, fix16_t angle) {
  vec2_t rot;
  v.x = fix16_sub(v.x, center.x);
  v.y = fix16_sub(v.y, center.y);
  rot.x = fix16_sub(fix16_mul(v.x, fix16_cos(angle)), fix16_mul(v.y, fix16_sin(angle)));
  rot.y = fix16_add(fix16_mul(v.x, fix16_sin(angle)), fix16_mul(v.y, fix16_cos(angle)));
  rot.x = fix16_add(rot.x, center.x);
  rot.y = fix16_add(rot.y, center.y);
  return rot;
}