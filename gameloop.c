// Pac Man
#include "sdlman.h"
#include <SDL/SDL_mixer.h> /* For audio. */



// Todos as "Definicoes"

#define SDLMAN_MAX_PLAYER_SPEED 5
#define SDLMAN_CHARACTER_SIZE 26
#define SDLMAN_ENEMY_COUNT 4
#define SDLMAN_PELLET_SIZE 4
#define SDLMAN_BOOSTER_SIZE 6
#define SDLMAN_MAX_PELLET 300 /* 15 x 20 */
#define SDLMAN_BOOSTER_TIME 120 /* Em ciclos de loop */

#define SDLMAN_SCORE_PELLET 2
#define SDLMAN_SCORE_FINISH 100
#define SDLMAN_SCORE_ENEMY -25 /* Penalidade por matar inimigos*/

#define SDLMAN_WORLD_AIR ' '
#define SDLMAN_WORLD_WALL '#'
#define SDLMAN_WORLD_PLAYER 'P'
#define SDLMAN_WORLD_ENEMY 'E'
#define SDLMAN_WORLD_PELLET '.'
#define SDLMAN_WORLD_BOOSTER '*'


// enum

enum {
  SDLMAN_DIRECTION_NONE  = 0,
  SDLMAN_DIRECTION_UP    = 1,
  SDLMAN_DIRECTION_DOWN  = 2,
  SDLMAN_DIRECTION_LEFT  = 3,
  SDLMAN_DIRECTION_RIGHT = 4,
};


// struct sdlman_character_s

typedef struct sdlman_character_s {
  int x, y; /* Coordenadas mundiais */
  int moving_direction, looking_direction;
  int speed;
  int draw_count; /* USado para animacao  */
  int killed;
} sdlman_character_t;

// struct sdlman_pellet_s

typedef struct sdlman_pellet_s {
  int x, y;
  int consumed;
  int boost_effect;
} sdlman_pellet_t;



// static sdlman_init_sound(iniciando o som)

static int sdlman_init_sound(Mix_Music **music, Mix_Chunk **chomp)
{
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
     fprintf(stderr, "Erro nao e possivel abrir o audio %s\n", SDL_GetError());
     Mix_CloseAudio();
     return -1;
  }

  // Testando o audio

  if (! (*music = Mix_LoadMUS("music.wav"))) {
     fprintf(stderr, "Erro nao foi possivel abrir o audio: %s\n",
       SDL_GetError());
     Mix_CloseAudio();
     return -1;
  }

  // testando arquivo de som

  if (! (*chomp = Mix_LoadWAV("chomp.wav"))) {
     fprintf(stderr, "Erro nao foi possivel abrir o audio: %s\n",
       SDL_GetError());
     Mix_CloseAudio();
     Mix_FreeMusic(*music);
     return -1;
  }

  return 0;
}


// static sdlman_play_music (ativando musica)

static void sdlman_play_music(Mix_Music *m)
{
  if (Mix_PlayMusic(m, 0) == -1)
    fprintf(stderr, "Nao e possivel reproduzir o audio %s\n", SDL_GetError());
}


// static sdlman_play_sound(ativando audio)

static void sdlman_play_sound(Mix_Chunk *s)
{
  /*pare todos os canais para evitar usa-los*/
  Mix_HaltChannel(-1);

  if (Mix_PlayChannel(-1, s, 0) == -1)
    fprintf(stderr, "Aviso :  nao e possivel reproduzir o som %s\n", SDL_GetError());
}


// static sdlman_load_world (escrever dentro do arquivo)

static int sdlman_load_world(char *filename, char *world)
{
  int c, w, h;
  FILE *fh;

  fh = fopen(filename, "r");
  if (fh == NULL) {
    fprintf(stderr, "Erro nao e possivel abrir o  '%s' para leitura \n", filename);
    return -1;
  }

  w = 0;
  h = 0;
  while ((c = fgetc(fh)) != EOF) {
    if (c == '\n' || w >= SDLMAN_WORLD_X_SIZE) {
      w = 0;
      h++;
      if (h >= SDLMAN_WORLD_Y_SIZE)
        return 0; /* limite alcanssado, Resgate */
    } else {
      world[(h * SDLMAN_WORLD_X_SIZE) + w] = c;
      w++;
    }
  }

  fclose(fh);
  return 0;
}


// static sdlman_locate_player(localizando jogador)

