#include "polygon.h"
#include "assert.h"
#include "list.h"
#include "math.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const int CIRCLE_DEF =
    60; // amount of points you need to make a circle from a makestar
const double TWO_PI = 2 * M_PI;

/**
 * @brief Returns list of vector_t's in the shape of a star
 *
 * @param num_points num_points-pointed star
 * @param center the coordinates of the center of the star
 * @param radius radius of inner polygon
 * @param pointy how far do the points of the star extend past "radius"
 * @return list_t*
 */
list_t *make_star(int num_points, vector_t center, double radius,
                  double pointy) {
  num_points = num_points * 2;

  vector_t curr = {0, radius};
  vector_t out = {0, pointy};
  vector_t in = vec_negate(out);
  list_t *star = list_init(num_points, free);

  for (int i = 0; i < num_points; i++) {
    vector_t *curr_pointer = malloc(sizeof(vector_t));
    if (i % 2 == 0) {
      curr = vec_add(curr, out);
      curr_pointer->x = curr.x;
      curr_pointer->y = curr.y;
    } else {
      curr = vec_add(curr, in);
      curr_pointer->x = curr.x;
      curr_pointer->y = curr.y;
    }
    list_add(star, curr_pointer);
    curr = vec_rotate(curr, (2 * M_PI) / num_points);
    out = vec_rotate(out, (2 * M_PI) / num_points);
    in = vec_rotate(in, (2 * M_PI) / num_points);
  }
  polygon_translate(star, center);
  return star;
}

double polygon_area(list_t *polygon) {
  double area = 0.0;
  int poly_size = list_size(polygon);

  for (int i = 1; i <= poly_size; i++) {

    area += ((vector_t *)list_get(polygon, i % poly_size))->y *
            (((vector_t *)list_get(polygon, (i - 1) % poly_size))->x -
             ((vector_t *)list_get(polygon, (i + 1) % poly_size))->x);
  }

  return 0.5 * area;
}

vector_t polygon_centroid(list_t *polygon) {
  double constant = 1 / (6 * (polygon_area(polygon)));
  if (constant == 0.) {
    return *(vector_t *)list_get(polygon, 0);
  };
  double c_x = 0.00;
  double c_y = 0.00;
  // can't mod by a double, so modding by size_t
  size_t poly_size = list_size(polygon);
  for (size_t i = 0; i < poly_size; i++) {
    double x_i = ((vector_t *)list_get(polygon, i))->x;
    double x_i1 = ((vector_t *)list_get(polygon, (i + 1) % poly_size))->x;
    double y_i = ((vector_t *)list_get(polygon, i))->y;
    double y_i1 = ((vector_t *)list_get(polygon, (i + 1) % poly_size))->y;

    c_x += (x_i + x_i1) * (x_i * y_i1 - x_i1 * y_i);
    c_y += (y_i + y_i1) * (x_i * y_i1 - x_i1 * y_i);
  }
  c_x = constant * c_x;
  c_y = constant * c_y;

  vector_t centroid = {c_x, c_y};
  return centroid;
}

void polygon_set_centroid(list_t *polygon, vector_t pos) {
  vector_t displacement = vec_subtract(pos, polygon_centroid(polygon));
  polygon_translate(polygon, displacement);
}

void polygon_translate(list_t *polygon, vector_t translation) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    *(vector_t *)list_get(polygon, i) =
        vec_add(*(vector_t *)list_get(polygon, i), translation);
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  // bring polygon to 0,0 so rotation matrix can work on origin
  vector_t negateP = vec_negate(point);
  polygon_translate(polygon, negateP);
  // do rotation on each vector
  for (size_t i = 0; i < list_size(polygon); i++) {
    *(vector_t *)list_get(polygon, i) =
        vec_rotate(*(vector_t *)list_get(polygon, i), angle);
  }
  // translate back
  polygon_translate(polygon, point);
}

void polygon_scale(list_t *polygon, double s) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *vertex = list_get(polygon, i);
    vertex->x *= s;
    vertex->y *= s;
  }
}

void normalize_shape(list_t *polygon) {
  vector_t c = polygon_centroid(polygon);
  double best_distance = -INFINITY;
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *vertex = list_get(polygon, i);
    if (dist(*vertex, c) > best_distance) {
      best_distance = dist(*vertex, c);
    }
  }
  polygon_scale(polygon, 1 / best_distance);
}

list_t *make_circle(double radius) {
  list_t *out = list_init(CIRCLE_DEF, free);
  double angle_offset =
      TWO_PI / CIRCLE_DEF; // plots a point at every angle offset
  for (int i = 0; i < CIRCLE_DEF; i++) {
    vector_t *vertex = malloc(sizeof(vector_t));
    double curr_angle = i * angle_offset;
    vector_t v = {radius * cos(curr_angle), radius * sin(curr_angle)};
    *vertex = v;
    list_add(out, vertex);
  }
  return out;
}

int step(double val, double edge) {
  if (val > edge) {
    return 1;
  }
  return 0;
}

/**
 * @brief Math stuff for making pac_man
 */
