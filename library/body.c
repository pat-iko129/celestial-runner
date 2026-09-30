#include "body.h"
#include "assert.h"
#include "color.h"
#include "list.h"
#include "math.h"
#include "vector.h"

#include "polygon.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const size_t INITIAL_SIZE = 10;

typedef struct body {
  list_t *shape;
  double mass;
  double abs_angle;
  double health;
  rgb_color_t color;

  vector_t velocity;
  vector_t centroid;
  vector_t force;
  vector_t impulse;

  void *info;
  free_func_t info_freer;
  bool remove;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  body_t *body = malloc(sizeof(body_t));

  body->shape = shape;
  body->mass = mass;
  body->color = color;
  body->abs_angle = 0;
  body->centroid = polygon_centroid(body->shape);
  body->velocity = VEC_ZERO; // initially at rest
  body->impulse = VEC_ZERO;
  body->force = VEC_ZERO;
  body->info = NULL;
  body->info_freer = NULL;
  body->remove = false;
  return body;
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            double health, void *info, free_func_t info_freer) {
  body_t *body = malloc(sizeof(body_t));
  body->shape = shape;
  body->mass = mass;
  body->color = color;
  body->abs_angle = 0;
  body->centroid = polygon_centroid(body->shape);
  body->velocity = VEC_ZERO; // initially at rest
  body->impulse = VEC_ZERO;
  body->force = VEC_ZERO;
  body->health = health;
  body->info = info;
  body->info_freer = info_freer;
  body->remove = false;
  return body;
}

void body_free(body_t *body) {
  list_free(body->shape);
  if (body->info != NULL && body->info_freer != NULL) {
    body->info_freer(body->info);
  }

  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *ret = list_init(list_size(body->shape), free);
  for (size_t i = 0; i < list_size(body->shape); i++) {
    list_add(ret, copy(list_get(body->shape, i)));
  }
  return ret;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

double body_get_rotation(body_t *body) { return body->abs_angle; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

void body_set_color(body_t *body, rgb_color_t color) { body->color = color; }

void body_set_centroid(body_t *body, vector_t x) {
  vector_t displacement = vec_subtract(x, body->centroid);
  polygon_translate(body->shape, displacement);
  body->centroid = x;
}

void body_constrain_centroid(body_t *body, vector_t min, vector_t max) {
  body_set_centroid(body, clamped(body_get_centroid(body), min, max));
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_scale(body_t *body, double s) {
  vector_t p = body_get_centroid(body);
  polygon_scale(body->shape, s);
  vector_t n = polygon_centroid(body->shape);
  polygon_translate(body->shape, vec_subtract(p, n));
}

void body_point_to(body_t *body, vector_t thing) {
  vector_t to = vec_subtract(body_get_centroid(body), thing);
  body_set_rotation(body, M_PI - angle_with_y(to));
}

void body_set_heading(body_t *body, vector_t heading) {
  body_point_to(body, vec_add(body_get_centroid(body), heading));
}

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->shape, -(body->abs_angle),
                 body->centroid); // rotate back to origin
  polygon_rotate(body->shape, angle, body->centroid);
  body->abs_angle = angle;
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_tick(body_t *body, double dt) {
  vector_t old_v = body_get_velocity(body);
  vector_t new_v =
      vec_add(vec_multiply(dt / body_get_mass(body), body->force), old_v);
  new_v = vec_add(vec_multiply(1 / body_get_mass(body), body->impulse), new_v);
  vector_t avg_v = vec_multiply(0.5, vec_add(old_v, new_v));

  body_set_velocity(body, new_v);
  vector_t displacement = vec_multiply(dt, avg_v);
  body_set_centroid(body, vec_add(body->centroid, displacement));
  body->impulse = VEC_ZERO;
  body->force = VEC_ZERO;
}

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

vector_t body_get_force(body_t *body) { return body->force; }

void body_set_force(body_t *body, vector_t f) { body->force = f; }

double body_get_mass(body_t *body) { return body->mass; }

void body_translate(body_t *body, vector_t translation) {
  body_set_centroid(body, vec_add(body_get_centroid(body), translation));
}

void body_set_health(body_t *body, double amt) { body->health = amt; }

double body_get_health(body_t *body) { return body->health; }

void *body_get_info(body_t *body) { return body->info; }

bool body_is_removed(body_t *body) { return body->remove; }

void body_remove(body_t *body) { body->remove = true; }