static int sdlman_locate_player(char *world, int *x, int *y)
{
  int i, j;
  for (i = 0; i < SDLMAN_WORLD_Y_SIZE; i++) {
    for (j = 0; j < SDLMAN_WORLD_X_SIZE; j++) {
      if (world[(i * SDLMAN_WORLD_X_SIZE) + j] == SDLMAN_WORLD_PLAYER) {
        *x = j * SDLMAN_BLOCK_SIZE;
        *y = i * SDLMAN_BLOCK_SIZE;
        return 0;
      }
    }
  }
  return -1; /* nao encontrado */
}


// static sdlman_locate_enemy (localizando o inimigo)

static int sdlman_locate_enemy(char *world, int *x, int *y, int n)
{
  int i, j, count;
  count = 0;
  for (i = 0; i < SDLMAN_WORLD_Y_SIZE; i++) {
    for (j = 0; j < SDLMAN_WORLD_X_SIZE; j++) {
      if (world[(i * SDLMAN_WORLD_X_SIZE) + j] == SDLMAN_WORLD_ENEMY) {
        if (count == n) {
          *x = j * SDLMAN_BLOCK_SIZE;
          *y = i * SDLMAN_BLOCK_SIZE;
          return 0;
        }
        count++;
      }
    }
  }
  return -1; /* nao encontrado */
}

// static sdlman_init_character(identificando caracter)

static void sdlman_init_character(sdlman_character_t *c, int x, int y)
{
  /* starting X and Y co-ordinates is dependent on world data. */
  c->x = x;
  c->y = y;
  c->moving_direction = c->looking_direction = SDLMAN_DIRECTION_NONE;
  c->speed = 0;
  c->draw_count = 0;
  c->killed = 0;
}


//  static sdlman_init_pellets(bolinha de inicializacao)

static void sdlman_init_pellets(char *world, sdlman_pellet_t *p, int *total)
{
  int i, j, n;
  n = 0;
  for (i = 0; i < SDLMAN_WORLD_Y_SIZE; i++) {
    for (j = 0; j < SDLMAN_WORLD_X_SIZE; j++) {
      if ((world[(i * SDLMAN_WORLD_X_SIZE) + j] == SDLMAN_WORLD_PELLET) ||
          (world[(i * SDLMAN_WORLD_X_SIZE) + j] == SDLMAN_WORLD_BOOSTER)) {
        p[n].x = (j * SDLMAN_BLOCK_SIZE) + (SDLMAN_BLOCK_SIZE / 2);
        p[n].y = (i * SDLMAN_BLOCK_SIZE) + (SDLMAN_BLOCK_SIZE / 2);
        p[n].consumed = 0;

        if (world[(i * SDLMAN_WORLD_X_SIZE) + j] == SDLMAN_WORLD_BOOSTER)
          p[n].boost_effect = 1;
        else
          p[n].boost_effect = 0;

        n++;
        if (n >= SDLMAN_MAX_PELLET - 1)
          break;
      }
    }
  }
  *total = n;
}

// static sdlman_draw_world_basic(criando o mapa basico)

static void sdlman_draw_world_basic(SDL_Surface *s, char *world)
{
  int i, j;
  SDL_Rect r;

  r.x = 0;
  r.y = 0;
  r.w = SDLMAN_WORLD_X_SIZE * SDLMAN_BLOCK_SIZE;
  r.h = SDLMAN_WORLD_Y_SIZE * SDLMAN_BLOCK_SIZE;
  SDL_FillRect(s, &r, 0x0);

  r.w = SDLMAN_BLOCK_SIZE;
  r.h = SDLMAN_BLOCK_SIZE;
  for (i = 0; i < SDLMAN_WORLD_Y_SIZE; i++) {
    for (j = 0; j < SDLMAN_WORLD_X_SIZE; j++) {
      if (world[(i * SDLMAN_WORLD_X_SIZE) + j] == SDLMAN_WORLD_WALL) {
        r.x = j * SDLMAN_BLOCK_SIZE;
        r.y = i * SDLMAN_BLOCK_SIZE;
        SDL_FillRect(s, &r, 0xffffff);
      }
    }
  }
}


//static sdlman_draw_world_bitmap(criando mapa do bitmap)

static void sdlman_draw_world_bitmap(SDL_Surface *s, SDL_Surface *ws)
{
  SDL_BlitSurface(ws, NULL, s, NULL);
}

// static sdlman_draw_player(jogador de empate)

