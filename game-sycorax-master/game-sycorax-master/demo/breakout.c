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
const double EPSILON = 0.1;

// Block Constants
const double BLOCK_MASS = 1;
const double BLOCK_SCALE_FACTOR = 50;
// const char *B = "B";
const rgb_color_t BLOCK_COLOR = {1, 0, 0};
const int LIST_SIZE = 50;
const int NUM_OF_BLOCKS = 50;

// dude constants
const double DUDE_MASS = INFINITY;
// const char *D = "D";
const rgb_color_t DUDE_COLOR = {1, 0, 1};
vector_t DUDE_INIT_POS = {500, 10};
const double DUDE_SCALAR = 50;
const double DUDE_WIDTH = 100;
const double DUDE_HEIGHT = 20;

// ball constants
const double BALL_MASS = 1;
const rgb_color_t BALL_COLOR = {1, 0, 1};
const double BALL_R = 10;
const vector_t BALL_INITIAL_VELOCITY = {0, 150};
const double BALL_SPEED = 400;

// orientations
const double UP = M_PI / 2;
const double DOWN = 3 * M_PI / 2;
const double LEFT = M_PI;
const double RIGHT = 0;
const int WIDE = 20;
const int TALL = 7;

// movement
const double MOVE = 800;

// rainbow time!!
const double RGB_SHIFT = M_PI / 1.5;
const double BRIGHTNESS = 1;
const double COLOR_SHIFT = .8;

// border constants
const rgb_color_t BORDER_COLOR = {1, 1, 1};
const double BORDER_THICKNESS = 5;
const double BORDER_MASS = INFINITY;

// misc
const double ELASTICITY = 0.5;

// enum deez
typedef enum { BALLER_MAN, BLOCK, DUDE, BORDER } breakout_type_t;

breakout_type_t *make_type_info(breakout_type_t type) {
  breakout_type_t *info = malloc(sizeof(*info));
  *info = type;
  return info;
}

typedef struct state {
  // state should only hold things that are changing per state
  double R_inc;      // color variable
  double total_time; // total time

  // objects on scene
  body_t *dude;
  scene_t *scene;
  int has_ball;
  bool game_over;
} state_t;

body_t *baller_man(vector_t pos, vector_t vel, float r) {
  body_t *baller = body_init_with_info(make_circle(r), BALL_MASS, BALL_COLOR,
                                       make_type_info(BALLER_MAN), NULL);
  body_set_centroid(baller, pos);
  body_set_velocity(baller, vel);
  // AYOOOOOOOOOO
  return baller;
}

body_t *make_dude() {
  list_t *dude_shape = rectangle(DUDE_WIDTH, DUDE_HEIGHT);
  body_t *dude = body_init_with_info(dude_shape, DUDE_MASS, DUDE_COLOR,
                                     make_type_info(DUDE), (free_func_t)NULL);
  body_set_centroid(dude, DUDE_INIT_POS);
  body_set_velocity(dude, (vector_t){0, 0});
  return dude;
}

body_t *make_block(vector_t pos, rgb_color_t color, double w, double h) {
  list_t *block_list = rectangle(w, h);
  // polygon_rotate(block_list, M_PI / 2, (vector_t){0, 0});
  body_t *block = body_init_with_info(block_list, BLOCK_MASS, color,
                                      make_type_info(BLOCK), (free_func_t)NULL);
  body_set_centroid(block, pos);
  body_set_velocity(block, (vector_t){0, 0});
  return block;
}

body_t *make_border(vector_t pos, rgb_color_t color, double w, double h) {
  list_t *block_list = rectangle(w, h);
  // polygon_rotate(block_list, M_PI / 2, (vector_t){0, 0});
  body_t *block =
      body_init_with_info(block_list, BORDER_MASS, color,
                          make_type_info(BORDER), (free_func_t)NULL);
  body_set_centroid(block, pos);
  body_set_velocity(block, (vector_t){0, 0});
  return block;
}

void dude_bounce(body_t *dude) {
  list_t *shape = body_get_shape(dude);
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t vertex = *(vector_t *)list_get(shape, i);
    if (vertex.x < 0) {
      body_translate(dude, vec_multiply(-vertex.x, X_HAT));
    }
    if (vertex.x > WINDOW.x) {
      body_translate(dude, vec_multiply(WINDOW.x - vertex.x, X_HAT));
    }
  }
  free(shape);
}

