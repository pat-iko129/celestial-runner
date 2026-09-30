#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <stdbool.h>

#include "math.h"

// Pellet constants
const double PELLET_RADIUS = 8;
const rgb_color_t PELLET_COLOR = {1, 1, 1};
const double PELLET_MASS = 5000;
const double RADIUS_KILL = 1;
// if monche reaches within this radius of the pacman centroid, free
const int NUM_PELLETS = 25;

// Pacman constants
const double PAC_MASS = 10;
const double PAC_RADIUS = 50;
const double PAC_MOUTH_ANGLE = M_PI / 6;
rgb_color_t PAC_COLOR = {1, 1, 0};
const vector_t INITIAL_VELOCITY = (vector_t){0., 0.};
const double ACC = 100.; // acceleration constant

// Pacman orientations
const double UP = M_PI / 2;
const double DOWN = 3 * M_PI / 2;
const double LEFT = M_PI;
const double RIGHT = 0;

// Window constants
const vector_t WINDOW = {.x = 1000, .y = 500};
const vector_t START = {.x = 500, .y = 250};

typedef struct state {
  // state should only hold things that are changing per state

  double R_inc; // color variable

  // checks if change in key
  char prev_pressed;
  char curr_pressed;

  // objects on scene
  body_t *pacman;
  scene_t *scene;
} state_t;

/**
 * @brief Creates a circle for the pellet, makes it into a pellet body and
 * shifts that to the given position
 *
 * @param pos position of pellet on screen
 * @return pellet body pointer
 */
body_t *make_pellet(vector_t pos) {
  list_t *pellet_list = make_circle(PELLET_RADIUS);
  body_t *pellet =
      body_init(pellet_list, PELLET_MASS, PELLET_COLOR); // creats circle pellet
  body_set_centroid(pellet, pos); // move pellet to given position
  return pellet;
}

/**
 * @brief Key handler function. Called every time a key is pressed
 * @param key which key is pressed
 * @param type key pressed or held
 * @param held_time zivial
 * @param state state_t of our current demo
 *
 */
void handler(char key, key_event_type_t type, double held_time, void *state) {

  ((state_t *)state)->curr_pressed = key;

  if (key == UP_ARROW) {
    // only keeps adding v if buttons continue in the same direction
    if (((state_t *)state)->curr_pressed == ((state_t *)state)->prev_pressed) {
      body_set_velocity(
          ((state_t *)state)->pacman,
          (vector_t){0, body_get_velocity(((state_t *)state)->pacman).y});
    }
    // otherwise instantly change directions by resetting v and rotating
    else {
      body_set_rotation(((state_t *)state)->pacman, UP);
      body_set_velocity(((state_t *)state)->pacman, VEC_ZERO);
    }
    vector_t up = vec_multiply(ACC, Y_HAT);
    body_set_velocity(
        ((state_t *)state)->pacman,
        vec_add(body_get_velocity(((state_t *)state)->pacman), up));
    ((state_t *)state)->prev_pressed = UP_ARROW;
  }

  if (key == DOWN_ARROW) {
    if (((state_t *)state)->curr_pressed == ((state_t *)state)->prev_pressed) {
      body_set_velocity(
          ((state_t *)state)->pacman,
          (vector_t){0, body_get_velocity(((state_t *)state)->pacman).y});
    } else {
      body_set_rotation(((state_t *)state)->pacman, DOWN);
      body_set_velocity(((state_t *)state)->pacman, VEC_ZERO);
    }
    vector_t down = vec_multiply(-1 * ACC, Y_HAT);
    body_set_velocity(
        ((state_t *)state)->pacman,
        vec_add(body_get_velocity(((state_t *)state)->pacman), down));
    ((state_t *)state)->prev_pressed = DOWN_ARROW;
  }

  if (key == LEFT_ARROW) {
    if (((state_t *)state)->curr_pressed == ((state_t *)state)->prev_pressed) {
      body_set_velocity(
          ((state_t *)state)->pacman,
          (vector_t){body_get_velocity(((state_t *)state)->pacman).x, 0});
    } else {
      body_set_rotation(((state_t *)state)->pacman, LEFT);
      body_set_velocity(((state_t *)state)->pacman, VEC_ZERO);
    }
    vector_t left = vec_multiply(-1 * ACC, X_HAT);
    body_set_velocity(
        ((state_t *)state)->pacman,
        vec_add(body_get_velocity(((state_t *)state)->pacman), left));
    ((state_t *)state)->prev_pressed = LEFT_ARROW;
  }

  if (key == RIGHT_ARROW) {
    if (((state_t *)state)->curr_pressed == ((state_t *)state)->prev_pressed) {
      body_set_velocity(
          ((state_t *)state)->pacman,
          (vector_t){body_get_velocity(((state_t *)state)->pacman).x, 0});
    } else {
      body_set_rotation(((state_t *)state)->pacman, RIGHT);
      body_set_velocity(((state_t *)state)->pacman, VEC_ZERO);
    }
    vector_t right = vec_multiply(ACC, X_HAT);
    body_set_velocity(
        ((state_t *)state)->pacman,
        vec_add(body_get_velocity(((state_t *)state)->pacman), right));
    ((state_t *)state)->prev_pressed = RIGHT_ARROW;
  }
}

