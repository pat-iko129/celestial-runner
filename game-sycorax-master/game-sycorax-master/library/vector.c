#include "vector.h"
#include "color.h"
#include "list.h"
#include "math.h"
#include <stdlib.h>

const vector_t VEC_ZERO = {0.0, 0.0};
const vector_t X_HAT = {1.0, 0.0};
const vector_t Y_HAT = {0.0, 1.0};
const int RANDOM_RES = 200;

float float_from_0_to_1() {
  float out = (float)(rand() % RANDOM_RES) / RANDOM_RES;
  return out;
}

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t sum;
  sum.x = v1.x + v2.x;
  sum.y = v1.y + v2.y;
  return sum;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t diff;
  diff.x = v1.x - v2.x;
  diff.y = v1.y - v2.y;
  return diff;
}

vector_t vec_negate(vector_t v) {
  vector_t neg;
  neg.x = -1 * v.x;
  neg.y = -1 * v.y;
  return neg;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t v_new;
  v_new.x = v.x * scalar;
  v_new.y = v.y * scalar;
  return v_new;
}

double vec_dot(vector_t v1, vector_t v2) {
  double new_x = v1.x * v2.x;
  double new_y = v1.y * v2.y;
  return new_x + new_y;
}

double vec_cross(vector_t v1, vector_t v2) { return v1.x * v2.y - v2.x * v1.y; }

vector_t vec_rotate(vector_t v, double angle) {
  vector_t rotated;
  rotated.x = v.x * cos(angle) - v.y * sin(angle);
  rotated.y = v.x * sin(angle) + v.y * cos(angle);
  return rotated;
}

double vec_mag(vector_t v) { return sqrt(vec_dot(v, v)); }

vector_t *copy(vector_t *in) {
  vector_t *out = malloc(sizeof(vector_t));
  *out = (vector_t){in->x, in->y};
  return out;
}

vector_t *vec_rand(vector_t min, vector_t max) {
  vector_t *out = malloc(sizeof(vector_t));
  *out = vec_add(min, (vector_t){float_from_0_to_1() * (max.x - min.x),
                                 float_from_0_to_1() * (max.y - min.y)});
  return out;
}

vector_t unit_vec(vector_t vec1, vector_t vec2) { // no longer pointer
  double mag = vec_mag(vec_subtract(vec1, vec2));
  if (mag == 0) {
    return VEC_ZERO; // change
  }
  vector_t sub = vec_subtract(vec1, vec2);
  vector_t val = {sub.x / mag, sub.y / mag};
  return val;
}

vector_t normalize_vec(vector_t vec1) { return unit_vec(vec1, VEC_ZERO); }

vector_t orthonormal_vec(vector_t line) {
  double new_y = -1 * line.x;
  line.x = line.y;
  line.y = new_y;

  line = normalize_vec(line);
  return line;
}

vector_t *make_pointer(vector_t v) {
  vector_t *bruh = malloc(sizeof(vector_t));
  bruh->x = v.x;
  bruh->y = v.y;
  return bruh;
}

/**
 * @brief makes an arrow starting from pos, pointing to force for debugging
 * purposes
 */
list_t *draw_vec(vector_t *pos, vector_t *force) {
  list_t *draw = list_init(3, free);
  vector_t *bruh = malloc(sizeof(vector_t));
  vector_t *dir = malloc(sizeof(vector_t));
  vector_t *bruh2 = malloc(sizeof(vector_t));

  *bruh2 = vec_add(
      *pos, vec_multiply(-2 / vec_mag(*force), vec_rotate(*force, M_PI * 0.5)));
  *bruh = vec_add(
      *pos, vec_multiply(2 / vec_mag(*force), vec_rotate(*force, M_PI * 0.5)));
  *dir = vec_add(*pos, vec_multiply(20. / vec_mag(*force), *force));
  list_add(draw, bruh);
  list_add(draw, bruh2);
  list_add(draw, dir);
  return draw;
}

double angle_with_y(vector_t v) { return atan2(v.x, v.y); }

double dist(vector_t a, vector_t b) { return vec_mag(vec_subtract(a, b)); };

vector_t clamped(vector_t p, vector_t min, vector_t max) {
  return (vector_t){fmax(fmin(p.x, max.x), min.x),
                    fmax(fmin(p.y, max.y), min.y)};
};