static void sdlman_draw_player(sdlman_character_t *p,
  SDL_Surface *s, SDL_Surface *ps, int boosted)
{
  SDL_Rect src, dst;
  int direction;

  src.w = src.h = dst.w = dst.h = SDLMAN_CHARACTER_SIZE;
  dst.x = p->x;
  dst.y = p->y;

  if (boosted)
    src.y = SDLMAN_CHARACTER_SIZE;
  else
    src.y = 0;

  src.x = 0;

  if (p->moving_direction == SDLMAN_DIRECTION_NONE)
    direction = p->looking_direction;
  else
    direction = p->moving_direction;

  switch (direction) {
  case SDLMAN_DIRECTION_UP:
    /* Nada para adicionar*/
    break;
  case SDLMAN_DIRECTION_DOWN:
    src.x += (SDLMAN_CHARACTER_SIZE * 4);
    break;

  case SDLMAN_DIRECTION_LEFT:
    src.x += (SDLMAN_CHARACTER_SIZE * 8);
    break;

  case SDLMAN_DIRECTION_RIGHT:
    src.x += (SDLMAN_CHARACTER_SIZE * 12);
    break;

  default:
    break;
  }

  if (p->draw_count < 4) {
    src.x += (p->draw_count * SDLMAN_CHARACTER_SIZE);
    p->draw_count++;
  } else if (p->draw_count == 4) {
    src.x += (SDLMAN_CHARACTER_SIZE * 3);
    p->draw_count++;
  } else if (p->draw_count == 5) {
    src.x += (SDLMAN_CHARACTER_SIZE * 2);
    p->draw_count++;
  } else if (p->draw_count == 6) {
    src.x += SDLMAN_CHARACTER_SIZE;
    p->draw_count++;
  } else
    p->draw_count = 0;

  SDL_BlitSurface(ps, &src, s, &dst);
}

// static sdlman_draw_enemy( pac man estatico para atrair inimigo)

static void sdlman_draw_enemy(sdlman_character_t *e,
  SDL_Surface *s, SDL_Surface *es, int texture, int boosted)
{
  SDL_Rect src, dst;

  src.w = src.h = dst.w = dst.h = SDLMAN_CHARACTER_SIZE;
  dst.x = e->x;
  dst.y = e->y;

  if (boosted)
    src.y = SDLMAN_CHARACTER_SIZE;
  else
    src.y = 0;

  src.x = 0;

  switch (texture) {
  case 0:
    /* NAda para adicionar */
    break;
  case 1:
    src.x += (SDLMAN_CHARACTER_SIZE * 4);
    break;

  case 2:
    src.x += (SDLMAN_CHARACTER_SIZE * 8);
    break;

  case 3:
    src.x += (SDLMAN_CHARACTER_SIZE * 12);
    break;

  default:
    break;
  }

  src.x += (e->draw_count * SDLMAN_CHARACTER_SIZE);
  e->draw_count++;
  if (e->draw_count > 3)
    e->draw_count = 0;

  SDL_BlitSurface(es, &src, s, &dst);
}


//static sdlman_draw_pellets(bolas de tracao estatica)

static void sdlman_draw_pellets(SDL_Surface *s, sdlman_pellet_t *p, int total)
{
  int i, size, color;
  SDL_Rect r;

  for (i = 0; i < total; i++) {
    if (p[i].consumed)
      continue;
    if (p[i].boost_effect) {
      size = SDLMAN_BOOSTER_SIZE;
      color = 0xffff00; /* Amarelo. */
    } else {
      size = SDLMAN_PELLET_SIZE;
      color = 0xffffff; /* Branco */
    }
    r.w = size;
    r.h = size;
    r.x = p[i].x - (size / 2);
    r.y = p[i].y - (size / 2);
    SDL_FillRect(s, &r, color);
  }
}


//static sdlman_world_collision(Colisao NO MAPA)

