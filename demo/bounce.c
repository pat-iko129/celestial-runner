#include "body.h"
#include "color.h"
#include "list.h"
#include "math.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <stdbool.h>

// constants
const double PI = 3.1415926;
const vector_t START = {.x = 500, .y = 250};
const vector_t WINDOW = {.x = 1000, .y = 500};

// star specifics
const int NUMBER_POINTS = 10;
const int BOUNCE_STAR_SIZE = 25;
const double BOUNCE_RADIUS = 50.0;

// velocity
// make velocity into vector_t
const double VELOCITYX = 3.00;
const double VELOCITYY = 2.00;
const double OMEGA = 1; // radial velocity

/**
 * Stores the demo state
 * Use this to store any variable needed every 'tick' of your demo
 */
typedef struct state {
  // state should only hold things that are changing per state
  // take out radius and omega because they are constant throughout
  double radius;
  double omega;
  double total_time;
  vector_t velocity;
  vector_t center;
  list_t *star; // needs 5 outer and 5 inner
  // do we need star_size?? probably a constant so no?
} state_t;

list_t *mk_star(state_t *state) {
  // take out state as a parameter, pass in number_points
  vector_t curr = {0, state->radius};
  vector_t out = {0, BOUNCE_STAR_SIZE};
  vector_t in = vec_negate(out);
  list_t *star = list_init(NUMBER_POINTS, free);

  for (size_t i = 0; i < NUMBER_POINTS; i++) {
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
    curr = vec_rotate(curr, (2 * PI) / NUMBER_POINTS);
    out = vec_rotate(out, (2 * PI) / NUMBER_POINTS);
    in = vec_rotate(in, (2 * PI) / NUMBER_POINTS);
  }
  // use centroid here
  vector_t center = state->center;
  polygon_translate(star, center);
  return star;
}

// write header comments for functions that we made ourselves that don't have
// specs in header files
void polygon_bounce(state_t *state) {
  for (size_t i = 0; i < list_size(state->star); i++) {
    vector_t side = *(vector_t *)list_get(state->star, i);
    if (side.y > WINDOW.y || side.y < 0) {
      double v_y = state->velocity.y;
      state->velocity.y = -v_y;
    }
    if (side.x > WINDOW.x || side.x < 0) {
      double v_x = state->velocity.x;
      state->velocity.x = -v_x;
    }
  }
}

/**
 * Initializes sdl as well as the variables needed
 * Creates and stores all necessary variables for the demo in a created state
 * variable Returns the pointer to this state (This is the state emscripten_main
 * and emscripten_free work with)
 */
state_t *emscripten_init() {
  // only happens once, sets up frames for everything to start going
  // sdl is the one that's updating every single frame
  // once sdl ends, emscripten realizes its the end and frees all variables
  state_t *state = malloc(sizeof(state_t));
  vector_t min = {0, 0};
  sdl_init(min, WINDOW);

  state->radius = BOUNCE_RADIUS;
  state->omega = OMEGA;

  state->total_time = 0.00;
  state->velocity.x = VELOCITYX;
  state->velocity.y = VELOCITYY;
  state->center = START;

  state->star = mk_star(state);

  return state;
}

/**
 * Called on each tick of the program
 * Updates the state variables and display as necessary, depending on the time
 * that has passed
 */
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->total_time = state->total_time + dt;
  // double new_time = total_time + dt;

  // drawing
  sdl_draw_polygon(state->star, (rgb_color_t){1, 0, 0});
  polygon_rotate(state->star, state->omega * dt, polygon_centroid(state->star));
  polygon_translate(state->star, state->velocity);

  polygon_bounce(state);
  // state->total_time = new_time;
  sdl_show();
}

/**
 * Frees anything allocated in the demo
 * Should free everything in state as well as state itself.
 */
void emscripten_free(state_t *state) {
  list_t *star = state->star;
  list_free(star);
  free(state);
}
