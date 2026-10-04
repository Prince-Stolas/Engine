#ifndef RENDER_H
#define RENDER_H

#define FB_WIDTH 400
#define FB_HEIGHT 300

#include <stdint.h>
#include <stdbool.h>

#include "typedef.h"

typedef struct sector sector_t;

typedef struct wall {
    vec2_t p1, p2;
    uint32_t color;
    bool is_portal;
    sector_t* portal_sector;
    bool collidable;
} wall_t;

typedef struct camera {
    vec3_t pos;
    float angle;
    float sy_offs;
} camera_t;

typedef struct sector {
    wall_t* walls;
    int wall_count;
    int floor_h;
    int ceil_h;
    uint32_t floor_color;
    uint32_t ceil_color;
} sector_t;

typedef struct clip_mask {
    int *top, *bottom;
    int start_x, end_x;
} clip_mask_t;

int r_init(char* title, int window_w, int window_h);
void r_update();
void r_destroy();
void r_clear_buffer();

void r_draw_point(int x, int y, uint32_t color);
void r_draw_wall(const camera_t* camera, const wall_t* wall, const sector_t* from_sect, const sector_t* current_sect, const clip_mask_t* clip_mask);
void r_draw_sector(const camera_t* camera, const sector_t* sector, const sector_t* from_sect, const clip_mask_t* clip_mask);
vec3_t r_transform_c(vec3_t wp, vec3_t camera_pos, float angle);
vec2_t r_transform_s(vec3_t wp, float fov, float sy_offs);
void r_draw_vert_line(int x, int y1, int y2, uint32_t color);

#endif