static int sdlman_world_collision(sdlman_character_t *c, char *world)
{
  int cx1, cx2, cy1, cy2;

  /* Encontre em todos os blocos do mapa em que o caracter esta localizado*/
  if (c->x / SDLMAN_BLOCK_SIZE ==
    (c->x + SDLMAN_CHARACTER_SIZE - 1) / SDLMAN_BLOCK_SIZE) {
    /* Permanente dentro do bloco na direcao X */
    cx1 = c->x / SDLMAN_BLOCK_SIZE;
    cx2 = -1;
  } else {
    /* Entre dois blocos na direcao X */
    cx1 = c->x / SDLMAN_BLOCK_SIZE;
    cx2 = cx1 + 1;
  }

  if (c->y / SDLMAN_BLOCK_SIZE ==
    (c->y + SDLMAN_CHARACTER_SIZE - 1) / SDLMAN_BLOCK_SIZE) {
    /* Permanente dentro do bloco na direção Y. */
    cy1 = c->y / SDLMAN_BLOCK_SIZE;
    cy2 = -1;
  } else {
    /* Entre dois blocos na direção Y. */
    cy1 = c->y / SDLMAN_BLOCK_SIZE;
    cy2 = cy1 + 1;
  }


  /* Verifique todas as bordas de colisão em potencial. (A menos que os índices estejam fora dos limites.) */

  if ((cy1 >= 0 && cy1 < SDLMAN_WORLD_Y_SIZE) &&
      (cx1 >= 0 && cx1 < SDLMAN_WORLD_X_SIZE)) {
    if (world[(cy1 * SDLMAN_WORLD_X_SIZE) + cx1] == SDLMAN_WORLD_WALL)
      return 1;
  }

  if ((cy2 >= 0 && cy2 < SDLMAN_WORLD_Y_SIZE) &&
      (cx1 >= 0 && cx1 < SDLMAN_WORLD_X_SIZE)) {
    if (world[(cy2 * SDLMAN_WORLD_X_SIZE) + cx1] == SDLMAN_WORLD_WALL)
      return 1;
  }

  if ((cy1 >= 0 && cy1 < SDLMAN_WORLD_Y_SIZE) &&
      (cx2 >= 0 && cx2 < SDLMAN_WORLD_X_SIZE)) {
    if (world[(cy1 * SDLMAN_WORLD_X_SIZE) + cx2] == SDLMAN_WORLD_WALL)
      return 1;
  }

  if ((cy2 >= 0 && cy2 < SDLMAN_WORLD_Y_SIZE) &&
      (cx2 >= 0 && cx2 < SDLMAN_WORLD_X_SIZE)) {
    if (world[(cy2 * SDLMAN_WORLD_X_SIZE) + cx2] == SDLMAN_WORLD_WALL)
      return 1;
  }


  /* Manuseie o envolvimento se o caractere estiver completamente fora da área da tela. */

  if (c->y > (SDLMAN_WORLD_Y_SIZE * SDLMAN_BLOCK_SIZE) - 1) {
    c->y = 0 - SDLMAN_CHARACTER_SIZE + 1;
  }

  if (c->x > (SDLMAN_WORLD_X_SIZE * SDLMAN_BLOCK_SIZE) - 1) {
    c->x = 0 - SDLMAN_CHARACTER_SIZE + 1;
  }

  if (c->y < 0 - SDLMAN_CHARACTER_SIZE + 1) {
    c->y = (SDLMAN_WORLD_Y_SIZE * SDLMAN_BLOCK_SIZE) - 1;
  }

  if (c->x < 0 - SDLMAN_CHARACTER_SIZE + 1) {
    c->x = (SDLMAN_WORLD_X_SIZE * SDLMAN_BLOCK_SIZE) - 1;
  }


  return 0; /* Nao ha colisao */
}


// static sdlman_character_collision( colisao de personagem )

static int sdlman_character_collision(sdlman_character_t *c1,
  sdlman_character_t *c2)
{
  if (c1->y >= c2->y - SDLMAN_CHARACTER_SIZE &&
    c1->y <= c2->y + SDLMAN_CHARACTER_SIZE) {
    if (c1->x >= c2->x - SDLMAN_CHARACTER_SIZE &&
      c1->x <= c2->x + SDLMAN_CHARACTER_SIZE) {
      return 1;
    }
  }
  return 0; /* Nao ha colisao */
}


// static sdlman_pellets_consumed (bolas consumidas )

static int sdlman_pellets_consumed(sdlman_pellet_t *p, int total)
{
  int i, consumed;

  consumed = 0;
  for (i = 0; i < total; i++) {
    if (p[i].consumed)
      consumed++;
  }

  return consumed;
}



// static sdlman_pellet_collision(colisao das bolas)

