#include "body.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "math.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <stdbool.h>

const int NUMPOINTS = 4;
const int ONSCREEN = 50; // amount of bodies on screen
const double MASS = 10.;
const double RADIUS = 5.;
const vector_t WINDOW = {.x = 1000, .y = 500};
const double GRAVITY = 10000.;
const double SPEED = .01;
const double CIRC_RADIUS = 100;
const int SEED = 4574;
const double DRAG = 0.8;

/**
 * @brief
 * Use this to store any variable needed every 'tick' of your demo
 */
typedef struct state {
  double R_inc;      // color variable
  double total_time; // total time since last star dropped
  scene_t *scene;
} state_t;

/**
 * @brief Initializes sdl as well as the variables needed
 * Creates and stores all necessary variables for the demo in a created state
 * variable Returns the pointer to this state (This is the state emscripten_main
 * and emscripten_free work with)
 *
 * @return state_t*
 */
state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  sdl_init(VEC_ZERO, WINDOW);

  state->total_time = 0.00;
  state->R_inc = 0;
  // initialization includes object specific freer function
  state->scene = scene_init();

  // Controls the randomness
  for (size_t i = 0; i < SEED; i++) {
    float_from_0_to_1();
  }

  for (size_t i = 0; i < ONSCREEN; i++) {
    double val = RADIUS * (1 + float_from_0_to_1());
    list_t *star_shape = make_star(
        NUMPOINTS,
        vec_add(vec_multiply(0.5, WINDOW),
                vec_multiply(CIRC_RADIUS,
                             (vector_t){cos(2 * M_PI * i / ONSCREEN),
                                        sin(2 * M_PI * i / ONSCREEN)})),
        val, val);
    scene_add_body(state->scene,
                   body_init(star_shape, MASS, (rgb_color_t){1, 0, 0}));
    body_set_velocity(
        scene_get_body(state->scene, i),
        vec_multiply(SPEED, *vec_rand(vec_negate(WINDOW), WINDOW)));
    create_drag(state->scene, DRAG, scene_get_body(state->scene, i));
  }

  body_set_velocity(scene_get_body(state->scene, 0), VEC_ZERO);
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    for (size_t j = 0; j < scene_bodies(state->scene); j++) {
      if (i != j) {
        create_newtonian_gravity(state->scene, GRAVITY,
                                 scene_get_body(state->scene, i),
                                 scene_get_body(state->scene, j));
      }
    }
  }

  return state;
}

/**
 * @brief Updates the state variables and display as necessary, depending on the
 * time that has passed
 *
 * @param state
 */
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  scene_draw(state->scene);

  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *bod = scene_get_body(state->scene, i);
    vector_t *cent = malloc(sizeof(vector_t));
    *cent = body_get_centroid(bod);

    vector_t *forc = malloc(sizeof(vector_t));
    *forc = body_get_force(bod);

    vector_t *vel = malloc(sizeof(vector_t));
    *vel = body_get_velocity(bod);
    sdl_draw_polygon(draw_vec(cent, forc), (rgb_color_t){0, 1, 0});

    sdl_draw_polygon(draw_vec(cent, vel), (rgb_color_t){0, 0, 1});

    free(cent);
    free(forc);
    free(vel);
  }
  scene_tick(state->scene, dt);
  sdl_show();
}

/**
 * @brief Frees anything allocated in the demo
 * Should free everything in state as well as state itself (including everything
 * in objects within the state)
 *
 * @param state
 */
void emscripten_free(state_t *state) {
  // polygon_free is being called from within list_free
  scene_free(state->scene);
  free(state);
}