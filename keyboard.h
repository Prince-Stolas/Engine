#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef struct {
    int w, a, s, d, q, e, r, f, t, g;
} keys_t;

extern keys_t keys;

void k_update();

#endif