static int sdlman_pellet_collision(sdlman_character_t *c, sdlman_pellet_t *p,
  int total, int *all_pellets_consumed, int *boost_effect)
{
  int i, consumed, collision;

  *all_pellets_consumed = 0;
  *boost_effect = 0;

  collision = 0;
  consumed = 0;
  for (i = 0; i < total; i++) {
    if (p[i].consumed) {
      consumed++;
      continue;
    }

    if (c->y >= p[i].y - SDLMAN_CHARACTER_SIZE && c->y <= p[i].y) {
      if (c->x >= p[i].x - SDLMAN_CHARACTER_SIZE && c->x <= p[i].x) {
        p[i].consumed = 1;
        if (p[i].boost_effect)
          *boost_effect = 1;
        consumed++;
        collision = 1;
      }
    }
  }

  if (consumed == total)
    *all_pellets_consumed = 1;

  if (collision)
    return 1;
  else
    return 0;
}


// static sdlman_enemy_direction_player (direcao do jogador inimigo)

static void sdlman_enemy_direction_player(sdlman_character_t *e,
  sdlman_character_t *p)
{
  /* Direção da base na localização do jogador. */
  if (e->y > p->y - SDLMAN_BLOCK_SIZE && e->y < p->y + SDLMAN_BLOCK_SIZE) {
    if (e->x > p->x)
      e->moving_direction = SDLMAN_DIRECTION_LEFT;
    else
      e->moving_direction = SDLMAN_DIRECTION_RIGHT;
  } else {
    if (e->y > p->y)
      e->moving_direction = SDLMAN_DIRECTION_UP;
    else
      e->moving_direction = SDLMAN_DIRECTION_DOWN;
  }
}



/* Encontre a abertura onde a direção oposta é uma parede.  */
// static sdlman_enemy_direction_opening(abertura de direcao do inimigo)

static int sdlman_enemy_direction_opening(sdlman_character_t *e, char *world)
{
  int x, y;

  /* Verifique se está dentro do bloco em ambas as direções. */
  if (e->x / SDLMAN_BLOCK_SIZE ==
    (e->x + SDLMAN_CHARACTER_SIZE - 1) / SDLMAN_BLOCK_SIZE) {
    x = e->x / SDLMAN_BLOCK_SIZE;
    if (e->y / SDLMAN_BLOCK_SIZE ==
      (e->y + SDLMAN_CHARACTER_SIZE - 1) / SDLMAN_BLOCK_SIZE) {
      y = e->y / SDLMAN_BLOCK_SIZE;

      /* Permitir somente se dentro dos limites menos 1. */
      if (y < SDLMAN_WORLD_Y_SIZE - 1 && x < SDLMAN_WORLD_X_SIZE - 1) {
        if (world[(y * SDLMAN_WORLD_X_SIZE) + x + 1] != SDLMAN_WORLD_WALL &&
          world[(y * SDLMAN_WORLD_X_SIZE) + x - 1] == SDLMAN_WORLD_WALL) {
          return SDLMAN_DIRECTION_RIGHT;
        }

        if (world[(y * SDLMAN_WORLD_X_SIZE) + x - 1] != SDLMAN_WORLD_WALL &&
          world[(y * SDLMAN_WORLD_X_SIZE) + x + 1] == SDLMAN_WORLD_WALL) {
          return SDLMAN_DIRECTION_LEFT;
        }

        if (world[((y + 1) * SDLMAN_WORLD_X_SIZE) + x] != SDLMAN_WORLD_WALL &&
          world[((y - 1) * SDLMAN_WORLD_X_SIZE) + x] == SDLMAN_WORLD_WALL) {
          return SDLMAN_DIRECTION_DOWN;
        }

        if (world[((y - 1) * SDLMAN_WORLD_X_SIZE) + x] != SDLMAN_WORLD_WALL &&
          world[((y + 1) * SDLMAN_WORLD_X_SIZE) + x] == SDLMAN_WORLD_WALL) {
          return SDLMAN_DIRECTION_UP;
        }
      }
    }
  }

  return 0; /* Nenhuma direcao encontrada */
}