list_t *pac_man_shape(double radius, double mouth_angle) {
  list_t *out = list_init(CIRCLE_DEF, free);
  double a_offset = TWO_PI / CIRCLE_DEF;
  for (int i = 0; i < CIRCLE_DEF; i++) {
    vector_t *vertex = malloc(sizeof(vector_t));
    double curr = i * a_offset;
    vector_t v = {radius * cos(curr), radius * sin(curr)};
    *vertex = v;
    *vertex = vec_multiply(
        step(curr, mouth_angle) * step(TWO_PI - mouth_angle, curr), *vertex);
    list_add(out, vertex);
  }
  return out;
}

/**
 * Makes a spaceship
 */
list_t *make_spaceship(double scale_factor) {
  list_t *spaceship = list_init(10, free);
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){1, 0})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){12, 0})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){12, 1})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){13, 1})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){13, 2})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){11, 2})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){11, 3})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){9, 3})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){9, 4})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 4})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 5})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){7, 5})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){7, 7})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){6, 7})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){6, 5})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){5, 5})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){5, 4})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){4, 4})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){4, 3})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 3})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){2, 3})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){2, 2})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){0, 2})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){0, 1})));
  list_add(spaceship,
           make_pointer(vec_multiply(scale_factor, (vector_t){1, 1})));
  normalize_shape(spaceship);
  polygon_scale(spaceship, scale_factor);
  return spaceship;
}

/*
 * Makes spaceship for celestial runner
 */
list_t *make_runner(double scale_factor) {
  list_t *runner_list = list_init(7, free);
  list_add(runner_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){0, 0})));
  list_add(runner_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 1})));
  list_add(runner_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){5, 1})));
  list_add(runner_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 0})));
  list_add(runner_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){5, 3})));
  list_add(runner_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){4, 2})));
  list_add(runner_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 3})));
  normalize_shape(runner_list);
  polygon_scale(runner_list, scale_factor);
  return runner_list;
}

/*
 * Makes stationary enemy for celestial runner
 */
list_t *make_enemy_stationary(double scale_factor) {
  list_t *enemy = list_init(6, free);
  list_add(enemy, make_pointer(vec_multiply(scale_factor, (vector_t){0, 0})));
  list_add(enemy, make_pointer(vec_multiply(scale_factor, (vector_t){2, 1})));
  list_add(enemy, make_pointer(vec_multiply(scale_factor, (vector_t){3, 3})));
  list_add(enemy, make_pointer(vec_multiply(scale_factor, (vector_t){3, 5})));
  list_add(enemy, make_pointer(vec_multiply(scale_factor, (vector_t){1, 4})));
  list_add(enemy, make_pointer(vec_multiply(scale_factor, (vector_t){0, 2})));
  polygon_rotate(enemy, M_PI / 6, VEC_ZERO);
  normalize_shape(enemy);
  polygon_scale(enemy, scale_factor);
  return enemy;
}

/**
 * Makes a space invader shape
 */
list_t *make_space_bruh(double scale_factor) {
  list_t *space_man_list = list_init(10, free);
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){0, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){1, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){1, 3})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){2, 3})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){2, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 0})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){5, 0})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){5, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 2})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 2})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){6, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){6, 0})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 0})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){9, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){9, 3})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){10, 3})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){10, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){11, 1})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){11, 4})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){10, 4})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){10, 5})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){9, 5})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){9, 6})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 6})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 7})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){9, 7})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){9, 8})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 8})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){8, 7})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){7, 7})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){7, 6})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){4, 6})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){4, 7})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 7})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 8})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){2, 8})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){2, 7})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 7})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){3, 6})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){2, 6})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){2, 5})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){1, 5})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){1, 4})));
  list_add(space_man_list,
           make_pointer(vec_multiply(scale_factor, (vector_t){0, 4})));

  return space_man_list;
}

list_t *make_turret(vector_t center, double scale_factor) {
  list_t *turret = list_init(6, free);
  vector_t start = {center.x - 1.5, center.y};
  list_add(turret, make_pointer(vec_multiply(scale_factor,
                                             (vector_t){start.x, start.y})));
  list_add(turret, make_pointer(vec_multiply(
                       scale_factor, (vector_t){start.x + 1, start.y + 1})));
  list_add(turret,
           make_pointer(vec_multiply(
               scale_factor, (vector_t){start.x + 1.5, start.y + 4}))); // point
  list_add(turret, make_pointer(vec_multiply(
                       scale_factor, (vector_t){start.x + 2, start.y + 1})));
  list_add(turret, make_pointer(vec_multiply(
                       scale_factor, (vector_t){start.x + 3, start.y})));
  list_add(turret,
           make_pointer(vec_multiply(
               scale_factor, (vector_t){start.x + 1.5, start.y - 1.5})));
  normalize_shape(turret);
  polygon_scale(turret, scale_factor);
  return turret;
}

list_t *rectangle(double w, double h) {
  list_t *rec = list_init(4, free);
  list_add(rec, make_pointer((vector_t){w, 0}));
  list_add(rec, make_pointer((vector_t){w, h}));
  list_add(rec, make_pointer((vector_t){0, h}));
  list_add(rec, make_pointer((vector_t){0, 0}));
  return rec;
}

list_t *make_rectangle(double scale_factor) {
  return rectangle(scale_factor * 1, scale_factor * 2);
}
