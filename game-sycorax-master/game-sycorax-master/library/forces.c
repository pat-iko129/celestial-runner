#include "forces.h"
#include "scene.h"

#include "body.h"
#include "vector.h"

#include "collision.h"
#include "math.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const double MIN_DIST_GRAV = 5;
const size_t BODIES_LIST_SIZE = 2;

typedef struct aux {
  double constant;
  body_t *body1;
  body_t *body2;
  free_func_t freer;
} aux_t;

typedef struct handler_aux {
  void *aux;
  collision_handler_t handler;
  list_t *bodies;
  bool prev_collided;
  bool collided;
} handler_aux_t;

aux_t *aux_init(double constant, body_t *body1, body_t *body2) {
  aux_t *fc = malloc(sizeof(aux_t));
  fc->body1 = body1;
  fc->body2 = body2;
  fc->constant = constant;
  return fc;
}

handler_aux_t *handler_aux_init(void *aux, collision_handler_t handler,
                                list_t *bodies, bool prev_collided,
                                bool collided) {
  handler_aux_t *out = malloc(sizeof(handler_aux_t));
  out->aux = aux;
  out->handler = handler;
  out->bodies = bodies;
  out->prev_collided = prev_collided;
  out->collided = collided;
  return out;
}

void aux_free(aux_t *aux) { free(aux); }

void handler_aux_free(handler_aux_t *handler_aux) {
  aux_free(handler_aux->aux);
  free(handler_aux);
}

void follow(void *in) {
  aux_t *input = (aux_t *)in;
  vector_t b1 = body_get_centroid(input->body1);
  vector_t b2 = body_get_centroid(input->body2);

  vector_t direction = vec_multiply(input->constant, unit_vec(b2, b1));

  body_set_velocity(input->body1,
                    vec_add(body_get_velocity(input->body1), direction));
  // body_set_velocity(input->body1, vec_multiply(0.9,
  // body_get_velocity(input->body1)));
}

/**
 * @brief force_creator_t for gravity
 */
void gravity(void *in) {
  aux_t *input = (aux_t *)in;
  double dist = vec_mag(vec_subtract(body_get_centroid(input->body1),
                                     body_get_centroid(input->body2)));
  double force_mag;
  vector_t unit = unit_vec(body_get_centroid(input->body1),
                           body_get_centroid(input->body2));
  vector_t grav_vec;
  force_mag = input->constant *
              (body_get_mass(input->body1) * body_get_mass(input->body2)) /
              pow(dist, 2);
  grav_vec = vec_multiply(force_mag, unit);

  if (dist > MIN_DIST_GRAV) {
    body_add_force((input)->body2, grav_vec);
    body_add_force((input)->body1, vec_negate(grav_vec));
  }
}

/**
 * @brief force_creator_t for spring
 */
void spring(void *in) {
  aux_t *input = (aux_t *)in;

  vector_t spring = vec_multiply(
      input->constant, (vec_subtract(body_get_centroid(input->body1),
                                     body_get_centroid(input->body2))));
  body_add_force(input->body2, spring);
  body_add_force(input->body1, vec_negate(spring));
}
void drag(void *in) {
  aux_t *input = (aux_t *)in;
  vector_t newV =
      vec_multiply(-1 * input->constant, body_get_velocity(input->body1));
  body_add_force(input->body1, newV);
}

void destructive_collision(body_t *body1, body_t *body2, vector_t axis,
                           void *aux) {
  // destroys both bodies, both conditionals should be true
  body_remove(body1);
  body_remove(body2);
}

void add_impulse(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  // get info from bodies -> shape and mass
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);

  // calculate impulse
  double u_a = vec_dot(body_get_velocity(body1), axis);
  double u_b = vec_dot(body_get_velocity(body2), axis);
  double reduced_mass = (mass1 * mass2) / (mass1 + mass2);
  if (mass1 == INFINITY) {
    reduced_mass = mass2;
  } else if (mass2 == INFINITY) {
    reduced_mass = mass1;
  }

  // add impulse in direction of axis (unit vector)
  double impulse = reduced_mass * (1 + ((aux_t *)aux)->constant) * (u_b - u_a);
  vector_t impulse_vec = vec_multiply(impulse, axis);
  body_add_impulse(body1, impulse_vec);
  body_add_impulse(body2, vec_multiply(-1, impulse_vec));
}

/**
 * @brief makes body1 follow body2
 */
void create_follow(scene_t *scene, double force, double drag, body_t *body1,
                   body_t *body2) {
  list_t *bodies = list_init(BODIES_LIST_SIZE, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(force, body1, body2);
  create_drag(scene, drag, body1);
  scene_add_bodies_force_creator(scene, (force_creator_t)follow, aux, bodies,
                                 (free_func_t)aux_free);
}

/**
 * @brief Adds gravity force between two bodies in scene
 */
void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  // aux is a struct that holds all the information needed to do gravity on
  // stuff
  // (in this case its our 2 bodies and the gravitational constant)
  list_t *bodies = list_init(BODIES_LIST_SIZE, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(G, body1, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)gravity, aux, bodies,
                                 (free_func_t)aux_free);
}

/**
 * @brief Adds spring force between two bodies in scene
 */
void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  list_t *bodies = list_init(BODIES_LIST_SIZE, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(k, body1, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)spring, aux, bodies,
                                 (free_func_t)aux_free);
}

/**
 * @brief Adds drag force onto a body in scene
 */
void create_drag(scene_t *scene, double gamma, body_t *body) {
  list_t *bodies = list_init(BODIES_LIST_SIZE - 1, (free_func_t)body_free);
  list_add(bodies, body);
  aux_t *aux = aux_init(gamma, body, NULL);
  scene_add_bodies_force_creator(scene, (force_creator_t)drag, aux, bodies,
                                 (free_func_t)aux_free);
}

void calculate_collision(handler_aux_t *handler_aux) {
  list_t *body_list_1 = body_get_shape(list_get(handler_aux->bodies, 0));
  list_t *body_list_2 = body_get_shape(list_get(handler_aux->bodies, 1));
  collision_info_t col = find_collision(body_list_1, body_list_2);
  list_free(body_list_1);
  list_free(body_list_2);

  handler_aux->collided = col.collided;
  if (!handler_aux->prev_collided && handler_aux->collided) {
    handler_aux->handler(list_get(handler_aux->bodies, 0),
                         list_get(handler_aux->bodies, 1), col.axis,
                         handler_aux->aux);
  }
  if (handler_aux->prev_collided == true && col.collided == false) {
    handler_aux->prev_collided = false;
  } else if (handler_aux->prev_collided == false && col.collided == true) {
    handler_aux->prev_collided = true;
  }
  // handler_aux->prev_collided = col.collided;
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  list_t *bodies = list_init(BODIES_LIST_SIZE - 1, (free_func_t)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  handler_aux_t *handler_aux =
      handler_aux_init(aux, handler, bodies, false, false);
  scene_add_bodies_force_creator(scene, (force_creator_t)calculate_collision,
                                 handler_aux, bodies, freer);
  //(free_func_t)handler_aux_free);
}

/**
 * Adds a force creator to a scene that destroys two bodies when they collide.
 * The bodies should be destroyed by calling body_remove().
 */
void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {

  aux_t *aux = aux_init(0.0, body1, body2);
  create_collision(scene, body1, body2, destructive_collision, aux,
                   (free_func_t)aux_free);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  aux_t *aux = aux_init(elasticity, body1, body2);
  create_collision(scene, body1, body2, add_impulse, aux,
                   (free_func_t)aux_free);
}