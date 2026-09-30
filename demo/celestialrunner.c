#include "body.h"
#include "collision.h"
#include "color.h"
#include "enemy.h"
#include "forces.h"
#include "list.h"
#include "math.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"

#include "SDL_mixer.h"
#include "SDL_thread.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
//#include <SDL2/SDL_ttf.h>

// Window constants
const vector_t WINDOW = {.x = 1000, .y = 500};
const vector_t START = {.x = 500, .y = 250};

// health bar constants
const vector_t HEALTH_BAR_SIZE = {.x = 1000, .y = 10};
const double HEALTH_BAR_MASS = 50;
const rgb_color_t HEALTH_BAR_COLOR = {1, 1, 1};
vector_t HEALTH_BAR_INIT_POS = {0, 500 / 2 - 10 / 2};
const double HEALTH_BAR_SCALAR = 5;
const double HEALTH_BAR_SPEED = 25000;
double HEALTH_POWER_UP_INTERVAL = 5.;
const double HEALTH_POWER_UP = 10.;

// spaceship constants
const double SPACESHIP_MASS = 50;
const rgb_color_t SPACESHIP_COLOR = {1, 1, 0.7};
vector_t SPACESHIP_INIT_POS = {0, 0};
const double SPACESHIP_SCALAR = 10;
const double SPACESHIP_SPEED = 25000;

const double SPACE_SHIP_HEALTH = 200;
// border constants
const rgb_color_t BORDER_COLOR = {1, 1, 1};
const double BORDER_THICKNESS = 5;
const double BORDER_MASS = INFINITY;

// orientations
const double UP = 0;
const double DOWN = M_PI;
const double LEFT = M_PI / 2;
const double RIGHT = 3 * M_PI / 2;

// Exploding constants
const double EXPLODE_HEALTH = 5;

// comic sans font dude
TTF_Font *COMIC_SANS;
TTF_Font *CONSOLAS;
const char *HEALTH_TEXT = "Health: ";

typedef struct state {
  // state should only hold things that are changing per state
  body_t *spaceship;
  scene_t *scene;
  double total_time;
  bool game_over;
  size_t level;
  size_t enemy_count;
  bool pause;

} state_t;

// int start_screen(state) {}

state_t *emscripten_init() {

  state_t *state = malloc(sizeof(state_t));

  // initializes sdl origin to the center of the screen rather than in a corner
  sdl_init(vec_multiply(-0.5, WINDOW), vec_multiply(0.5, WINDOW));
  int ttf = TTF_Init();
  if (ttf != 0) {
    printf("this is an error %s\n", TTF_GetError());
  }

  COMIC_SANS = TTF_OpenFont("assets/COMIC.TTF", 16);
  CONSOLAS = TTF_OpenFont("assets/CONSOLA.TTF", 16);

  if (!COMIC_SANS) {
    printf("this is an error %s\n", TTF_GetError());
  }
  // else{
  //   printf("hahah");
  // }
  // printf("%s\n", TTF_GetError());
  create_text(CONSOLAS, (char *)HEALTH_TEXT);

  state->scene = scene_init();
  state->total_time = 0;
  state->game_over = false;
  state->level = 1;
  state->pause = false;

  // body_t *health_bar = body_init_with_info(
  //     rectangle(HEALTH_BAR_SIZE.x, HEALTH_BAR_SIZE.y), HEALTH_BAR_MASS,
  //     HEALTH_BAR_COLOR, SPACE_SHIP_HEALTH, NULL, free);
  // body_set_centroid(health_bar, HEALTH_BAR_INIT_POS);
  // state->health_bar = health_bar;

  list_t *health_bar = rectangle(HEALTH_BAR_SIZE.x, HEALTH_BAR_SIZE.y);

  polygon_set_centroid(health_bar, HEALTH_BAR_INIT_POS);
  sdl_draw_polygon(health_bar, HEALTH_BAR_COLOR);

  list_t *shape = make_runner(SPACESHIP_SCALAR);
  enemy_t *e = enemy_init((ctype_t)SPACESHIP);
  body_t *spaceship = body_init_with_info(
      shape, SPACESHIP_MASS, SPACESHIP_COLOR, SPACE_SHIP_HEALTH, e, free);
  body_set_centroid(spaceship, SPACESHIP_INIT_POS);
  state->spaceship = spaceship;
  scene_add_body(state->scene, spaceship);
  return state;
}

void handler(char key, key_event_type_t type, double held_time, void *state) {
  if (key == P && type == KEY_PRESSED) {
    ((state_t *)state)->pause = !((state_t *)state)->pause;
  }
  if (key == MOUSE_MOTIONS) {
    int *x = malloc(sizeof(int));
    int *y = malloc(sizeof(int));
    printf("yuh\n");
    SDL_PumpEvents();
    Uint32 mouse_state = SDL_GetMouseState(x, y);
    printf("x: coord %d, y: coord %d\n", *x, *y);
  }
}

void mouse_handles(state_t *state) {
  int x = 0;
  int y = 0;
  SDL_Event *event = malloc(sizeof(*event));
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_MOUSEMOTION:
      x = event->motion.x;
      y = event->motion.y;
      printf("x coord: %d, y coord: %d", x, y);
      break;
    }
  }
}

void new_handles(state_t *state) {
  body_t *curr = state->spaceship;
  const Uint8 *stte = SDL_GetKeyboardState(NULL);
  if (stte[SDL_SCANCODE_RIGHT]) {
    body_add_impulse(curr, (vector_t){SPACESHIP_SPEED, 0});
  }
  if (stte[SDL_SCANCODE_LEFT]) {
    body_add_impulse(curr, (vector_t){-SPACESHIP_SPEED, 0});
  }
  if (stte[SDL_SCANCODE_UP]) {
    body_add_impulse(curr, (vector_t){0, SPACESHIP_SPEED});
  }
  if (stte[SDL_SCANCODE_DOWN]) {
    body_add_impulse(curr, (vector_t){0, -SPACESHIP_SPEED});
  }
  // if (stte[SDL_SCANCODE_P]) {
  //   state->pause = !(state->pause);
  // } // use given key handler!!
}

