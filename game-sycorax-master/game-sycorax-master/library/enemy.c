#include "enemy.h"
#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "math.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"

// Ugly, find better solution
const vector_t SPAWN_RANGE = {.x = 1000, .y = 400};

const double EPSILON = 1.;
const double ENEMY_MASS = 30;
const rgb_color_t ENEMY_COLOR = {0, 1, 1};
const double ENEMY_RADIUS = 20;
const double ENEMY_HEALTH = 2.;

const int NUM_MISSILES_EXPLODED = 5;
const double EXPLODE_INTERVAL = 8.0;
const double EXPLODE_RADIUS = 12.0;
const double EXPLODE_VELOCITY_CONST = 20.0;
const rgb_color_t EXPLODE_COLOR = {0.6, 1, 0};

// Moves, turret looking
const double MOVING_INTERVAL = 3.;
const double MOVING_RADIUS = 20.;
const rgb_color_t MOVING_COLOR = {1, 0.7, 0};
const double STOP_INTERVAL = 3.;
const double MOVING_SPEED = 60.;

// Stationary, points at bro
const double STATIONARY_INTERVAL = 3.;
const double STATIONARY_RADIUS = 15.;
const rgb_color_t STATIONARY_COLOR = {1, 1, 0};

// Heart, heals spaceship
const double HEART_TIME_OUT_INTERVAL = 5.;
const double HEART_RADIUS = 12.;
const rgb_color_t HEART_COLOR = {1, 0, 0};

const double ADD_ENEMY_INTERVAL = 5.0;
const double ENEMY_DAMAGE = 1.0;
const vector_t ENEMY_VELOCITY = (vector_t){50., 50.};

const double MISSILE_SPEED = 30.;
const double MISSILE_DRAG = 480.;
const double EXPLODING_MISSILE_DRAG = 200.;
const double EXPLODING_MISSILE_SPEED = 20.;
const double MISSILE_TIME_OUT_INTERVAL = 10.0;
const double EXPLODING_MISSILE_TIME_OUT_INTERVAL = 3.;
const double MISSLE_DAMAGE = 1;
const double MISSILE_RADIUS = 5;
const rgb_color_t MISSILE_COLOR = {1, 0, 0};

typedef struct enemy {
  ctype_t enemy_type;
  double timer;
  double interval;
  // firing interval for monsters and self time out intervals
  // for missiles
  bool fire;
  bool killed;
  parametric_t parametric;
  double damage;
  double total_time;
  double parametric_helper;
  double scale;
} enemy_t;

vector_t new_pos_on_screen() {
  return (vector_t){.x = (float_from_0_to_1() - 0.5) * SPAWN_RANGE.x,
                    .y = (float_from_0_to_1() - 0.5) * SPAWN_RANGE.y};
}

vector_t moving_enemy_behaviour(body_t *moving) {
  enemy_t *e = (enemy_t *)body_get_info(moving);
  vector_t curr = body_get_velocity(moving);
  vector_t curr_pos = body_get_centroid(moving);
  if (e->total_time > e->parametric_helper) {
    if (vec_mag(curr) < 0.1) {
      // set a new heading, do math for it and its time
      vector_t next_pos = new_pos_on_screen();
      vector_t direction = vec_subtract(next_pos, curr_pos);
      double time_to = vec_mag(direction) / MOVING_SPEED;
      e->parametric_helper += time_to;
      return vec_multiply(MOVING_SPEED, unit_vec(next_pos, curr_pos));
    } else {
      // stop, set a time
      e->parametric_helper += STOP_INTERVAL;
      return VEC_ZERO;
    }
  } else {
    return curr;
  }
}

vector_t default_parametric(body_t *mr_bro) {
  return body_get_velocity(mr_bro);
}

void follow_parametric(body_t *enemy) {
  enemy_t *e = (enemy_t *)body_get_info(enemy);
  body_set_velocity(enemy, e->parametric(enemy));
}

ctype_t *make_type_info(ctype_t type) {
  ctype_t *info = malloc(sizeof(*info));
  *info = type;
  return info;
}

ctype_t get_type(body_t *body) {
  return (*(enemy_t *)body_get_info(body)).enemy_type;
}

enemy_t *enemy_init(ctype_t type) {
  enemy_t *enemy = malloc(sizeof(enemy_t));
  enemy->enemy_type = type;
  enemy->scale = ENEMY_RADIUS;
  enemy->parametric = default_parametric;
  enemy->parametric_helper = 0.;
  if (type == EXPLODE) {
    enemy->interval = EXPLODE_INTERVAL;
    enemy->scale = EXPLODE_RADIUS;
  } else if (type == MOVING) {
    enemy->interval = MOVING_INTERVAL;
    enemy->scale = MOVING_RADIUS;
    enemy->parametric = moving_enemy_behaviour;
  } else if (type == STATIONARY) {
    enemy->interval = STATIONARY_INTERVAL;
    enemy->scale = STATIONARY_RADIUS;
  } else if (type == MISSILE) {
    enemy->interval = MISSILE_TIME_OUT_INTERVAL;
    enemy->scale = MISSILE_RADIUS;
  } else if (type == HEART) {
    enemy->interval = HEART_TIME_OUT_INTERVAL;
    enemy->scale = HEART_RADIUS;
  }
  enemy->timer = 0.0;
  enemy->fire = false;
  enemy->killed = false;
  enemy->damage = ENEMY_DAMAGE;
  enemy->total_time = 0.0;
  return enemy;
}

