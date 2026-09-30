// TODO: IMPLEMENT YOUR TESTS IN THIS FILE
#include "body.h"
#include "forces.h"
#include "scene.h"
#include "test_util.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

list_t *make_shape() {
  list_t *shape = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, +1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-1, +1};
  list_add(shape, v);
  return shape;
}

// tests force
void test_dampened_spring() {
  const double M = 10;
  const double K = 2;
  const double A = 3;
  const double GAMMA = 1.;
  const double DT = 1e-6;
  const int STEPS = 1000000;

  const double omega_0 = sqrt(K / M);
  const double tornado = GAMMA / (2 * M * omega_0);
  const double omega_D = omega_0 * sqrt(1 - (tornado * tornado));

  double t = 0;

  scene_t *scene = scene_init();
  body_t *mass = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass, (vector_t){A, 0});
  scene_add_body(scene, mass);
  body_t *anchor = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  scene_add_body(scene, anchor);
  create_spring(scene, K, mass, anchor);
  create_drag(scene, GAMMA, mass);
  for (int i = 0; i < STEPS; i++) {
    assert(vec_equal(body_get_centroid(anchor), VEC_ZERO));
    assert(vec_isclose(body_get_centroid(mass),
                       (vector_t){exp(-tornado * omega_0 * i * DT) *
                                      (A * cos(omega_D * i * DT) +
                                       ((tornado * A * omega_0) / omega_D) *
                                           sin(omega_D * i * DT)),
                                  0}));

    scene_tick(scene, DT);
    t += DT;
  }
  scene_free(scene);
}

void test_drag() {
  const double GAMMA = 1.0;
  const double M = 10.;
  const double STEPS = 10000;
  const double DT = 1e-6;
  const vector_t START = {500, 250};

  scene_t *scene = scene_init();
  body_t *object = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(object, START);
  body_set_velocity(object, (vector_t){1, 0});
  scene_add_body(scene, object);
  create_drag(scene, GAMMA, object);
  double time_passed = 0;
  double ratio = -GAMMA / M;
  for (size_t i = 0; i < STEPS; i++) {
    assert(
        vec_isclose(body_get_centroid(object),
                    (vector_t){510 + exp(ratio * time_passed) / ratio, 250}));
    scene_tick(scene, DT);
    time_passed += DT;
  }
  scene_free(scene);
}

void test_impulse() {
  const double M = 10;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000;
  scene_t *scene = scene_init();
  body_t *body1 = body_init(make_shape(), M, (rgb_color_t){1, 0, 1});
  body_set_centroid(body1, (vector_t){A, 0});
  scene_add_body(scene, body1);
  body_t *body2 = body_init(make_shape(), M, (rgb_color_t){0, 1, 1});
  scene_add_body(scene, body2);

  for (size_t i = 0; i < STEPS; i++) {
    assert(vec_isclose(body_get_velocity(body1), (vector_t){(double)i / M, 0}));

    body_add_impulse(body1, (vector_t){1, 0});
    scene_tick(scene, DT);
  }

  scene_free(scene);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  // anything conservation, momentum, spring, etc.
  DO_TEST(test_dampened_spring)
  DO_TEST(test_drag)
  DO_TEST(test_impulse)

  puts("student_tests PASS");
}
