#include "list.h"
#include "test_util.h"
#include "vector.h"
#include <assert.h>
#include <stdlib.h>

void test_list_size0() {
  list_t *l = list_init(0, free);
  assert(list_size(l) == 0);
  list_free(l);
}
void add_test() {
  list_t *l = list_init(1, free);
  vector_t *test1 = malloc(sizeof(vector_t));
  list_add(l, test1);
  assert(list_size(l) == 1);
  // free(test1);
  list_free(l);
}

void add_stress_test() {
  list_t *l = list_init(1, free);
  int count = 0;
  for (int i = 0; i < 900; i++) {
    char *v = malloc(sizeof(char));
    *v = 'a';
    list_add(l, v);
    count++;
    assert(list_size(l) == count);
    assert(*(char *)list_get(l, i) == 'a');
    // free(v);
  }
  list_free(l);
}

void remove_test() {
  list_t *l = list_init(1, free);
  char *add = malloc(sizeof(char) * 2);
  *add = "ab";
  list_add(l, add);
  void *val = list_remove(l, list_size(l) - 1);
  assert(*(char *)val == *add);

  assert(list_size(l) == 0);
  list_free(l);
  free(add);
}

void add_pop_test() {
  list_t *l = list_init(1, free);
  int count = 0;
  for (int i = 0; i < 900; i++) {
    char *v = malloc(sizeof(char));
    *v = (char)i;
    list_add(l, v);
    count++;
    assert(list_size(l) == count);
    assert(*(char *)list_get(l, i) == (char)i);
  }
  for (int i = 899; i >= 0; i--) {
    void *val = list_remove(l, list_size(l) - 1);
    assert((char)i == *(char *)val);
    free(val);
  }
  list_free(l);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-linearguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(remove_test)
  DO_TEST(add_stress_test)
  DO_TEST(test_list_size0)
  DO_TEST(add_test)
  DO_TEST(add_pop_test)
  printf("hello mr bruh. This is list test!\n");
}