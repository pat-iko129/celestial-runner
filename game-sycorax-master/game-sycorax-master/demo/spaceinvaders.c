#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <stdbool.h>

#include "math.h"

// Window constants
const vector_t WINDOW = {.x = 1000, .y = 500};
const vector_t START = {.x = 500, .y = 250};

// guardian constants
const char *S = "S";
const double GUARD_MASS = 10;
const double GUARD_RADIUS = 5.;
rgb_color_t GUARD_COLOR = {1., 1., 1.};
const vector_t GUARD_INITIAL_VELOCITY = (vector_t){0., 0.};
const double GUARD_SCALE_FACTOR = 5.;
const double ACC = 100.; // acceleration constant

// monster constants
const int MONSTER_PIXELS = 16;
const char *M = "M";
const int LIST_SIZE = 50;
const double MONSTER_SCALE_FACTOR = 2.;
const double MONSTER_MASS = 10;
const rgb_color_t MONSTER_COLOR = {0, 1, 0};
const int NUM_OF_MONSTER = 50;
const vector_t MONSTER_VELOCITY = {100, 0};
vector_t MONSTER_INIT_POS = {MONSTER_SCALE_FACTOR * MONSTER_PIXELS * 0.5,
                             500 - MONSTER_SCALE_FACTOR *MONSTER_PIXELS * 0.5};
double MONSTER_INCREMENT =
    MONSTER_SCALE_FACTOR * MONSTER_PIXELS; // 16 bits long

const char *E = "E";

// laser constants
const char *L = "L";
const rgb_color_t LASER_COLOR = {1, 0, 0};
const vector_t LASER_VELOCITY = {0, 300};
const double LASER_MASS = 5;
const double LASER_SCALE_FACTOR = 4;
const double LASER_INTERVAL = 1;

// guardian orientations
const double UP = M_PI / 2;
const double DOWN = 3 * M_PI / 2;
const double LEFT = M_PI;
const double RIGHT = 0;
const int WIDE = 5;
const int TALL = 6;
// movement
const double MOVE = 200;

typedef struct state {
  // state should only hold things that are changing per state

  double R_inc;      // color variable
  double total_time; // total time

  double monster_time;

  // objects on scene
  body_t *guardian;
  scene_t *scene;

  bool game_over;
} state_t;

body_t *make_laser(vector_t pos, vector_t velocity) {
  list_t *laser_list = make_rectangle(LASER_SCALE_FACTOR);
  body_t *laser = body_init_with_info(laser_list, LASER_MASS, LASER_COLOR,
                                      (char *)L, (free_func_t)NULL);
  body_set_centroid(laser, pos);
  body_set_velocity(laser, velocity);
  return laser;
}

body_t *make_monster(vector_t pos) {
  list_t *monster_list = make_space_bruh(MONSTER_SCALE_FACTOR);
  body_t *monster = body_init_with_info(
      monster_list, MONSTER_MASS, MONSTER_COLOR, (char *)M, (free_func_t)NULL);
  body_set_centroid(monster, pos);
  body_set_velocity(monster, MONSTER_VELOCITY);
  return monster;
}

/**
 * @brief Checks if guardian's off the screen in the x direction
 *
 * @param shape guardian's body's list of vectors
 * @return bool if guardian is off the screen in x field
 */
bool off_screen_x(list_t *shape) {
  vector_t pos = polygon_centroid(shape);
  if (pos.x >= WINDOW.x - GUARD_SCALE_FACTOR || pos.x <= GUARD_SCALE_FACTOR) {
    return true;
  } else {
    return false;
  }
}

void guardian_bounce(body_t *guardian) {
  list_t *shape = body_get_shape(guardian);
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t vertex = *(vector_t *)list_get(shape, i);
    if (vertex.x < 0) {
      body_translate(guardian, vec_multiply(-vertex.x, X_HAT));
    }
    if (vertex.x > WINDOW.x) {
      body_translate(guardian, vec_multiply(WINDOW.x - vertex.x, X_HAT));
    }
  }
  free(shape);
}

/**
 * @brief
 *
 *
 * @param key which key is pressed
 * @param type key pressed or held
 * @param held_time zivial
 * @param state state_t of our current demo
 *
 */
