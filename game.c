#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <SDL.h>

#include "keyboard.h"
#include "typedef.h"
#include "game.h"
#include "render.h"
#include "util.h"

void g_move_player(player_t* player, vec3_t delta, float coll_mult) {
    for (int i = 0; i < player->current_sector->wall_count; i++) {
        wall_t* wall = &player->current_sector->walls[i];
        vec2_t vec_wp1_wp2 = { wall->p2.x - wall->p1.x, wall->p2.y - wall->p1.y };
        vec2_t vec_wp1_player = { player->pos.x - wall->p1.x, player->pos.y - wall->p1.y };

        vec2_t vec_wp1_player_next = { player->pos.x + delta.x - wall->p1.x, player->pos.y + delta.y - wall->p1.y };
        vec2_t vec_wp1_player_next_coll = { player->pos.x + delta.x * coll_mult - wall->p1.x, player->pos.y + delta.y * coll_mult - wall->p1.y };

        if (SIGN(cross2d(vec_wp1_wp2, vec_wp1_player)) != SIGN(cross2d(vec_wp1_wp2, vec_wp1_player_next_coll))) {
            //if (wall->collidable) return;
        }

        if (wall->is_portal &&
            cross2d(vec_wp1_wp2, vec_wp1_player) < 0 &&
            SIGN(cross2d(vec_wp1_wp2, vec_wp1_player)) != SIGN(cross2d(vec_wp1_wp2, vec_wp1_player_next))) {
            player->current_sector = wall->portal_sector;
            break;
        }
    }

    player->pos.x += delta.x;
    player->pos.y += delta.y;
    player->pos.z += delta.z;
}

void g_game_loop(player_t* player) {
    float sy_offs = 0;

    clip_mask_t clip_mask = (clip_mask_t){
        .top = calloc(FB_WIDTH, sizeof(int)),
        .bottom = malloc(FB_WIDTH * sizeof(int)),
        .start_x = 0,
        .end_x = FB_WIDTH - 1
    };

    for (int i=0;i<FB_WIDTH;i++) {
        clip_mask.bottom[i] = FB_HEIGHT - 1;
    }

    uint64_t prev = SDL_GetPerformanceCounter();
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        uint64_t now = SDL_GetPerformanceCounter();
        float dt = (float)(now - prev) / SDL_GetPerformanceFrequency();
        prev = now;

        r_clear_buffer();
        k_update();

        if (keys.q) player->angle -= 1 * dt;
        if (keys.e) player->angle += 1 * dt;

        float dx = 0.0f, dy = 0.0f;
        if (keys.w) {
            dx += sin(player->angle) * 50 * dt;
            dy += cos(player->angle) * 50 * dt;
        }
        if (keys.s) {
            dx -= sin(player->angle) * 50 * dt;
            dy -= cos(player->angle) * 50 * dt;
        }
        if (keys.a) {
            dx += sin(player->angle - M_PI / 2) * 50 * dt;
            dy += cos(player->angle - M_PI / 2) * 50 * dt;
        }
        if (keys.d) {
            dx += sin(player->angle + M_PI / 2) * 50 * dt;
            dy += cos(player->angle + M_PI / 2) * 50 * dt;
        }
        g_move_player(player, (vec3_t){.x = dx, .y = dy, .z = 0}, 2.0f);

        if (keys.r) player->pos.z += 50 * dt;
        if (keys.f) player->pos.z -= 50 * dt;
        if (keys.t) sy_offs += 200 * dt;
        if (keys.g) sy_offs -= 200 * dt;

        camera_t camera = {
            .pos = player->pos,
            .angle = player->angle,
            .sy_offs = sy_offs
        };

        r_draw_sector(&camera, player->current_sector, NULL, &clip_mask);

        r_update();
    }

    free(clip_mask.top);
    free(clip_mask.bottom);
}