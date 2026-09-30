#include "list.h"
#include "assert.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * A growable array of elements, stored as pointers to malloc()ed elements.
 * A list owns all the vectors in it, so it is responsible for free()ing them.
 * This line does two things:
 * - Declares a "struct list" type
 * - Makes "list_t" an alias for "struct list"
 */
typedef struct list {
  void **data;
  size_t size;
  size_t capacity;
  object_free freer;
} list_t;

list_t *list_init(size_t initial_size, object_free freerFUNC) {
  list_t *list = malloc(sizeof(list_t));
  list->size = 0;
  list->capacity = initial_size;
  list->data = malloc(initial_size * sizeof(void *));
  list->freer = freerFUNC; // object specific free function, if none, just
                           // declare with normal free function
  return list;
}

void list_free(list_t *list) {
  for (size_t i = 0; i < list->size; i++) {
    (list->freer)(list->data[i]); // use object specific free function to free
                                  // within the object
  }
  free(list->data);
  free(list);
}

void list_resize(list_t *list) {
  list->capacity = (list->capacity * 2) + 1;
  list->data = (void **)realloc(list->data, list->capacity * sizeof(void *));
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index < list->size);
  assert(index >= 0);
  void *void_get = (list->data[index]);
  return void_get;
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);
  if (list->size >= list->capacity) {
    list_resize(list);
  }
  list->data[list->size] = value;
  list->size++;
}

/**
 * @brief Removes last index and returns it
 *
 * @param list
 * @return void*
 */
void *list_remove(list_t *list, size_t index) {
  assert(list->size > index);
  void *out = list->data[index];
  for (size_t i = index; i < list->size - 1; i++) {
    list->data[i] = list->data[i + 1];
  }
  list->data[list->size - 1] = NULL;
  list->size--;
  return out;
}

/**
 * @brief Removes first index and returns it
 *
 * @param list
 * @return void*
 */
void *list_pop(list_t *list) {
  assert(list->size > 0);
  void *out = list->data[0];
  for (int i = 0; i < list->size - 1; i++) {
    list->data[i] = list->data[i + 1];
  }
  list->data[list->size - 1] = NULL;
  list->size--;
  return out;
}