#include "sdlman.h"
#include <SDL/SDL_main.h> /* Para remapear a main () para portabilidade. */
#include <time.h>

// Todos as "Definicoes"

#define SDLMAN_FILENAME_LENGTH 16
#define SDLMAN_MAX_ENEMY_SPEED 9
#define SDLMAN_MAX_WORLD 5
#define SDLMAN_MENU_NUMBER_WIDTH 26
#define SDLMAN_MENU_NUMBER_HEIGHT 32

#define SDLMAN_HIGHSCORE_FILE "highscore.dat"


// static sdlman_draw_menu_number( desenhando o numero do menu)
static void sdlman_draw_menu_number(SDL_Surface *s, SDL_Surface *ns, int x, int y, int n)
{
    int i, pow, digits;
    SDL_Rect src, dst;

    src.w = dst.w = SDLMAN_MENU_NUMBER_WIDTH;
    src.h = dst.h = SDLMAN_MENU_NUMBER_HEIGHT;

    dst.x = x;
    dst.y = y;
    src.y = 0;

    /* Encontre dígitos em número.*/
    if (n > 0)
    {
        digits = 0;
        i = n;
        while (i != 0)
        {
            i = i / 10;
            digits++;
        }
    }
    else
    {
        digits = 1;
        n = 0; /* Força para zero se for negativo. */
    }

    while (digits > 0)
    {
        pow = 1;
        for (i = 0; i < digits - 1; i++)
            pow *= 10;

    switch ((n / pow) % 10)
    {
        case 1:
          src.x = 0;
          break;
        case 2:
          src.x = SDLMAN_MENU_NUMBER_WIDTH;
          break;
        case 3:
          src.x = SDLMAN_MENU_NUMBER_WIDTH * 2;
          break;
        case 4:
          src.x = SDLMAN_MENU_NUMBER_WIDTH * 3;
          break;
        case 5:
          src.x = SDLMAN_MENU_NUMBER_WIDTH * 4;
          break;
        case 6:
          src.x = SDLMAN_MENU_NUMBER_WIDTH * 5;
          break;
        case 7:
          src.x = SDLMAN_MENU_NUMBER_WIDTH * 6;
          break;
        case 8:
          src.x = SDLMAN_MENU_NUMBER_WIDTH * 7;
          break;
        case 9:
          src.x = SDLMAN_MENU_NUMBER_WIDTH * 8;
          break;
        case 0:
          src.x = SDLMAN_MENU_NUMBER_WIDTH * 9;
          break;
        default:
          return; /* Não encontrado, apenas retorne. */
    }

    SDL_BlitSurface(ns, &src, s, &dst);

    digits--;
    dst.x += SDLMAN_MENU_NUMBER_WIDTH;
  }
}



static void sdlman_load_highscore(int *table, char *filename)
{
  int i;
  FILE *fh;

  fh = fopen(filename, "r");
  if (fh == NULL) {
    fprintf(stderr, "Aviso: não é possível carregar o arquivo de recordes.\n");
    /* Define todos os recordes para zero.*/
    for (i = 0; i < SDLMAN_MAX_WORLD; i++) {
      table[i] = 0;
    }
  } else {
    fread(table, sizeof(int), SDLMAN_MAX_WORLD, fh);
    fclose(fh);
  }
}



static void sdlman_save_highscore(int *table, char *filename)
{
  FILE *fh;

  fh = fopen(filename, "w");
  if (fh == NULL) {
    fprintf(stderr, "Aviso: Não é possível salvar o arquivo de recordes.\n");
  } else {
    fwrite(table, sizeof(int), SDLMAN_MAX_WORLD, fh);
    fclose(fh);
  }
}



