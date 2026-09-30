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

#include "state.h"

#include <stdbool.h>

// window constants
const vector_t WINDOW = {.x = 1000, .y = 500};
const vector_t START = {.x = 500, .y = 250};

// circle constants
const double CIRCLE_RADIUS = 15;
const double CIRCLE_MASS = 1;
const vector_t START_CIRCLE_POS = {0, 250};
const vector_t CIRCLE_INCREMENT = {CIRCLE_RADIUS * 2,
                                   0}; // increments by diameter of a circle
const rgb_color_t START_CIRCLE_COLOR = {1, 0, 0};
const rgb_color_t ANCHOR_COLOR = {1, 1, 1};

// physics constants
const double K = 50;
const double SPEED = 5.;
const double GAMMA = 1.;
const double MAGNITUDE = 150.;

// color constants
const double RGB_SHIFT = M_PI / 3.0;
const double COLOR_SHIFT = 0.1;
const double BRIGHTNESS = 1.0;

typedef struct state {

  scene_t *spring_scene;

  // color variables
  double R_inc;
  double curr_color;

} state_t;

body_t *make_spring_circle(vector_t pos, rgb_color_t color, double mass) {
  list_t *circ_list = make_circle(CIRCLE_RADIUS);
  body_t *circle = body_init(circ_list, mass, color); // creats circle as a body
  body_set_centroid(circle, pos); // move circle body to given position
  return circle;
}

state_t *emscripten_init() {

  state_t *state = malloc(sizeof(state_t));

  // sdl window dependent variables
  sdl_init(VEC_ZERO, WINDOW);
  size_t num_of_circles = WINDOW.x / CIRCLE_RADIUS;

  // scene and positions
  state->spring_scene = scene_init();
  vector_t anchor_pos = START_CIRCLE_POS;
  vector_t circ_pos = START_CIRCLE_POS;

  // creates and initalizes positions of all circles on screen
  for (size_t i = 0; i < num_of_circles; i++) {
    // adds in anchors
    scene_add_body(state->spring_scene,
                   make_spring_circle(anchor_pos, ANCHOR_COLOR, INFINITY));
    anchor_pos = vec_add(anchor_pos, CIRCLE_INCREMENT);

    // calculates change x position, then y
    double ratio = (double)i / (double)num_of_circles * (16 * M_PI);
    vector_t newV = {0, MAGNITUDE * cos(ratio)};
    circ_pos = vec_add(circ_pos, newV);

    // adds in spring circles
    scene_add_body(
        state->spring_scene,
        make_spring_circle(
            circ_pos,
            (rgb_color_t){
                pow((1 + sin((2 * RGB_SHIFT) + state->curr_color)) * 0.5,
                    BRIGHTNESS),
                pow((1 + sin((4 * RGB_SHIFT) + state->curr_color)) * 0.5,
                    BRIGHTNESS),
                pow((1 + sin(state->curr_color)) * 0.5, BRIGHTNESS)},
            CIRCLE_MASS));

    state->curr_color += COLOR_SHIFT;

    // goes back to origin
    circ_pos = vec_add(circ_pos, vec_negate(newV));
    // moves over by circle incrementation to next dude
    circ_pos = vec_add(circ_pos, CIRCLE_INCREMENT);
    // sets velocity to 0 and let forces take over
    body_set_velocity(scene_get_body(state->spring_scene,
                                     scene_bodies(state->spring_scene) - 1),
                      VEC_ZERO);
  }

  // creates all forces between bodies needed
  for (size_t i = 0; i < scene_bodies(state->spring_scene) - 1; i += 2) {
    body_t *body1 = scene_get_body(state->spring_scene, i + 1);
    body_t *anchor = scene_get_body(state->spring_scene, i);
    create_spring(state->spring_scene, K, body1, anchor);
    create_drag(
        state->spring_scene,
        GAMMA * (pow(1 + (((double)i) / scene_bodies(state->spring_scene)), 3)),
        body1);
  }

  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  scene_tick(state->spring_scene, dt);
  scene_draw(state->spring_scene);
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->spring_scene);
  free(state);
}