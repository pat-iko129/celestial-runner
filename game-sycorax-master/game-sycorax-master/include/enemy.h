#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "scene.h"
#include "state.h"
#include "vector.h"

// enum deez
// celestial_type_t = ctype_t
typedef enum {
  SPACESHIP,
  MISSILE,
  EXPLODE,
  MOVING,
  STATIONARY,
  HEART,
  BORDER
} ctype_t;

/**
 * A function called to set a body's velocity for all time
 * Returns the velocity for that specific time
 */
typedef vector_t (*parametric_t)(body_t *broski);

typedef struct enemy enemy_t;

void follow_parametric(body_t *enemy);

ctype_t *make_type_info(ctype_t type);

ctype_t get_type(body_t *body);

enemy_t *enemy_init(ctype_t type);

body_t *add_enemy(scene_t *scene, vector_t pos, ctype_t type);

void enemy_shoot(scene_t *scene, body_t *enemy, vector_t dirction, double drag,
                 double force, double timeout);

void enemy_time(scene_t *scene, body_t *enemy, double dt);

void missile_time_out(scene_t *scene, body_t *missile);

void highlight(body_t *bod, body_t *stupid_idiot_bebe_fucking_heck,
               vector_t useless2, void *useless3);
void lose_health_on_hit(body_t *bod1, body_t missle, vector_t bruh,
                        void *lololol);
void moving_enemy(scene_t *scene, body_t *enemy);

void missile_collision(body_t *body1, body_t *missile, vector_t axis,
                       void *aux);

void explode(scene_t *scene, body_t *enemy);

// double get_health(enemy_t *bruh);
#endif