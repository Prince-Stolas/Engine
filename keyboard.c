#include <SDL.h>

#include "keyboard.h"

keys_t keys;

void k_update() {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    keys.w = state[SDL_SCANCODE_W];
    keys.a = state[SDL_SCANCODE_A];
    keys.s = state[SDL_SCANCODE_S];
    keys.d = state[SDL_SCANCODE_D];
    keys.q = state[SDL_SCANCODE_Q];
    keys.e = state[SDL_SCANCODE_E];
    keys.r = state[SDL_SCANCODE_R];
    keys.f = state[SDL_SCANCODE_F];
    keys.t = state[SDL_SCANCODE_T];
    keys.g = state[SDL_SCANCODE_G];
}