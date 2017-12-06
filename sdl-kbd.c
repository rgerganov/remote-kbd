#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fatal("SDL_Init Error: %s", SDL_GetError());
    }
    SDL_Window *window = SDL_CreateWindow("sdl-kbd", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fatal("CreateWindow Error: %s", SDL_GetError());
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fatal("CreateRenderer Error: %s", SDL_GetError());
    }

    SDL_Event e;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYDOWN) {
                SDL_SetRenderDrawColor(renderer, 0xdc, 0x0c, 0x0c, 0xFF);
                SDL_RenderClear(renderer);
                printf("scancode: %d keycode: %d\n", e.key.keysym.scancode, e.key.keysym.sym);
            } else if (e.type == SDL_KEYUP) {
                SDL_SetRenderDrawColor(renderer, 0xdc, 0xdc, 0xdc, 0xFF);
                SDL_RenderClear(renderer);
            }
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
