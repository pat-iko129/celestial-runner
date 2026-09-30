#include "level.h"
#include "body.h"
#include "color.h"
#include "enemy.h"

void highlight(body_t *bod, body_t *stupid_idiot_bebe_fucking_heck,
               vector_t useless2, void *useless3) {
  body_set_color(bod, YELLOW);
  body_set_color(stupid_idiot_bebe_fucking_heck, YELLOW);
}

void lose_health_on_hit(body_t *bod1, body_t *missile, vector_t bruh,
                        void *lololol) {
  //   enemy_t * e = (enemy_t *) body_get_info(missile);
  //   body_lose_health(bod1, get_health(e));
  //   if (body_get_health(bod1) <= 0){
  //     body_remove(bod1);
  //   }
  //   body_remove(missile);
}