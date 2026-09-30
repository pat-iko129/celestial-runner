#include "scene.h"
#include "body.h"
#include "list.h"
#include "sdl_wrapper.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

size_t const INITIAL_NUM_BODIES = 20;

typedef struct scene {
  // list_t handles resizing
  list_t *bodies_list;

  // forces list holds a list of force_and_aux objects.
  list_t *forces_list;
} scene_t;

/**
 * @brief Holds a function pointer force
 * and parameters aux. In forces.c,
 * aux is defined as another struct.
 * Therefore, you can apply its force like so:
 * imagine f is a force_and_aux_t. Now, to apply this force we do
 *
 * f->force(f->aux)
 *
 * ^this is calling the function pointed to by force->force with
 * f->aux as its parameters
 */
typedef struct force_and_aux {
  force_creator_t force;
  void *aux;
  free_func_t freer;
  list_t *bodies;
} force_and_aux_t;

void force_and_aux_free(force_and_aux_t *pair) {
  if (pair->freer != NULL) {
    pair->freer(pair->aux);
  }

  while (list_size(pair->bodies) > 0) {
    list_remove(pair->bodies, 0);
  }
  list_free(pair->bodies);
  free(pair);
}

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  scene->bodies_list = list_init(INITIAL_NUM_BODIES, (free_func_t)body_free);
  scene->forces_list =
      list_init(INITIAL_NUM_BODIES, (free_func_t)force_and_aux_free);
  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->forces_list);
  list_free(scene->bodies_list);
  free(scene);
}

size_t scene_bodies(scene_t *scene) {
  size_t body_count = list_size(scene->bodies_list);
  return body_count;
}

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies_list, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies_list, body);
}

void scene_remove_force(scene_t *scene, size_t index) {
  force_and_aux_free(list_remove(scene->forces_list, index));
}

/**
 * @deprecated do not use, use body_remove()
 */
void scene_remove_body(scene_t *scene, size_t index) {
  // body_free(list_remove(scene->bodies_list, index));

  body_remove(scene_get_body(scene, index));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  for (size_t i = 0; i < list_size(bodies); i++) {
    if (body_is_removed(list_get(bodies, i))) {
      return;
    }
  }
  force_and_aux_t *link = malloc(sizeof(force_and_aux_t));
  link->force = forcer;
  link->aux = aux;
  link->freer = freer;
  link->bodies = bodies;
  list_add(scene->forces_list, link);
}

/**
 * @deprecated do not use, use scene_add_bodies_force_creator
 */
void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  // force_and_aux_t *link = malloc(sizeof(force_and_aux_t));
  // link->force = forcer;
  // link->aux = aux;
  // link->freer = freer;
  // list_add(scene->forces_list, link);
  list_t *bodies = list_init(0, (free_func_t)body_free);
  scene_add_bodies_force_creator(scene, forcer, aux, bodies, freer);
}

void scene_apply_forces(scene_t *scene) {
  // applies every force contained in the list of forces
  for (size_t i = 0; i < list_size(scene->forces_list); i++) {
    force_and_aux_t *f = list_get(scene->forces_list, i);
    f->force(f->aux);
  }
}

void scene_tick(scene_t *scene, double dt) {
  // updates scene, sets forces equal to zero
  scene_apply_forces(scene);
  for (size_t i = 0; i < list_size(scene->bodies_list); i++) {
    body_t *bod = scene_get_body(scene, i);
    body_tick(bod, dt);
    vector_t v = body_get_velocity(bod);
    if (vec_mag(v) > 0) {
      body_set_heading(bod, v);
    }
    body_set_force(bod, VEC_ZERO);
  }

  // removal of forces associated with flagged bodies
  for (int i = list_size(scene->forces_list) - 1; (int)i >= 0; i--) {
    force_and_aux_t *curr = list_get(scene->forces_list, i);
    bool remove = false;
    for (size_t j = 0; j < list_size(curr->bodies); j++) {
      if (body_is_removed(list_get(curr->bodies, j))) {
        remove = true;
      }
    }
    if (remove) {
      scene_remove_force(scene, i);
    }
  }

  // removal of flagged bodies
  for (size_t i = scene_bodies(scene) - 1; (int)i >= 0; i--) {
    if (body_is_removed(scene_get_body(scene, i))) {
      body_free(list_remove(scene->bodies_list, i));
    }
  }
}

void scene_draw(scene_t *scene) {
  // draws every body in the scene
  for (int i = 0; i < list_size(scene->bodies_list); i++) {
    body_t *curr = scene_get_body(scene, i);
    sdl_draw_polygon(body_get_shape(curr), body_get_color(curr));
  }
}