#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>

#include "render.h"
#include "typedef.h"
#include "util.h"

uint32_t* pixels;
SDL_Texture* freamebuffer;
SDL_Renderer* renderer;
SDL_Window* window;

int r_init(char* title, int window_w, int window_h) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_w, window_h, SDL_WINDOW_SHOWN);

    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        fprintf(stderr, "Failed to create renderer\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_RenderSetLogicalSize(renderer, FB_WIDTH, FB_HEIGHT);

    freamebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FB_WIDTH, FB_HEIGHT);

    if (!freamebuffer) {
        fprintf(stderr, "Failed to create texture\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int fb_size = FB_WIDTH * FB_HEIGHT;
    pixels = malloc(fb_size * sizeof(uint32_t));

    memset(pixels, 0, fb_size * sizeof(uint32_t));

    return 0;
}

void r_update() {
    SDL_UpdateTexture(freamebuffer, NULL, pixels, FB_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, freamebuffer, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void r_destroy() {
    SDL_DestroyTexture(freamebuffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(pixels);
}

void r_clear_buffer() {
    memset(pixels, 0, FB_WIDTH * FB_HEIGHT * sizeof(uint32_t));
}

void r_draw_point(int x, int y, uint32_t color) {
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) {
        return;
    }
    pixels[y * FB_WIDTH + x] = color;
}

vec3_t r_transform_c(vec3_t wp, vec3_t camera_pos, float angle) {
    float dx, dy, cx, cy, cz;

    dx = wp.x - camera_pos.x;
    dy = wp.y - camera_pos.y;

    cx = dx * cos(angle) - dy * sin(angle);
    cy = dx * sin(angle) + dy * cos(angle);
    cz = wp.z - camera_pos.z;

    return (vec3_t){.x = cx, .y = cy, .z = cz};
}

vec2_t r_transform_s(vec3_t cp, float fov, float sy_offs) {
    float sx, sy;

    float rfov = fov * M_PI / 180;
    float f = (FB_WIDTH / 2) / tanf(rfov / 2);

    sx = f*cp.x / cp.y + FB_WIDTH / 2;
    sy = f*(-cp.z) / cp.y + FB_HEIGHT / 2 + sy_offs;

    return (vec2_t){.x = sx, .y = sy};
}

void r_draw_vert_line(int x, int y1, int y2, uint32_t color) {
    if (x < 0 || x >= FB_WIDTH) {
        return;
    }

    if (y1 > y2) {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    for (int y = MAX(0, y1); y <= MIN(FB_HEIGHT - 1, y2); y++) {
        pixels[y * FB_WIDTH + x] = color;
    }
}

bool r_is_onscreen(vec2_t sp) {
    return sp.y > 0 && sp.y < FB_HEIGHT && sp.x < FB_WIDTH && sp.x > 0;
}

vec3_t r_clip_to_near(vec3_t cp1, vec3_t cp2, float near) {
    float t = (near - cp1.y) / (cp2.y - cp1.y);
    return (vec3_t){
        .x = cp1.x + t * (cp2.x - cp1.x),
        .y = near,
        .z = cp1.z + t * (cp2.z - cp1.z)
    };
}

void r_draw_wall(const camera_t* camera, const wall_t* wall, const sector_t* from_sect, const sector_t* current_sect, const clip_mask_t* clip_mask) {
    float near = 0.01;

    vec2_t vec_wp1_wp2 = { wall->p2.x - wall->p1.x, wall->p2.y - wall->p1.y };
    vec2_t vec_wp1_camera = { camera->pos.x - wall->p1.x, camera->pos.y - wall->p1.y };
    
    if (cross2d(vec_wp1_wp2, vec_wp1_camera) > 0) return;

    vec3_t ctl = r_transform_c((vec3_t){.x = wall->p1.x, .y = wall->p1.y, .z = current_sect->ceil_h}, camera->pos, camera->angle);
    vec3_t ctr = r_transform_c((vec3_t){.x = wall->p2.x, .y = wall->p2.y, .z = current_sect->ceil_h}, camera->pos, camera->angle);
    vec3_t cbl = r_transform_c((vec3_t){.x = wall->p1.x, .y = wall->p1.y, .z = current_sect->floor_h}, camera->pos, camera->angle);
    vec3_t cbr = r_transform_c((vec3_t){.x = wall->p2.x, .y = wall->p2.y, .z = current_sect->floor_h}, camera->pos, camera->angle);

    vec3_t cptl, cptr, cpbl, cpbr;
    vec2_t sptl, sptr, spbl, spbr;
    bool pwallb = false, pwallt = false;
    clip_mask_t* new_clip_mask;
    if (wall->is_portal) {
        new_clip_mask = malloc(sizeof(clip_mask_t));
        new_clip_mask->top = malloc(FB_WIDTH * sizeof(int));
        new_clip_mask->bottom = malloc(FB_WIDTH * sizeof(int));
        new_clip_mask->start_x = clip_mask->start_x;
        new_clip_mask->end_x = clip_mask->end_x;
        memcpy(new_clip_mask->top, clip_mask->top, FB_WIDTH * sizeof(int));
        memcpy(new_clip_mask->bottom, clip_mask->bottom, FB_WIDTH * sizeof(int));

        if (current_sect->ceil_h > wall->portal_sector->ceil_h) {
            pwallt = true;

            //cptl = r_transform_c((vec3_t){.x = wall->p1.x, .y = wall->p1.y, .z = wall->portal_sector->ceil_h}, camera.pos, camera.angle, 90);
            //cptr = r_transform_c((vec3_t){.x = wall->p2.x, .y = wall->p2.y, .z = wall->portal_sector->ceil_h}, camera.pos, camera.angle, 90);

            cptl = ctl; cptl.z = wall->portal_sector->ceil_h - camera->pos.z;
            cptr = ctr; cptr.z = wall->portal_sector->ceil_h - camera->pos.z;

            if (cptl.y < near) cptl = r_clip_to_near(cptl, cptr, near);
            if (cptr.y < near) cptr = r_clip_to_near(cptr, cptl, near);

            sptl = r_transform_s(cptl, 90, camera->sy_offs);
            sptr = r_transform_s(cptr, 90, camera->sy_offs);
        }
        if (current_sect->floor_h < wall->portal_sector->floor_h) {
            pwallb = true;
            //cpbl = r_transform_c((vec3_t){.x = wall->p1.x, .y = wall->p1.y, .z = wall->portal_sector->floor_h}, camera.pos, camera.angle, 90);
            //cpbr = r_transform_c((vec3_t){.x = wall->p2.x, .y = wall->p2.y, .z = wall->portal_sector->floor_h}, camera.pos, camera.angle, 90);
            cpbl = cbl; cpbl.z = wall->portal_sector->floor_h - camera->pos.z;
            cpbr = cbr; cpbr.z = wall->portal_sector->floor_h - camera->pos.z;
            if (cpbl.y < near) cpbl = r_clip_to_near(cpbl, cpbr, near);
            if (cpbr.y < near) cpbr = r_clip_to_near(cpbr, cpbl, near);

            spbl = r_transform_s(cpbl, 90, camera->sy_offs);
            spbr = r_transform_s(cpbr, 90, camera->sy_offs);
        }
    }

    if (cbl.y <= near && cbr.y <= near && ctl.y <= near && ctr.y <= near) {
        return;
    }

    if (ctl.y < near) ctl = r_clip_to_near(ctl, ctr, near);
    if (ctr.y < near) ctr = r_clip_to_near(ctr, ctl, near);
    if (cbl.y < near) cbl = r_clip_to_near(cbl, cbr, near);
    if (cbr.y < near) cbr = r_clip_to_near(cbr, cbl, near);

    vec2_t stl = r_transform_s(ctl, 90, camera->sy_offs);
    vec2_t str = r_transform_s(ctr, 90, camera->sy_offs);
    vec2_t sbl = r_transform_s(cbl, 90, camera->sy_offs);
    vec2_t sbr = r_transform_s(cbr, 90, camera->sy_offs);

    if (wall->is_portal) {
        if (!pwallt) {
            sptl = stl;
            sptr = str;
        }
        if (!pwallb) {
            spbl = sbl;
            spbr = sbr;
        }
    }

    /*
    if (sbl.x > sbr.x) {
        vec2_t temp = sbl;
        sbl = sbr;
        sbr = temp;
    }

    if (stl.x > str.x) {
        vec2_t temp = stl;
        stl = str;
        str = temp;
    }
    */

    float dx = sbr.x - sbl.x;
    float dty = str.y - stl.y;
    float dby = sbr.y - sbl.y;

    float port_dx = 0, port_dty, port_dby;
    if (wall->is_portal) {
        if (pwallt) {
            port_dx = sptr.x - sptl.x;
            port_dty = sptr.y - sptl.y;
        }
        
        if (pwallb) {
            if (!port_dx) port_dx = spbr.x - spbl.x;
            port_dby = spbr.y - spbl.y;
        }
    }

    int start_x, end_x;
    //if (clip_mask) {
        if (sbl.x <= clip_mask->start_x) start_x = clip_mask->start_x;
        else {
            start_x = sbl.x;
            if (wall->is_portal) new_clip_mask->start_x = sbl.x;
        }
        if (sbr.x > clip_mask->end_x) end_x = clip_mask->end_x;
        else {
            end_x = sbr.x;
            if (wall->is_portal) new_clip_mask->end_x = sbr.x;
        }
    /*} else {
        start_x = sbl.x;
        end_x = sbr.x;
    }*/

    for (int x = start_x; x <= end_x; x++) {
        int ty = (dty/dx) * (x-stl.x) + stl.y;
        int by = (dby/dx) * (x-sbl.x) + sbl.y;

        int top, bottom;
        bool skip_wall = false;
        //if (clip_mask) {
            int clip_ty = clip_mask->top[x];
            int clip_by = clip_mask->bottom[x];

            if (ty > clip_by || by < clip_ty) skip_wall = true;

            if (ty < clip_ty) ty = clip_ty;
            if (by > clip_by) by = clip_by;

            //----------

                // Debug: draw clip mask
                //draw_vert_line(x, clip_ty, clip_by, 0xFFFF0000);
                //draw_point(x, ty, 0xFF00FF00);
                //draw_point(x, by, 0xFF0000FF);
                //
            
            top = clip_ty;
            bottom = clip_by;
        /*} else {
            top = 0;
            bottom = FB_HEIGHT - 1;
        }*/

        if (top < ty) {
            if (skip_wall) r_draw_vert_line(x, top, bottom, current_sect->ceil_color);
            else r_draw_vert_line(x, top, ty-1, current_sect->ceil_color);
        }
        if (bottom > by) {
            if (skip_wall) r_draw_vert_line(x, top, bottom, current_sect->floor_color);
            else r_draw_vert_line(x, by+1, bottom, current_sect->floor_color);
        }

        //printf("2\n");

        if (wall->is_portal) {
            if (ty > clip_mask->top[x]) new_clip_mask->top[x] = ty;
            if (by < clip_mask->bottom[x]) new_clip_mask->bottom[x] = by;
        }


        if (skip_wall) continue;
        if (wall->is_portal) {
            if (pwallt) {
                int port_ty = (port_dty/port_dx) * (x-sptl.x) + sptl.y;
                r_draw_vert_line(x, ty, port_ty, wall->color);
            }

            if (pwallb) {
                int port_by = (port_dby/port_dx) * (x-spbl.x) + spbl.y;
                r_draw_vert_line(x, port_by, by, wall->color);
            }
        } else {
            r_draw_vert_line(x, ty, by, wall->color);
        }
    }

    if (wall->is_portal && from_sect != wall->portal_sector) {
        r_draw_sector(camera, wall->portal_sector, current_sect, new_clip_mask);

        free(new_clip_mask->top);
        free(new_clip_mask->bottom);
        free(new_clip_mask);
    }
}

void r_draw_sector(const camera_t* camera, const sector_t* sector, const sector_t* from_sect, const clip_mask_t* clip_mask) {
    for (int i = 0; i < sector->wall_count; i++) {
        r_draw_wall(camera, &sector->walls[i], from_sect, sector, clip_mask);
    }
}