int main(int argc, char *argv[])
{
  SDL_Event event;
  SDL_Surface *screen, *menu_surface, *number_surface, *temp_surface;
  int game_done, game_result, world_number, enemy_speed, game_score;
  char layout_file[SDLMAN_FILENAME_LENGTH];
  char graphic_file[SDLMAN_FILENAME_LENGTH];
  int high_score[SDLMAN_MAX_WORLD];

  /* Use srand () em vez de srandom () */
  srand((unsigned)time(NULL));

  /* Tente carregar recordes do arquivo. */
  sdlman_load_highscore(high_score, SDLMAN_HIGHSCORE_FILE);

  /* Vídeo e inicialização de tela aqui, por causa do menu gráfico. */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    fprintf(stderr, "Erro: não é possível inicializar o SDL: %s\n", SDL_GetError());
    return 1;
  }
  atexit(SDL_Quit);
  screen = SDL_SetVideoMode(SDLMAN_WORLD_X_SIZE * SDLMAN_BLOCK_SIZE,SDLMAN_WORLD_Y_SIZE * SDLMAN_BLOCK_SIZE, 24, SDL_DOUBLEBUF);
  if (screen == NULL) {
    fprintf(stderr, "Erro: não é possível definir o modo de vídeo: %s\n", SDL_GetError());
    return 1;
  }
  SDL_WM_SetCaption("SDL-Man", "SDL-Man");


  /* Carregar e converter arquivos gráficos de menu. */
  temp_surface = SDL_LoadBMP("menu.bmp");
  if (temp_surface == NULL)
{
    fprintf(stderr, "Erro: não é possível carregar os gráficos do menu: %s\n",SDL_GetError());
    return 1;
  } else {
    menu_surface = SDL_DisplayFormat(temp_surface);
    if (menu_surface == NULL) {
      fprintf(stderr, "Erro: Não é possível converter os gráficos do menu: %s\n",
        SDL_GetError());
      return 1;
    }
    SDL_FreeSurface(temp_surface);
  }

  temp_surface = SDL_LoadBMP("number.bmp");
  if (temp_surface == NULL) {
    fprintf(stderr, "Erro: não é possível carregar gráficos numéricos: %s\n",
      SDL_GetError());
    SDL_FreeSurface(menu_surface);
    return 1;
  } else {
    number_surface = SDL_DisplayFormat(temp_surface);
    if (number_surface == NULL) {
      fprintf(stderr, "Erro: Não é possível converter gráficos inimigos: %s\n",
        SDL_GetError());
      SDL_FreeSurface(menu_surface);
      return 1;
    }
    SDL_FreeSurface(temp_surface);
  }


  /* valores padrão. */
  enemy_speed = 5;
  world_number = 1;


  /* Loop do menu principal. */
  game_done = 0;
  while (! game_done) {

    /* processo para entrada do usuário. */
    if (SDL_PollEvent(&event) == 1) {
      switch(event.type) {
      case SDL_QUIT:
        game_done = 1;
        break;

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_q:
          game_done = 1;
          break;

        case SDLK_RETURN:
        case SDLK_SPACE:
          snprintf(layout_file, SDLMAN_FILENAME_LENGTH, "world%d.layout",
            world_number);
          snprintf(graphic_file, SDLMAN_FILENAME_LENGTH, "world%d.bmp",
            world_number);

          game_result = sdlman_gameloop(screen, layout_file, graphic_file,
            enemy_speed, &game_score);

          if (game_result == SDLMAN_GAMELOOP_OK) {
            /* Atualizar pontuação  */
            if (game_score > high_score[world_number - 1])
              high_score[world_number - 1] = game_score;
          } else {
            game_done = 1; /* falhou . */
          }
          break;

        case SDLK_w:
          world_number++;
          if (world_number > SDLMAN_MAX_WORLD)
            world_number = 1;
          break;

        case SDLK_s:
          enemy_speed++;
          if (enemy_speed > SDLMAN_MAX_ENEMY_SPEED)
            enemy_speed = 1;
          break;

        default:
          break;
        }
        break;

      default:
        continue;
      }
    }


    /* Desenhe gráficos. */
    SDL_BlitSurface(menu_surface, NULL, screen, NULL);
    sdlman_draw_menu_number(screen, number_surface, 385, 163, world_number);
    sdlman_draw_menu_number(screen, number_surface, 385, 211, enemy_speed);
    sdlman_draw_menu_number(screen, number_surface, 385, 259,
      high_score[world_number - 1]);

    SDL_Flip(screen);
    SDL_Delay(SDLMAN_GAME_SPEED);
  }


  /* Limpar */
  SDL_FreeSurface(menu_surface);
  SDL_FreeSurface(number_surface);
  SDL_FreeSurface(screen);
  sdlman_save_highscore(high_score, SDLMAN_HIGHSCORE_FILE);

  return 0;
}

