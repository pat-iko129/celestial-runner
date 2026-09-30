#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "body.h"
#include "color.h"
#include "enemy.h"
#include "vector.h"

typedef struct level level_t;

// make a struct for levels
void level_logic(list_t *level, int n_of_enemies);

void highlight(body_t *bod, body_t *stupid_idiot_bebe_fucking_heck,
               vector_t useless2, void *useless3);
void lose_health_on_hit(body_t *bod1, body_t *missle, vector_t bruh,
                        void *lololol);
#endif