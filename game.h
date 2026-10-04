#ifndef GAME_H
#define GAME_H

#include "render.h"

typedef struct player {
    vec3_t pos;
    float angle;
    sector_t* current_sector;
} player_t;

void g_game_loop(player_t* player);

#endif