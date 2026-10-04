#include <SDL.h>

#include "render.h"
#include "typedef.h"
#include "keyboard.h"
#include "game.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (r_init("Window", 800, 600) != 0) {
        fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }

    player_t player = { .pos = {0, 0, 10}, .angle = 0 };

    wall_t* walls1 = malloc(4 * sizeof(wall_t));
    walls1[0] = (wall_t){ .p1 = {-20, 20}, .p2 = {20, 20}, .color = 0xFFFF0000, .is_portal = false, .collidable = true };
    walls1[1] = (wall_t){ .p1 = {20, 20}, .p2 = {100, -100}, .color = 0xFF0000FF, .is_portal = true, .collidable = false };
    walls1[2] = (wall_t){ .p1 = {100, -100}, .p2 = {-100, -100}, .color = 0xFF0000FF, .is_portal = false, .collidable = true };
    walls1[3] = (wall_t){ .p1 = {-100, -100}, .p2 = {-20, 20}, .color = 0xFF0000FF, .is_portal = true, .collidable = false };
    sector_t sector1 = {
        .walls = walls1,
        .wall_count = 4,
        .floor_h = 0,
        .ceil_h = 20,
        .floor_color = 0xFFCCCCCC,
        .ceil_color = 0xFF999999
    };

    wall_t* walls2 = malloc(4 * sizeof(wall_t));
    walls2[0] = (wall_t){ .p1 = {-20, 40}, .p2 = {-20, 20}, .color = 0xFFFF0000, .is_portal = false, .collidable = true };
    walls2[1] = (wall_t){ .p1 = {-20, 20}, .p2 = {-100, -100}, .color = 0xFF0000FF, .is_portal = true, .collidable = false };
    walls2[2] = (wall_t){ .p1 = {-100, -100}, .p2 = {-100, 150}, .color = 0xFF00FFFF, .is_portal = false, .collidable = true };
    walls2[3] = (wall_t){ .p1 = {-100, 150}, .p2 = {-20, 40}, .color = 0xFF0000FF, .is_portal = true, .collidable = false };
    sector_t sector2 = {
        .walls = walls2,
        .wall_count = 4,
        .floor_h = 0,
        .ceil_h = 20,
        .floor_color = 0xFFCCCCCC,
        .ceil_color = 0xFF999999
    };

    wall_t* walls3 = malloc(5 * sizeof(wall_t));
    walls3[0] = (wall_t){ .p1 = {20, 40}, .p2 = {-20, 40}, .color = 0xFFFF0000, .is_portal = false, .collidable = true };
    walls3[1] = (wall_t){ .p1 = {-20, 40}, .p2 = {-100, 150}, .color = 0xFF0000FF, .is_portal = true, .collidable = false };
    walls3[2] = (wall_t){ .p1 = {-100, 150}, .p2 = {0, 200}, .color = 0xFFFF00FF, .is_portal = false, .collidable = true };
    walls3[3] = (wall_t){ .p1 = {0, 200}, .p2 = {100, 150}, .color = 0xFFFF00FF, .is_portal = false, .collidable = true };
    walls3[4] = (wall_t){ .p1 = {100, 150}, .p2 = {20, 40}, .color = 0xFF0000FF, .is_portal = true, .collidable = false };
    sector_t sector3 = {
        .walls = walls3,
        .wall_count = 5,
        .floor_h = 0,
        .ceil_h = 20,
        .floor_color = 0xFFCCCCCC,
        .ceil_color = 0xFF999999
    };

    wall_t* walls4 = malloc(4 * sizeof(wall_t));
    walls4[0] = (wall_t){ .p1 = {20, 20}, .p2 = {20, 40}, .color = 0xFFFF0000, .is_portal = false, .collidable = true };
    walls4[1] = (wall_t){ .p1 = {20, 40}, .p2 = {100, 150}, .color = 0xFF0000FF, .is_portal = true, .collidable = false };
    walls4[2] = (wall_t){ .p1 = {100, 150}, .p2 = {100, -100}, .color = 0xFFFFFFFF, .is_portal = false, .collidable = true };
    walls4[3] = (wall_t){ .p1 = {100, -100}, .p2 = {20, 20}, .color = 0xFF0000FF, .is_portal = true, .collidable = false };
    sector_t sector4 = {
        .walls = walls4,
        .wall_count = 4,
        .floor_h = 0,
        .ceil_h = 20,
        .floor_color = 0xFFCCCCCC,
        .ceil_color = 0xFF999999
    };

    walls1[1].portal_sector = &sector4;
    walls1[3].portal_sector = &sector2;
    walls2[1].portal_sector = &sector1;
    walls2[3].portal_sector = &sector3;
    walls3[1].portal_sector = &sector2;
    walls3[4].portal_sector = &sector4;
    walls4[1].portal_sector = &sector3;
    walls4[3].portal_sector = &sector1;

    player.current_sector = &sector1;

    g_game_loop(&player);

    free(walls1);
    free(walls2);
    free(walls3);
    free(walls4);

    r_destroy();
    return 0;
}