list_t *update_health_bar(body_t *spaceship) {
  double val = body_get_health(spaceship) / SPACE_SHIP_HEALTH;
  list_t *health_bar = rectangle(val * HEALTH_BAR_SIZE.x, HEALTH_BAR_SIZE.y);
  polygon_set_centroid(
      health_bar,
      (vector_t){((HEALTH_BAR_INIT_POS.x + WINDOW.x) / 2) * val - WINDOW.x / 2,
                 HEALTH_BAR_INIT_POS.y});
  return health_bar;
}

vector_t rand_vec_in_window() {
  double x_quad;
  double y_quad;
  if (float_from_0_to_1() < .5) {
    x_quad = -1;
  } else {
    x_quad = 1;
  }
  if (float_from_0_to_1() < .5) {
    y_quad = -1;
  } else {
    y_quad = 1;
  }

  vector_t newPos = {x_quad * WINDOW.x / 2 * float_from_0_to_1(),
                     (y_quad * WINDOW.y / 2 - HEALTH_BAR_SIZE.y) *
                         float_from_0_to_1()};
  return newPos;
}

void health_increase(body_t *body1) {
  body_set_health(body1, body_get_health(body1) + HEALTH_POWER_UP);
  if (body_get_health(body1) >= SPACE_SHIP_HEALTH) {
    body_set_health(body1, SPACE_SHIP_HEALTH);
  }
}

void emscripten_main(state_t *state) {
  sdl_clear();
  SDL_Texture *health_texture = create_text(CONSOLAS, (char *)HEALTH_TEXT);
  SDL_Texture *img_texture = screen_load();

  // printf("time lmao %f\n", state->total_time);
  double dt = time_since_last_tick();
  if (!state->game_over && !state->pause) {
    // New key handler, more smooth. Might get flamed at code review though. Oh
    // well
    new_handles(state);

    // pause button
    state->total_time += dt;

    double enemy_int = 4. * state->level;

    if (state->enemy_count != 0 || true) {
      if (state->total_time >= enemy_int) {

        // body_t *moving = add_enemy(state->scene, rand_vec_in_window(),
        // MOVING); state->enemy_count++; enemy_time(state->scene, moving, dt);

        body_t *moving = add_enemy(state->scene, rand_vec_in_window(), MOVING);
        state->enemy_count++;

        body_t *explode =
            add_enemy(state->scene, rand_vec_in_window(), EXPLODE);
        state->enemy_count++;

        // link to all previous missiles
        for (size_t i = 0; i < scene_bodies(state->scene); i++) {
          body_t *bod = scene_get_body(state->scene, i);
          if (*(ctype_t *)body_get_info(bod) == (ctype_t)MISSILE) {
            create_collision(state->scene, moving, bod, missile_collision, NULL,
                             free);
            create_collision(state->scene, explode, bod, missile_collision,
                             NULL, free);
          }
        }

        if (state->total_time >= HEALTH_POWER_UP_INTERVAL) {
          body_t *heart = add_enemy(state->scene, rand_vec_in_window(), HEART);
          state->enemy_count++;
          HEALTH_POWER_UP_INTERVAL += HEALTH_POWER_UP_INTERVAL;
          create_collision(state->scene, state->spaceship, heart,
                           health_increase, NULL, free);
        }

        // link spaceship to heart health

        // enemy_time(state->scene, stat, dt);

        // body_t * missile = add_enemy(state->scene, VEC_ZERO, MISSILE);
        // state->enemy_count++;

        state->total_time = 0.0;
      }
    }

    body_set_velocity(state->spaceship, VEC_ZERO);
    scene_tick(state->scene, dt);

    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
      body_t *bod = scene_get_body(state->scene, i);
      enemy_time(state->scene, bod, dt);
      ctype_t type = get_type(bod);
      if (type == MOVING || type == STATIONARY) {
        body_point_to(bod, body_get_centroid(state->spaceship));
      }
      if (type == MISSILE) {
        missile_time_out(state->scene, bod);
      }
    }

    if (state->enemy_count <= 0 && false) {
      state->level++;
    }

    // for (size_t i = 0; i < scene_bodies(state->scene); i++){
    //   body_t * body = (body_t *)scene_get_body(state->scene, i);
    //   ctype_t type = get_type(body);
    //   if (type == STATIONARY || type == MOVING || type == EXPLODE){
    //     enemy_time(state->scene, body, dt);
    //   }
    //   body_free(body);
    // }

    // polygon_set_centroid(health_bar, HEALTH_BAR_INIT_POS);
    if (body_get_health(state->spaceship) <= 0.) {
      state->game_over = true;
    } else {
      body_constrain_centroid(state->spaceship, vec_multiply(-0.5, WINDOW),
                              vec_multiply(0.5, WINDOW));
    }
  }
  if (body_get_health(state->spaceship) > 0) {
    list_t *health_bar = update_health_bar(state->spaceship);
    sdl_draw_polygon(health_bar, HEALTH_BAR_COLOR);
  } else {
    // sdl_draw_polygon(health_bar, HEALTH_BAR_COLOR);
  }
  scene_draw(state->scene);
  sdl_on_key((key_handler_t)handler);

  sdl_show();
  // SDL_DestroyTexture(img_texture);
  // SDL_DestroyTexture(health_texture);
}

void emscripten_free(state_t *state) { scene_free(state->scene); }