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
    //printf(">> scancode: %d\n", scancode);
    if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0) {
        return KEY_1 + (scancode - SDL_SCANCODE_1);
    }
    if (scancode >= SDL_SCANCODE_F1 && scancode <= SDL_SCANCODE_F10) {
        return KEY_F1 + (scancode - SDL_SCANCODE_F1);
    }

    switch (scancode) {
        case SDL_SCANCODE_A:
            return KEY_A;
        case SDL_SCANCODE_B:
            return KEY_B;
        case SDL_SCANCODE_C:
            return KEY_C;
        case SDL_SCANCODE_D:
            return KEY_D;
        case SDL_SCANCODE_E:
            return KEY_E;
        case SDL_SCANCODE_F:
            return KEY_F;
        case SDL_SCANCODE_G:
            return KEY_G;
        case SDL_SCANCODE_H:
            return KEY_H;
        case SDL_SCANCODE_I:
            return KEY_I;
        case SDL_SCANCODE_J:
            return KEY_J;
        case SDL_SCANCODE_K:
            return KEY_K;
        case SDL_SCANCODE_L:
            return KEY_L;
        case SDL_SCANCODE_M:
            return KEY_M;
        case SDL_SCANCODE_N:
            return KEY_N;
        case SDL_SCANCODE_O:
            return KEY_O;
        case SDL_SCANCODE_P:
            return KEY_P;
        case SDL_SCANCODE_Q:
            return KEY_Q;
        case SDL_SCANCODE_R:
            return KEY_R;
        case SDL_SCANCODE_S:
            return KEY_S;
        case SDL_SCANCODE_T:
            return KEY_T;
        case SDL_SCANCODE_U:
            return KEY_U;
        case SDL_SCANCODE_V:
            return KEY_V;
        case SDL_SCANCODE_W:
            return KEY_W;
        case SDL_SCANCODE_X:
            return KEY_X;
        case SDL_SCANCODE_Y:
            return KEY_Y;
        case SDL_SCANCODE_Z:
            return KEY_Z;
        case SDL_SCANCODE_LEFT:
            return KEY_LEFT;
        case SDL_SCANCODE_RIGHT:
            return KEY_RIGHT;
        case SDL_SCANCODE_UP:
            return KEY_UP;
        case SDL_SCANCODE_DOWN:
            return KEY_DOWN;
        case SDL_SCANCODE_RETURN:
            return KEY_ENTER;
        case SDL_SCANCODE_ESCAPE:
            return KEY_ESC;
        case SDL_SCANCODE_BACKSPACE:
            return KEY_BACKSPACE;
        case SDL_SCANCODE_PAGEUP:
            return KEY_PAGEUP;
        case SDL_SCANCODE_PAGEDOWN:
            return KEY_PAGEDOWN;
        case SDL_SCANCODE_F11:
            return KEY_F11;
        case SDL_SCANCODE_F12:
            return KEY_F12;
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
            } else if (e.type == SDL_KEYUP) {
                SDL_SetRenderDrawColor(renderer, 0xdc, 0xdc, 0xdc, 0xFF);
                SDL_RenderClear(renderer);
                int code = translate(e.key.keysym.scancode);
                if (code < 0) {
                    continue;
                }
                if (send_key(client_sock, code, 0) < 0) {
                    fprintf(stderr, "error on send\n");
                }
                if (send_syn(client_sock) < 0) {
                    fprintf(stderr, "error on send\n");
                }
            }
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