void block_ball_collision(body_t *ball, body_t *block, vector_t axis,
                          void *aux) {
  if (*(breakout_type_t *)body_get_info(block) == 1) {
    body_remove(block);
  }
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
  if (key == LEFT_ARROW) {
    body_set_velocity(((state_t *)state)->dude, VEC_ZERO);

    body_set_velocity(((state_t *)state)->dude, (vector_t){-MOVE, 0});
    if (type == KEY_RELEASED) {
      body_set_velocity(((state_t *)state)->dude, VEC_ZERO);
    }
  }

  if (key == UP_ARROW && ((state_t *)state)->has_ball < 2) {
    vector_t dude_center = body_get_centroid(((state_t *)state)->dude);
    dude_center.y += (DUDE_HEIGHT * 0.5) + BALL_R + EPSILON;
    body_t *baller = baller_man(dude_center, BALL_INITIAL_VELOCITY, BALL_R);

    body_set_velocity(baller,
                      vec_add(BALL_INITIAL_VELOCITY,
                              body_get_velocity(((state_t *)state)->dude)));

    ((state_t *)state)->has_ball++;

    scene_add_body(((state_t *)state)->scene, baller);

    for (size_t i = 0; i < scene_bodies(((state_t *)state)->scene); i++) {
      body_t *cur_bod = scene_get_body(((state_t *)state)->scene, i);
      switch (*(breakout_type_t *)body_get_info(cur_bod)) {
      case BALLER_MAN:
        for (size_t j = 0; j < scene_bodies(((state_t *)state)->scene); j++) {
          body_t *possible_block = scene_get_body(((state_t *)state)->scene, j);
          switch (*(breakout_type_t *)body_get_info(possible_block)) {
          case BLOCK:
            create_collision(((state_t *)state)->scene, cur_bod, possible_block,
                             (collision_handler_t)block_ball_collision, NULL,
                             NULL);

          case DUDE:
            create_physics_collision(((state_t *)state)->scene, ELASTICITY,
                                     cur_bod, possible_block);
          case BORDER:
            create_physics_collision(((state_t *)state)->scene, ELASTICITY,
                                     cur_bod, possible_block);
          case BALLER_MAN:
            create_physics_collision(((state_t *)state)->scene, ELASTICITY,
                                     cur_bod, possible_block);
          }
        }
      case DUDE:
        break;
      case BLOCK:
        break;
      case BORDER:
        break;
      }
    }
  }

  if (key == RIGHT_ARROW) {
    body_set_velocity(((state_t *)state)->dude, VEC_ZERO);

    body_set_velocity(((state_t *)state)->dude, (vector_t){MOVE, 0});
    if (type == KEY_RELEASED) {
      body_set_velocity(((state_t *)state)->dude, VEC_ZERO);
    }
  }
}

size_t get_dude_index(scene_t *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    switch (*(breakout_type_t *)body_get_info(body)) {
    case DUDE:
      return i;
    default:
      break;
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
  state->has_ball = 0;

  float w = WINDOW.x / WIDE;
  float h = w * 0.5;
  vector_t init_pos = {w / 2, WINDOW.y - h / 2};
  vector_t pos = init_pos;

  double curr_color = 1;

  // make border
  body_t *border_l = make_border((vector_t){0, WINDOW.y / 2}, BORDER_COLOR,
                                 BORDER_THICKNESS, WINDOW.y);
  body_t *border_r = make_border((vector_t){WINDOW.x, WINDOW.y / 2},
                                 BORDER_COLOR, BORDER_THICKNESS, WINDOW.y);
  body_t *border_top = make_border((vector_t){WINDOW.x / 2, WINDOW.y},
                                   BORDER_COLOR, WINDOW.x, BORDER_THICKNESS);

  scene_add_body(state->scene, border_l);
  scene_add_body(state->scene, border_r);
  scene_add_body(state->scene, border_top);

  body_t *dude = make_dude();
  scene_add_body(state->scene, dude);

  size_t dude_index = get_dude_index(state->scene);
  state->dude = scene_get_body(state->scene, dude_index);

  for (int i = 0; i < WIDE; i++) {
    for (int j = 0; j < TALL; j++) {
      double block_col_r = (1 + sin((2 * RGB_SHIFT) + curr_color)) * 0.5;
      double block_col_g = (1 + sin((4 * RGB_SHIFT) + curr_color)) * 0.5;
      double block_col_b = (1 + sin(curr_color)) * 0.5;

      double rgb_mag =
          sqrt(pow(block_col_r, 2) + pow(block_col_g, 2) + pow(block_col_b, 2));

      double block_col_r_unit = block_col_r / rgb_mag;
      double block_col_g_unit = block_col_g / rgb_mag;
      double block_col_b_unit = block_col_b / rgb_mag;

      double block_col_r_true = block_col_r_unit * BRIGHTNESS;
      double block_col_g_true = block_col_g_unit * BRIGHTNESS;
      double block_col_b_true = block_col_b_unit * BRIGHTNESS;

      rgb_color_t block_color = {block_col_r_true, block_col_g_true,
                                 block_col_b_true};

      curr_color += COLOR_SHIFT;

      pos.x = init_pos.x + i * w;
      pos.y = init_pos.y - j * h;
      body_t *block_bro = make_block(pos, block_color, w * 0.9, h * 0.9);

      scene_add_body(state->scene, block_bro);
    }
  }

  sdl_on_key((key_handler_t)handler);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double last_tick = time_since_last_tick();
  state->total_time = state->total_time + last_tick;

  int block_count = 0;
  int ball_count = 1;
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    list_t *m_shape = body_get_shape(body);
    rgb_color_t color = body_get_color(body);
    switch (*(breakout_type_t *)body_get_info(body)) {
    case BLOCK:
      block_count++;
      break;
    case DUDE:
      dude_bounce(state->dude);
      break;
    case BALLER_MAN:
      ball_count++;
      body_set_velocity(
          body,
          vec_multiply(BALL_SPEED, normalize_vec(body_get_velocity(body))));
      if (body_get_centroid(body).y >= WINDOW.y) {
        body_remove(body);
        ball_count--;
        ball_count--;
      }
      if (body_get_centroid(body).y <= 0) {
        body_remove(body);
        ball_count--;
        ball_count--;
      }
    default:
      break;
    }
    sdl_draw_polygon(m_shape, color);
    free(m_shape);
  }
  if (ball_count == 0) {
    state->game_over = true;
  }
  // exit cases
  if (block_count <= 0) {
    state->game_over = true;
  }

  scene_tick(state->scene, last_tick);
  sdl_show();

  if (state->game_over) {
    emscripten_free(state);
    state = emscripten_init();
  }
}

void emscripten_free(state_t *state) {
  body_free(state->dude);
  scene_free(state->scene);
}