void handler(char key, key_event_type_t type, double held_time, void *state) {
  if (key == LEFT_ARROW) {
    body_set_velocity(((state_t *)state)->guardian, VEC_ZERO);

    body_set_velocity(((state_t *)state)->guardian, (vector_t){-MOVE, 0});
    if (type == KEY_RELEASED) {
      body_set_velocity(((state_t *)state)->guardian, VEC_ZERO);
    }
  }

  if (key == UP_ARROW) {
    if (held_time < LASER_INTERVAL) {
      body_t *laser = make_laser(
          body_get_centroid(((state_t *)state)->guardian), LASER_VELOCITY);
      scene_add_body(((state_t *)state)->scene, laser);
      for (size_t i = 0; i < scene_bodies(((state_t *)state)->scene); i++) {
        body_t *curr_bod = scene_get_body(((state_t *)state)->scene, i);
        if (*(char *)body_get_info(curr_bod) == 'M') {
          create_destructive_collision(((state_t *)state)->scene, laser,
                                       curr_bod);
        }
      }
    }
  }

  if (key == RIGHT_ARROW) {
    body_set_velocity(((state_t *)state)->guardian, VEC_ZERO);

    body_set_velocity(((state_t *)state)->guardian, (vector_t){MOVE, 0});
    if (type == KEY_RELEASED) {
      body_set_velocity(((state_t *)state)->guardian, VEC_ZERO);
    }
  }
}

size_t get_spaceship_index(scene_t *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    if (*(char *)body_get_info(body) == 'S') {
      return i;
    }
  }
  return 0;
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  sdl_init(VEC_ZERO, WINDOW);
  state->scene = scene_init();
  state->total_time = 0;
  state->game_over = false;
  body_set_centroid(state->guardian, vec_multiply(0.5, WINDOW));
  body_set_velocity(state->guardian, GUARD_INITIAL_VELOCITY);

  vector_t pos = MONSTER_INIT_POS;
  for (int i = 0; i < WIDE; i++) {
    for (int j = 0; j < TALL; j++) {
      scene_add_body(state->scene, make_monster(vec_add(
                                       pos, vec_multiply(MONSTER_INCREMENT,
                                                         (vector_t){i, -j}))));
    }
  }

  body_t *guardian =
      body_init_with_info(make_spaceship(GUARD_RADIUS), GUARD_MASS, GUARD_COLOR,
                          (char *)S, (free_func_t)NULL);
  scene_add_body(state->scene, guardian);
  size_t spaceship_index = get_spaceship_index(state->scene);
  state->guardian = scene_get_body(state->scene, spaceship_index);
  sdl_on_key((key_handler_t)handler);
  return state;
}

// shifts rows of monsters when they hit the end of the screen
void shift_rows(body_t *monster) {
  vector_t pos = body_get_centroid(monster);
  //  - 2 * MONSTER_SCALE_FACTOR
  // + 2 * MONSTER_SCALE_FACTOR
  if (pos.x > WINDOW.x - MONSTER_PIXELS ||
      pos.x < MONSTER_PIXELS) { // MONSTER_PIXELS is width of spaceship
    pos.y -= TALL * MONSTER_INCREMENT;
    if (pos.x > WINDOW.x - MONSTER_PIXELS) {
      pos.x = WINDOW.x - MONSTER_PIXELS;
    }
    if (pos.x < MONSTER_PIXELS) {
      pos.x = MONSTER_PIXELS;
    }
    body_set_centroid(monster, pos);
    vector_t curr_v = body_get_velocity(monster);
    body_set_velocity(monster, (vector_t){-1 * curr_v.x, 0});
  }
}

void monster_fire(state_t *state) {
  int n = (int)scene_bodies(state->scene);
  int rand_monster = rand() % n;
  body_t *chosen_one = scene_get_body(state->scene, rand_monster);
  while (*(char *)body_get_info(chosen_one) != 'M') {
    rand_monster = rand() % n;
    chosen_one = scene_get_body(state->scene, rand_monster);
  }
  body_t *laser =
      make_laser(body_get_centroid(chosen_one), vec_negate(LASER_VELOCITY));
  scene_add_body(state->scene, laser);
  create_destructive_collision(state->scene, laser, state->guardian);
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double last_tick = time_since_last_tick();
  state->total_time = state->total_time + last_tick;
  state->monster_time = state->monster_time + last_tick;

  int m_count = 0;

  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    list_t *m_shape = body_get_shape(body);
    rgb_color_t color = body_get_color(body);
    if (*(char *)body_get_info(body) == 'M') { // monster
      m_count++;
      shift_rows(body);
      if (body_get_centroid(body).y < MONSTER_PIXELS) {
        state->game_over = true;
      }
    }
    if (*(char *)body_get_info(body) == 'S') { // guardian
      guardian_bounce(state->guardian);
    }
    sdl_draw_polygon(m_shape, color);
    free(m_shape);
  }

  // monsters shooting
  if (state->monster_time > LASER_INTERVAL) {
    monster_fire(state);
    state->monster_time = 0;
  }

  // exit cases
  if (m_count == 0) {
    state->game_over = true;
  }

  if (body_is_removed(state->guardian)) {
    state->game_over = true;
    exit(0);
  }
  if (state->game_over) {
    exit(0);
  } else {
    scene_tick(state->scene, last_tick);
    sdl_show();
  }
}

void emscripten_free(state_t *state) {
  body_free(state->guardian);
  scene_free(state->scene);
}