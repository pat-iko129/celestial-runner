#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "level.h"
#include "list.h"
#include "math.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <stdbool.h>

#include "SDL_mixer.h"
#include "SDL_thread.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

// Window constants
const vector_t WINDOW_SIZE = {.x = 1000, .y = 500};

// Misc
const double R = 7;
const double M = 10;
const int N = 50;
const double G = 10000.;
const double ELASTICITY = .15;
const double GAMMA = 0.9;

typedef struct state {
  double total_time;
  scene_t *scene;
} state_t;

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  sdl_init(vec_multiply(-0.5, WINDOW_SIZE), vec_multiply(0.5, WINDOW_SIZE));
  state->scene = scene_init();
  state->total_time = 0.;
  for (int i = 0; i < N; i++) {
    list_t *shape = make_circle(R);
    body_t *curr = body_init_with_info(shape, M, RED, 0.1, NULL, free);
    vector_t *pos = vec_rand(vec_multiply(-0.5, WINDOW_SIZE),
                             vec_multiply(0.5, WINDOW_SIZE));
    body_set_centroid(curr, *pos);
    free(pos);
    scene_add_body(state->scene, curr);
  }
  for (int i = 0; i < N; i++) {
    body_t *curr = scene_get_body(state->scene, (size_t)i);
    create_drag(state->scene, GAMMA, curr);

    for (int j = 0; j < N; j++) {
      if (i != j) {
        body_t *bro = scene_get_body(state->scene, (size_t)j);
        create_newtonian_gravity(state->scene, G, curr, bro);
        create_collision(state->scene, curr, bro, highlight, NULL, NULL);
        create_physics_collision(state->scene, ELASTICITY, curr, bro);
      }
    }
  }
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();

  scene_draw(state->scene);
  scene_tick(state->scene, dt);
  sdl_show();
}

void emscripten_free(state_t *state) { scene_free(state->scene); }
