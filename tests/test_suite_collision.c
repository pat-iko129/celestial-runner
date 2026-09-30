#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "body.h"
#include "collision.h"
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "test_util.h"
#include "vector.h"

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

void test_find_collision() {
  list_t *shape1 = make_shape();
  list_t *shape2 = make_shape();
  assert(find_collision(shape1, shape2).collided);
  list_free(shape1);
  list_free(shape2);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }
  DO_TEST(test_find_collision);
  puts("collsion_test PASS");
}