/**
 * @brief Makes pacman body
 *
 * @param pac_list shape of pacman
 * @return pacman's body
 */
body_t *make_pacman(list_t *pac_list) {
  body_t *out = body_init(pac_list, PAC_MASS, PAC_COLOR);
  return out;
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  sdl_init(VEC_ZERO, WINDOW);
  state->scene = scene_init();
  state->pacman = make_pacman(pac_man_shape(PAC_RADIUS, PAC_MOUTH_ANGLE));
  state->curr_pressed = 0;
  state->prev_pressed = 0;
  body_set_centroid(state->pacman, vec_multiply(0.5, WINDOW));

  // adds initial pellets to screen
  for (size_t i = 0; i < NUM_PELLETS; i++) {
    scene_add_body(state->scene, make_pellet(*vec_rand(VEC_ZERO, WINDOW)));
  }
  // set pacman initial v
  body_set_velocity(state->pacman, INITIAL_VELOCITY);
  sdl_on_key((key_handler_t)handler);
  return state;
}

/**
 * @brief Checks if a pellet are within the range of pacman's center
 *
 * @param pacman pacman body pointer
 * @param pellet pellet body pointer
 * @return bool if pellet is within pacman's centroid
 */
bool is_food(body_t *pacman, body_t *pellet) {
  bool dude = (vec_mag(vec_subtract(body_get_centroid(pellet),
                                    body_get_centroid(pacman))) <= PAC_RADIUS);
  if (dude) {
    body_remove(pellet);
  }
  return dude;
}

/**
 * @brief Checks if pacman's off the screen in the x direction
 *
 * @param shape pacman's body's list of vectors
 * @return bool if pacman is off the screen in x field
 */
bool off_screen_x(list_t *shape) {
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t pac = *(vector_t *)list_get(shape, i);
    if (pac.x < WINDOW.x && pac.x > 0) {
      return false; // not off screen -> on screen
    }
  }
  return true;
}

/**
 * @brief Checks if pacman's off the screen in the y direction
 *
 * @param shape pacman's body's list of vectors
 * @return bool if pacman is off the screen in y field
 */
bool off_screen_y(list_t *shape) {
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t pac = *(vector_t *)list_get(shape, i);
    if (pac.y < WINDOW.y && pac.y > 0) {
      return false;
    }
  }
  return true;
}

void emscripten_main(state_t *state) {
  rainbows(body_get_color(state->pacman), .2);
  sdl_clear();
  double last_tick = time_since_last_tick();
  list_t *pac = body_get_shape(state->pacman);

  // loop screen on x axis
  if (off_screen_x(pac)) {
    vector_t old_pos = body_get_centroid(state->pacman);
    if (old_pos.x > WINDOW.x) {
      body_set_centroid(state->pacman, (vector_t){0 - PAC_RADIUS, old_pos.y});
    } else {
      body_set_centroid(state->pacman,
                        (vector_t){WINDOW.x + PAC_RADIUS, old_pos.y});
    }
  }

  // loop screen on y axis
  if (off_screen_y(pac)) {
    vector_t old_pos = body_get_centroid(state->pacman);
    if (old_pos.y > WINDOW.y) {
      body_set_centroid(state->pacman, (vector_t){old_pos.x, -PAC_RADIUS});
    } else {
      body_set_centroid(state->pacman,
                        (vector_t){old_pos.x, WINDOW.y + PAC_RADIUS});
    }
  }

  scene_tick(state->scene, last_tick);
  scene_draw(state->scene);

  for (size_t i = 0; i < (scene_bodies(state->scene)); i++) {
    if (is_food(state->pacman, scene_get_body(state->scene, i))) {
      scene_add_body(state->scene, make_pellet(*vec_rand(VEC_ZERO, WINDOW)));
    }
  }
  // if (is_food(state->pacman, scene_get_body(state->scene, i)) == true) {
  //     // for every pellet that's been ate another appears
  //     scene_remove_body(state->scene, i);
  //     scene_add_body(state->scene, make_pellet(*vec_rand(VEC_ZERO, WINDOW)));
  //   }
  //   list_t *pel = body_get_shape(scene_get_body(state->scene, i));
  //   sdl_draw_polygon(pel, PELLET_COLOR);
  //   list_free(pel);
  // }
  body_tick(state->pacman, last_tick);
  sdl_draw_polygon(pac, body_get_color(state->pacman));
  list_free(pac);
  sdl_show();
}

void emscripten_free(state_t *state) {
  body_free(state->pacman);
  scene_free(state->scene);
}