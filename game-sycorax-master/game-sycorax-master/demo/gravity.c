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

// star specifics
const int NUMBER_POINTS = 2;
const int LOWEST_POLYGON = 3; // fewest number of points in a star, triangle
const int POLYGON_RANGE = 5;  // polygon_range + lowest_polygon = max number of
                              // points we want in a star
const int NUMBER_OF_POLYGONS = 4;
const double RAD = 25.;
const double MASS = 1.4210;
// color constants
const double RGB_SHIFT = M_PI / 3.0;
const double COLOR_SHIFT = 0.1;
const double BRIGHTNESS = 1.0;

// physics
const vector_t INITIAL_VELOCITY = {.x = 1.00, .y = -2.00};
const vector_t GRAVITY = {.x = 0, .y = -10};
const double OMEGA = 1; // radial velocity
const double TIME_INTERVAL = 10;
const double PER = 2;

// various constants
const vector_t WINDOW = {.x = 1000, .y = 500};
const vector_t START = {.x = (2 * RAD), .y = 250 - (2 * RAD)};
const int FINER = 1000;

/**
 * Stores the demo state
 * Use this to store any variable needed every 'tick' of your demo
 */
typedef struct state {
  // state should only hold things that are changing per state

  int curr_points;   // num of points for upcoming star
  double R_inc;      // color variable
  double total_time; // total time since last star dropped
  list_t *polygons;
} state_t;

/**
 * Adds a polygon to the polygon list in state
 *
 *  @param state
 *  @param p
 */
void addPolygon(state_t *state, body_t *p) { list_add(state->polygons, p); }

/**
 * Wrapper function for sdl_draw_polygon
 *
 *  @param polygon
 *  @param state
 */
void draw_polygon(body_t *polygon, state_t *state) {
  sdl_draw_polygon(body_get_shape(polygon), body_get_color(polygon));
}

/**
 * Takes in a polygon object and bounces it based on
 * the WINDOW_SIZE!
 * Bounces if it hits or goes beyond an edge
 *
 * @param polygon
 */
void polygon_bounce(body_t *polygon) {
  bool flipY = false;
  bool flipX = false;
  for (size_t i = 0; i < list_size(body_get_shape(polygon)); i++) {
    vector_t vertex = *(vector_t *)list_get(body_get_shape(polygon), i);

    // if it goes above screen, move back down by that distance in the y
    // direction
    if (vertex.y > WINDOW.y) {
      body_translate(polygon, vec_multiply(WINDOW.y - vertex.y, Y_HAT));
      flipY = true;
    }

    // same logic applies for the other two walls
    // not the right wall though, because it needs to leave screen
    if (vertex.y < 0) {
      body_translate(polygon, vec_multiply(-vertex.y, Y_HAT));
      flipY = true;
    }
    if (vertex.x < 0) {
      body_translate(polygon, vec_multiply(-vertex.x, X_HAT));
      flipX = true;
    }
  }

  // change velocity for bounce
  if (flipX) {
    vector_t new = {-body_get_velocity(polygon).x,
                    body_get_velocity(polygon).y};
    body_set_velocity(polygon, new);
  }
  if (flipY) {
    vector_t new = {body_get_velocity(polygon).x,
                    -body_get_velocity(polygon).y};
    body_set_velocity(polygon, new);
  }
}

/**
 * @brief Checks if polygon is off screen
 *
 * @param polygon
 * @return false -> not off screen
 */
bool off_screen(body_t *polygon) {
  for (size_t i = 0; i < list_size(body_get_shape(polygon)); i++) {
    vector_t side = *(vector_t *)list_get(body_get_shape(polygon), i);
    if (side.y < WINDOW.y && side.y > 0 && side.x < WINDOW.x && side.x > 0) {
      return false; // not off screen -> on screen
    }
  }
  return true;
}

/**
 * @brief Adds new star (body_t *) to polygon list in state
 *
 * @param state
 */
void add_new_star(state_t *state) {
  // creates new star using current state variables
  body_t *new_poly = body_init(
      make_star(state->curr_points, START, RAD, RAD), MASS,
      (rgb_color_t){
          pow((1 + sin((2 * RGB_SHIFT) + state->R_inc)) * 0.5, BRIGHTNESS),
          pow((1 + sin((4 * RGB_SHIFT) + state->R_inc)) * 0.5, BRIGHTNESS),
          pow((1 + sin(state->R_inc)) * 0.5, BRIGHTNESS)});
  body_set_velocity(new_poly, INITIAL_VELOCITY);
  addPolygon(state, new_poly);

  // the next two lines updates state variables for next star
  // randomly regenerates num of points we want
  state->curr_points = LOWEST_POLYGON + (rand() % POLYGON_RANGE);
  // shifts color
  state->R_inc += COLOR_SHIFT;
}

/**
 * @brief Initializes sdl as well as the variables needed
 * Creates and stores all necessary variables for the demo in a created state
 * variable Returns the pointer to this state (This is the state emscripten_main
 * and emscripten_free work with)
 *
 * @return state_t*
 */
state_t *emscripten_init() {
  // only happens once, sets up frames for everything to start going
  // sdl is the one that's updating every single frame
  // once sdl ends, emscripten realizes its the end and frees all variables

  state_t *state = malloc(sizeof(state_t));
  sdl_init(VEC_ZERO, WINDOW);

  state->total_time = 0.00;
  state->curr_points = 5;
  state->R_inc = 0;
  state->polygons = list_init(
      NUMBER_OF_POLYGONS,
      (free_func_t)
          body_free); // initialization includes object specific freer function

  // initializes starts with a star
  add_new_star(state);
  return state;
}

/**
 * @brief Updates polygon specific physics
 * Rotates about center, translates by velocity, bounces if touches or goes
 * beyond a window edge
 *
 * @param polygon
 * @param dt
 */
void polygon_update(body_t *polygon, double dt) {
  body_set_rotation(polygon, body_get_rotation(polygon) + (OMEGA * dt));

  body_set_velocity(polygon, vec_add(body_get_velocity(polygon), GRAVITY));
  body_translate(polygon, body_get_velocity(polygon));
  polygon_bounce(polygon);
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
  state->total_time = state->total_time + dt;

  // Every period, we add a new star to page
  if ((state->total_time >= PER)) {
    add_new_star(state);
    state->total_time = 0; // time since last star dropped would be 0 since we
                           // just added a new star
  }

  // Every frame, sdl draws all the polygons onto the screen
  // draws backwards because we are freeing from the front
  // list_size would change within the for loop if we drew from 0 to len
  for (size_t i = list_size(state->polygons) - 1; (int)i >= 0; i--) {
    body_t *curr_poly = list_get(state->polygons, i);

    // if off screen, pop
    if (off_screen(curr_poly)) {
      // free the polygon points
      body_t *removed_polygon = list_remove(state->polygons, i);
      body_free(removed_polygon);
    }
    // otherwise, updates and redraws
    else {
      polygon_update(curr_poly, dt);
      draw_polygon(curr_poly, state);
    }
  }
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
  list_free(state->polygons);
  free(state);
}