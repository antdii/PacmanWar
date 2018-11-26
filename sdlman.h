#ifndef _SDLMAN_H
#define _SDLMAN_H

/* padrao de bibliotecas*/
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

/* definicoes */
#define SDLMAN_BLOCK_SIZE 32
#define SDLMAN_WORLD_X_SIZE 20
#define SDLMAN_WORLD_Y_SIZE 15

#define SDLMAN_GAMELOOP_OK 0
#define SDLMAN_GAMELOOP_FAIL -1
#define SDLMAN_GAMELOOP_QUIT -2

#define SDLMAN_GAME_SPEED 20

/* Protótipo para gameloop.*/
int sdlman_gameloop(SDL_Surface *screen, char *world_layout_file,
  char *world_graphic_file, int enemy_speed, int *score);

#endif /* _SDLMAN_H */