// loop do jogo
int sdlman_gameloop(SDL_Surface *screen, char *world_layout_file,
  char *world_graphic_file, int enemy_speed, int *score)
{
  int i, j, temp_x, temp_y, collision, direction, done_status;
  SDL_Event event;
  SDL_Surface *player_surface, *enemy_surface, *world_surface, *temp_surface;
  Mix_Music *music;
  Mix_Chunk *chomp;
  char world[SDLMAN_WORLD_X_SIZE * SDLMAN_WORLD_Y_SIZE] = {SDLMAN_WORLD_AIR};
  sdlman_character_t player, enemy[SDLMAN_ENEMY_COUNT];
  sdlman_pellet_t pellet[SDLMAN_MAX_PELLET];
  int total_pellets, boost_effect, all_pellets_consumed, booster_time;

  if (sdlman_load_world(world_layout_file, world) != 0) {
    fprintf(stderr, "Erro: Não foi possível carregar o arquivo de layout do mapa.\n");
    return SDLMAN_GAMELOOP_FAIL;
  }

  if (sdlman_locate_player(world, &temp_x, &temp_y) != 0) {
    fprintf(stderr, "Erro: Não foi possível localizar o player no arquivo de layout do mapa.\n");
    return SDLMAN_GAMELOOP_FAIL;
  } else {
    sdlman_init_character(&player, temp_x, temp_y);
  }

  for (i = 0; i < SDLMAN_ENEMY_COUNT; i++) {
    if (sdlman_locate_enemy(world, &temp_x, &temp_y, i) == 0)
      sdlman_init_character(&enemy[i], temp_x, temp_y);
      /* Comece com a direção de movimento aleatório. */
      enemy[i].moving_direction = (rand() % 4) + 1;
  }

  sdlman_init_pellets(world, pellet, &total_pellets);
  *score = 0;
  boost_effect = all_pellets_consumed = booster_time = 0;

  /* Carregar e converter arquivos gráficos. */
  temp_surface = SDL_LoadBMP("player.bmp");
  if (temp_surface == NULL) {
    fprintf(stderr, "Erro: Não é possível carregar gráficos do jogador: %s\n",
      SDL_GetError());
    return SDLMAN_GAMELOOP_FAIL;
  } else {
    player_surface = SDL_DisplayFormat(temp_surface);
    if (player_surface == NULL) {
      fprintf(stderr, "Erro: Não é possível converter os gráficos do jogador: %s\n",
        SDL_GetError());
      return SDLMAN_GAMELOOP_FAIL;
    }
    SDL_FreeSurface(temp_surface);
  }

  temp_surface = SDL_LoadBMP("enemy.bmp");
  if (temp_surface == NULL) {
    fprintf(stderr, "Erro: Não é possível carregar gráficos inimigos: %s\n",
      SDL_GetError());
    SDL_FreeSurface(player_surface);
    return SDLMAN_GAMELOOP_FAIL;
  } else {
    enemy_surface = SDL_DisplayFormat(temp_surface);
    if (enemy_surface == NULL) {
      fprintf(stderr, "Erro: Não é possível converter gráficos inimigos:%s\n",
        SDL_GetError());
      SDL_FreeSurface(player_surface);
      return SDLMAN_GAMELOOP_FAIL;
    }
    SDL_FreeSurface(temp_surface);
  }

  temp_surface = SDL_LoadBMP(world_graphic_file);
  if (temp_surface == NULL) {
    fprintf(stderr, "Aviso: não é possível carregar gráficos do mapa: %s\n",
      SDL_GetError());
    fprintf(stderr, " Usando o desenho básico para gráficos do mapa.\n");
    world_surface = NULL;
  } else {
    world_surface = SDL_DisplayFormat(temp_surface);
    if (world_surface == NULL) {
      fprintf(stderr, "Aviso: não é possível converter gráficos do mapa: %s\n",
        SDL_GetError());
      fprintf(stderr, "Usando o desenho básico para gráficos do mapa.\n");
    }
    SDL_FreeSurface(temp_surface);
  }

/* Inicialize o som e uma  musica. */
  if (sdlman_init_sound(&music, &chomp) != 0) {
    SDL_FreeSurface(player_surface);
    SDL_FreeSurface(enemy_surface);
    if (world_surface != NULL)
      SDL_FreeSurface(world_surface);
    return SDLMAN_GAMELOOP_FAIL;
  }
  sdlman_play_music(music);


  /* loop do jogo principal */
  done_status = 1;
  while (done_status == 1) {

    /* Colete a entrada do player e defina sinalizadores apropriados. */
    if (SDL_PollEvent(&event) == 1) {
      switch(event.type) {
      case SDL_QUIT:
        done_status = SDLMAN_GAMELOOP_QUIT;
        break;

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_q:
          /* Ir para o menu em vez de sair . */
          done_status = SDLMAN_GAMELOOP_OK;
          break;

        case SDLK_UP:
          player.moving_direction = SDLMAN_DIRECTION_UP;
          break;

        case SDLK_DOWN:
          player.moving_direction = SDLMAN_DIRECTION_DOWN;
          break;

        case SDLK_LEFT:
          player.moving_direction = SDLMAN_DIRECTION_LEFT;
          break;

        case SDLK_RIGHT:
          player.moving_direction = SDLMAN_DIRECTION_RIGHT;
          break;

        default:
          break;
        }
        break;

      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_UP:
          if (player.moving_direction == SDLMAN_DIRECTION_UP) {
            player.moving_direction = SDLMAN_DIRECTION_NONE;
            player.looking_direction = SDLMAN_DIRECTION_UP;
            player.speed = 0;
          }
          break;

        case SDLK_DOWN:
          if (player.moving_direction == SDLMAN_DIRECTION_DOWN) {
            player.moving_direction = SDLMAN_DIRECTION_NONE;
            player.looking_direction = SDLMAN_DIRECTION_DOWN;
            player.speed = 0;
          }
          break;

        case SDLK_LEFT:
          if (player.moving_direction == SDLMAN_DIRECTION_LEFT) {
            player.moving_direction = SDLMAN_DIRECTION_NONE;
            player.looking_direction = SDLMAN_DIRECTION_LEFT;
            player.speed = 0;
          }
          break;

        case SDLK_RIGHT:
          if (player.moving_direction == SDLMAN_DIRECTION_RIGHT) {
            player.moving_direction = SDLMAN_DIRECTION_NONE;
            player.looking_direction = SDLMAN_DIRECTION_RIGHT;
            player.speed = 0;
          }
          break;

        default:
          break;
        }

      default:
        continue; /* Importante! Para evitar a captura de eventos de bloqueio do mouse, etc. */
      }
    }


    /* Mova o jogador e verifique se há colisões no mapa */
    switch (player.moving_direction) {
    case SDLMAN_DIRECTION_UP:
      player.speed++;
      if (player.speed > SDLMAN_MAX_PLAYER_SPEED)
        player.speed--;
      player.y -= player.speed;
      /* Continue se movendo de volta passo a passo, até a borda da parede. */
      while (sdlman_world_collision(&player, world) != 0)
        player.y++;
      break;

    case SDLMAN_DIRECTION_DOWN:
      player.speed++;
      if (player.speed > SDLMAN_MAX_PLAYER_SPEED)
        player.speed--;
      player.y += player.speed;
      while (sdlman_world_collision(&player, world) != 0)
        player.y--;
      break;

    case SDLMAN_DIRECTION_LEFT:
      player.speed++;
      if (player.speed > SDLMAN_MAX_PLAYER_SPEED)
        player.speed--;
      player.x -= player.speed;
      while (sdlman_world_collision(&player, world) != 0)
        player.x++;
      break;

    case SDLMAN_DIRECTION_RIGHT:
      player.speed++;
      if (player.speed > SDLMAN_MAX_PLAYER_SPEED)
        player.speed--;
      player.x += player.speed;
      while (sdlman_world_collision(&player, world) != 0)
        player.x--;
      break;

    default:
      break;
    }


    /* Mova os inimigos e verifique suas colisões no mapa. */
    for (i = 0; i < SDLMAN_ENEMY_COUNT; i++) {
      if (enemy[i].killed)
        continue;

      /* Tente passar por uma abertura. */
      if ((direction = sdlman_enemy_direction_opening(&enemy[i], world)) != 0) {
        if (rand() % 3 == 0)
          enemy[i].moving_direction = direction;
      }

      collision = 0;

      switch (enemy[i].moving_direction) {
      case SDLMAN_DIRECTION_UP:
        enemy[i].speed++;
        if (enemy[i].speed > enemy_speed)
          enemy[i].speed--;
        enemy[i].y -= enemy[i].speed;
        while (sdlman_world_collision(&enemy[i], world) != 0) {
          enemy[i].y++;
          collision = 1;
        }
        /*Volte se colidir com outro inimigo. */
        for (j = 0; j < SDLMAN_ENEMY_COUNT; j++) {
          if (enemy[j].killed)
            continue;
          if (j != i) {
            while (sdlman_character_collision(&enemy[i], &enemy[j])) {
              enemy[i].y++;
              collision = 1;
            }
          }
        }
        break;

      case SDLMAN_DIRECTION_DOWN:
        enemy[i].speed++;
        if (enemy[i].speed > enemy_speed)
          enemy[i].speed--;
        enemy[i].y += enemy[i].speed;
        while (sdlman_world_collision(&enemy[i], world) != 0) {
          enemy[i].y--;
          collision = 1;
        }
        for (j = 0; j < SDLMAN_ENEMY_COUNT; j++) {
          if (enemy[j].killed)
            continue;
          if (j != i) {
            while (sdlman_character_collision(&enemy[i], &enemy[j])) {
              enemy[i].y--;
              collision = 1;
            }
          }
        }
        break;

      case SDLMAN_DIRECTION_LEFT:
        enemy[i].speed++;
        if (enemy[i].speed > enemy_speed)
          enemy[i].speed--;
        enemy[i].x -= enemy[i].speed;
        while (sdlman_world_collision(&enemy[i], world) != 0) {
          enemy[i].x++;
          collision = 1;
        }
        for (j = 0; j < SDLMAN_ENEMY_COUNT; j++) {
          if (enemy[j].killed)
            continue;
          if (j != i) {
            while (sdlman_character_collision(&enemy[i], &enemy[j])) {
              enemy[i].x++;
              collision = 1;
            }
          }
        }
        break;

      case SDLMAN_DIRECTION_RIGHT:
        enemy[i].speed++;
        if (enemy[i].speed > enemy_speed)
          enemy[i].speed--;
        enemy[i].x += enemy[i].speed;
        while (sdlman_world_collision(&enemy[i], world) != 0) {
          enemy[i].x--;
          collision = 1;
        }
        for (j = 0; j < SDLMAN_ENEMY_COUNT; j++) {
          if (enemy[j].killed)
            continue;
          if (j != i) {
            while (sdlman_character_collision(&enemy[i], &enemy[j])) {
              enemy[i].x--;
              collision = 1;
            }
          }
        }
        break;

      default:
        break;
      }

      /* Mude a direção para o jogador ou aleatória se bater em alguma coisa. */
      if (collision) {
        if (rand() % 3 == 0)
          enemy[i].moving_direction = (rand() % 4) + 1;
        else {
          if (booster_time == 0) /* Apenas passe para o jogador se não estiver bugado. */
            sdlman_enemy_direction_player(&enemy[i], &player);
        }
      }
    }


    /* Verifique as colisões entre o jogador e os inimigos. */
    for (i = 0; i < SDLMAN_ENEMY_COUNT; i++) {
      if (enemy[i].killed)
        continue;
      if (sdlman_character_collision(&player, &enemy[i])) {
        if (booster_time > 0) {
          enemy[i].killed = 1;
          sdlman_play_sound(chomp);
          *score += SDLMAN_SCORE_ENEMY;
        } else {
          fprintf(stderr, "Info: Killed by the enemy.\n");
          done_status = SDLMAN_GAMELOOP_OK;
        }
      }
    }


    /* Verifique as colisões e saia se a última pastilha tiver sido consumida. */
    if (sdlman_pellet_collision(&player, pellet, total_pellets,
      &all_pellets_consumed, &boost_effect) == 1) {
      sdlman_play_sound(chomp);
      if (all_pellets_consumed) {
        fprintf(stderr, "Info: All pellets consumed.\n");
        *score += SDLMAN_SCORE_FINISH; /* Pontuação extra para consumir todos.. */
        done_status = SDLMAN_GAMELOOP_OK;
      }
      if (boost_effect)
        booster_time = SDLMAN_BOOSTER_TIME;
    }
    if (booster_time > 0)
      booster_time--;


    /* Desenhe gráficos e relaxe a execução. */
    if (world_surface != NULL)
      sdlman_draw_world_bitmap(screen, world_surface);
    else
      sdlman_draw_world_basic(screen, world);

    sdlman_draw_pellets(screen, pellet, total_pellets);

    sdlman_draw_player(&player, screen, player_surface, booster_time);

    for (i = 0; i < SDLMAN_ENEMY_COUNT; i++) {
      if (enemy[i].killed)
        continue;
      sdlman_draw_enemy(&enemy[i], screen, enemy_surface, i, booster_time);
    }

    SDL_Flip(screen);
    SDL_Delay(SDLMAN_GAME_SPEED);
  }


  /* Limpar. */
  SDL_FreeSurface(player_surface);
  SDL_FreeSurface(enemy_surface);
  if (world_surface != NULL)
    SDL_FreeSurface(world_surface);
  Mix_CloseAudio();
  Mix_FreeMusic(music);
  Mix_FreeChunk(chomp);

  /* Atualizar pontuação. */
  *score += sdlman_pellets_consumed(pellet, total_pellets) *
    SDLMAN_SCORE_PELLET;
  *score *= enemy_speed;
  if (*score < 0)
    *score = 0;

  if (done_status != SDLMAN_GAMELOOP_OK)
    *score = 0; /* Resetando pontuacao */

  return done_status;
}
