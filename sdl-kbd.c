#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "common.h"

static int send_key(int sock, int code, int value)
{
    struct input_event ev = {
        .type = EV_KEY,
        .code = code,
        .value = value
    };
    print_event(&ev);
    return send_event(sock, &ev);
}

static int send_syn(int sock)
{
    struct input_event ev = {
        .type = EV_SYN,
        .code = SYN_REPORT,
        .value = 0
    };
    print_event(&ev);
    return send_event(sock, &ev);
}

static int translate(int scancode)
{
    switch (scancode) {
        case SDL_SCANCODE_LEFT:
            return KEY_LEFT;
        case SDL_SCANCODE_RIGHT:
            return KEY_RIGHT;
        default:
            return -1;
    }
}

int main(int argc, char *argv[])
{
    char *host;
    int port;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <IP>:<port>\n", argv[0]);
        return 1;
    }
    parse_host_port(argv[1], &host, &port);
    signal(SIGPIPE, SIG_IGN);
    int client_sock = connect_socket(host, port);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fatal("SDL_Init Error: %s", SDL_GetError());
    }
    char title[1024];
    snprintf(title, 1024, "Sending events to %s:%d", host, port);
    SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
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
                int code = translate(e.key.keysym.scancode);
                if (code < 0) {
                    continue;
                }
                if (send_key(client_sock, code, 1) < 0) {
                    fprintf(stderr, "error on send\n");
                }
                if (send_syn(client_sock) < 0) {
                    fprintf(stderr, "error on send\n");
                }
                if (send_key(client_sock, code, 0) < 0) {
                    fprintf(stderr, "error on send\n");
                }
                if (send_syn(client_sock) < 0) {
                    fprintf(stderr, "error on send\n");
                }
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