// gets index of given enemy_type
size_t get_index(scene_t *scene, ctype_t type) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    enemy_t enemy = *(enemy_t *)body_get_info(body);
    if (enemy.enemy_type == type) {
      return i;
    }
  }
  return 0;
}

void missile_collision(body_t *body1, body_t *missile, vector_t axis,
                       void *aux) {
  body_set_health(body1, body_get_health(body1) - MISSLE_DAMAGE);
  if (body_get_health(body1) <= 0) {
    body_remove(body1);
  }
  body_remove(missile);
}

body_t *add_enemy(scene_t *scene, vector_t pos, ctype_t type) {
  enemy_t *e = enemy_init(type);
  rgb_color_t curr_color = ENEMY_COLOR;
  list_t *shape = make_circle(e->scale);
  switch (type) {
  case MOVING:
    shape = make_turret(pos, e->scale);
    curr_color = MOVING_COLOR;
    break;
  case STATIONARY:
    shape = make_enemy_stationary(e->scale);
    curr_color = STATIONARY_COLOR;
    break;
  case MISSILE:
    shape = make_rectangle(e->scale);
    curr_color = MISSILE_COLOR;
    break;
  case HEART:
    shape = make_circle(HEART_RADIUS);
    curr_color = HEART_COLOR;
  default:
    break;
  }

  body_t *enemy =
      body_init_with_info(shape, ENEMY_MASS, curr_color, ENEMY_HEALTH, e, free);
  body_set_centroid(enemy, pos);
  scene_add_body(scene, enemy);
  return enemy;
}
// double get_health(enemy_t *bruh) { return bruh->health; }

void enemy_shoot(scene_t *scene, body_t *enemy, vector_t direction, double drag,
                 double force, double timeout) {
  body_t *mis =
      add_enemy(scene, vec_add(body_get_centroid(enemy), direction), MISSILE);
  body_set_velocity(mis, vec_multiply(EXPLODE_VELOCITY_CONST, direction));
  ((enemy_t *)body_get_info(mis))->interval = timeout;
  for (int i = 0; i < scene_bodies(scene); i++) {
    body_t *bod = scene_get_body(scene, i);
    ctype_t bod_info = *(ctype_t *)body_get_info(bod);
    if (bod_info != (ctype_t)MISSILE && bod_info != (ctype_t)HEART) {
      create_collision(scene, bod, mis, missile_collision, NULL, free);
    }
  }
  create_follow(scene, force, drag, mis,
                scene_get_body(scene, get_index(scene, SPACESHIP)));
  sound_effect();
}

void enemy_time(scene_t *scene, body_t *enemy, double dt) {
  enemy_t *e = (enemy_t *)body_get_info(enemy);
  ctype_t type = get_type(enemy);
  follow_parametric(enemy);
  e->timer += dt;

  if (e->timer > e->interval) {
    if (type == STATIONARY || type == MOVING) {
      size_t space_idx = get_index(scene, SPACESHIP);
      vector_t cent = body_get_centroid(scene_get_body(scene, space_idx));
      vector_t space_enemy_direction =
          vec_multiply(e->scale + MISSILE_RADIUS + EPSILON,
                       unit_vec(cent, body_get_centroid(enemy)));
      enemy_shoot(scene, enemy, space_enemy_direction, MISSILE_DRAG,
                  MISSILE_SPEED, MISSILE_TIME_OUT_INTERVAL);
      e->timer = 0.0;
    }

    if (type == EXPLODE) {
      explode(scene, enemy);
    }

    if (type == HEART) {
      body_remove(enemy);
      return;
    }
  }
  e->total_time += dt;
}

void missile_time_out(scene_t *scene, body_t *missile) {
  enemy_t *e = (enemy_t *)body_get_info(missile);
  if (e->total_time > e->interval) {
    body_remove(missile);
  }
}

void moving_enemy(scene_t *scene, body_t *enemy) {
  // vector_t centroid = body_get_centroid(enemy);
  // double e_time = ((enemy_t *)body_get_info(enemy))->total_time;
  // // printf("enemy time %f\n", e_time);
  // double period = 2 * M_PI * 50;

  // vector_t vel = {vec_mag(ENEMY_VELOCITY) * cos(period * e_time),
  //                 vec_mag(ENEMY_VELOCITY) * sin(period * e_time)};
  // body_set_velocity(enemy, vel);
  // body_set_centroid(enemy, (vector_t){90 * cos(period * e_time), 90 *
  // sin(period * e_time)}); printf("centroid x %d, centroid y%d\n",
  // body_get_centroid(enemy).x, body_get_centroid(enemy).y);
}

void explode(scene_t *scene, body_t *enemy) {
  enemy_t *e = (enemy_t *)body_get_info(enemy);
  // vector_t centroid = body_get_centroid(enemy);
  double angle = ((double)(2 * M_PI)) / ((double)NUM_MISSILES_EXPLODED);
  for (size_t i = 0; i < NUM_MISSILES_EXPLODED; i++) {
    double circle_ang = (double)(i)*angle;
    vector_t vec = (vector_t){cos(circle_ang), sin(circle_ang)};
    enemy_shoot(
        scene, enemy,
        vec_multiply(e->scale + sqrt(3) * MISSILE_RADIUS + EPSILON, vec),
        EXPLODING_MISSILE_DRAG, EXPLODING_MISSILE_SPEED,
        EXPLODING_MISSILE_TIME_OUT_INTERVAL);
  }
  body_remove(enemy);
}