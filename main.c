/*******************************************************************
* Projeto PacMan Wars Linguagem C - LAboratorio de Programacao ll  *
* Bibliotecas ultilizadas SDL e SDL_mixer                          *
* Autores : Wanderson Luan e Antonio Dionisio                      *
* Compilador CodeBlocks + mingw + sdl _ sdl_mix                    *
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/***Bibliotecas SDL***/
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h> // Sons

/* definicoes */
#define pacWar_BLOCK_SIZE 32
#define pacWar_WORLD_X_SIZE 20
#define pacWar_WORLD_Y_SIZE 15

#define pacWarGame_OK 0
#define pacWarGame_FAIL -1
#define pacWarGame_QUIT -2

#define pacWar_GAME_SPEED 20

#define pacWar_MAX_PLAYER_SPEED 5
#define pacWar_CHARACTER_SIZE 26
#define pacWar_ENEMY_COUNT 4
#define pacWar_PELLET_SIZE 4
#define pacWar_BOOSTER_SIZE 6
#define pacWar_MAX_PELLET 300 /* 15 x 20 */
#define pacWar_BOOSTER_TIME 120 /* Em ciclos de loop */

#define pacWar_SCORE_PELLET 2

#define pacWar_SCORE_ENEMY -25 /* Penalidade por matar enemys*/

#define pacWar_WORLD_AIR ' '
#define pacWar_WORLD_WALL '#'
#define pacWar_WORLD_PLAYER 'P'
#define pacWar_WORLD_ENEMY 'E'
#define pacWar_WORLD_PELLET '.'
#define pacWar_WORLD_BOOSTER '*'

#define pacWar_FILENAME_LENGTH 16
#define pacWar_MAX_ENEMY_SPEED 9
#define pacWar_MAX_WORLD 5
#define larguraNum 26 /** determina a altura dos números utilizados no menu e no game **/
#define alturaNum 32

#define pacWar_HIGHSCORE_FILE "highscore.dat"

#define pacWar_DIRECTION_NONE 0
#define pacWar_DIRECTION_UP 1
#define pacWar_DIRECTION_DOWN 2
#define pacWar_DIRECTION_LEFT 3
#define pacWar_DIRECTION_RIGHT 4

// struct pacWar_character_s
typedef struct pacWar_character_s { /** Pac's "fantasias"**/
    int x, y; /* Coordenadas*/
    int moving_direction, looking_direction;
    int speed;
    int draw_count; /** Usado para animacao **/
    int killed;
} pacWar_character_t;

// struct pacWar_pellet_s
typedef struct pacWar_pellet_s { /** bolas **/
    int x, y;
    int consumed;
    int boost_effect;
} pacWar_pellet_t;

/* Protótipos*/
int pacWarGame(SDL_Surface *, char *,char *, int , int *);
int iniSom(Mix_Music **, Mix_Chunk **, Mix_Chunk **);
void tocaMusica(Mix_Music *);
void tocaSons(Mix_Chunk *);
int pacWarCarregaMapa(char *, char *); /** lê o arquivo layout e verifica as configurações do layout em relação á ' ', '#', '*', '.' **/
int localizaPac(char *, int *, int *); /** lê o arquivo layout e verifica onde está posicionado o pacman **/
int localizaFantasmas(char *, int *, int *, int );
void pacWar_init_character(pacWar_character_t *c, int x, int y);
void pacWar_init_pellets(char *world, pacWar_pellet_t *p, int *total);
//void pacWarDesenhoworld_basic(SDL_Surface *s, char *world);
void desenhaMapa(SDL_Surface *, SDL_Surface *);
void desenhaPac(pacWar_character_t *,SDL_Surface *, SDL_Surface *, int );
void desenhaFantasmas(pacWar_character_t *, SDL_Surface *, SDL_Surface *, int , int );
void desenhaPastilhas(SDL_Surface *, pacWar_pellet_t *, int );
int colisaoMapa(pacWar_character_t *, char *);
int pacWar_character_collision(pacWar_character_t *c1,pacWar_character_t *c2);
int pastilhasConsumidas(pacWar_pellet_t *, int );
int comendoPastilhas(pacWar_character_t *, pacWar_pellet_t *,int , int *, int *);
void pacWar_enemy_direction_player(pacWar_character_t *e,pacWar_character_t *p);
int pacWar_enemy_direction_opening(pacWar_character_t *e, char *world);
void desenhaNum(SDL_Surface *, SDL_Surface *, int , int , int );
void pacWar_load_highscore(int *table, char *filename);
void pacWar_save_highscore(int *table, char *filename);



int main(int argc, char *argv[])
{
    SDL_Event event;

    /** SDL_Event é interessante ==> É uma união... União é um tipo de dados que possui ("retorna") um único
     dado entre uma variedade disponível entre seus membros, ou seja, só um membro será usado de fato, excluindo
     a possibilidade dos outros. Seu tamanho é o tamanho do maior membro. É como uma estrutura na qual todos os
     membros são armazenados no mesmo endereço. É uma forma de dinamizar o dado - um único dado que pode assumir
     várias formas -, ou seja, é útil para criar um "tipo variável".**/

    SDL_Surface *screen, *menu_surface, *number_surface, *temp_surface;
    int game_done, game_result, world_number, enemy_speed, game_score;
    char layout_file[pacWar_FILENAME_LENGTH];
    char graphic_file[pacWar_FILENAME_LENGTH];
    int high_score[pacWar_MAX_WORLD];

    srand((unsigned)time(NULL));

    /** Tente carregar recordes do arquivo. **/
    pacWar_load_highscore(high_score, pacWar_HIGHSCORE_FILE);

    /** Vídeo e inicialização de tela aqui, por causa do menu gráfico. **/
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        printf(stderr, "Erro: não é possível inicializar o SDL: %s\n", SDL_GetError());
        return 1;
    }
    screen = SDL_SetVideoMode(pacWar_WORLD_X_SIZE * 40/*pacWar_BLOCK_SIZE*/,
                              pacWar_WORLD_Y_SIZE *40 /*pacWar_BLOCK_SIZE*/, 24, SDL_DOUBLEBUF/*SDL_FULLSCREEN*/);
                              /** controla a janela aberta **/

    if (screen == NULL)
    {
        fprintf(stderr, "Erro: não é possível definir o modo de vídeo: %s\n", SDL_GetError());
        return 1;
    }
    //SDL_WM_SetCaption("SDL-Man", "SDL-Man");


    /** Carregar e converter arquivos gráficos de menu. **/
    temp_surface = SDL_LoadBMP("menu.bmp");

    if (temp_surface == NULL)
    {
        fprintf(stderr, "Erro: não é possível carregar os gráficos do menu: %s\n",SDL_GetError());
        return 1;
    }
    else
    {
        menu_surface = SDL_DisplayFormat(temp_surface);
        if (menu_surface == NULL)
        {
            fprintf(stderr, "Erro: Não é possível converter os gráficos do menu: %s\n", SDL_GetError());
            return 1;
        }
        SDL_FreeSurface(temp_surface);
    }

    temp_surface = SDL_LoadBMP("number.bmp");

    if (temp_surface == NULL)
    {
        fprintf(stderr, "Erro: não é possível carregar gráficos numéricos: %s\n",SDL_GetError());
        SDL_FreeSurface(menu_surface);
        return 1;
    }
    else
    {
        number_surface = SDL_DisplayFormat(temp_surface);
        if (number_surface == NULL)
        {
            fprintf(stderr, "Erro: Não é possível converter gráficos enemys: %s\n",SDL_GetError());
            SDL_FreeSurface(menu_surface);
            return 1;
        }
        SDL_FreeSurface(temp_surface);
    }


    /** Inicializando valores na tela de Menu **/
    enemy_speed = 1;
    world_number = 1;


    /** Menu **/
    game_done = 0;
    while (! game_done)
    {


        /** processo para entrada **/
        if (SDL_PollEvent(&event) == 1) /** SDL_PollEvent ==> Aguarda a definição do usuário: 0 se event for NULL **/
        {
            switch(event.type) /** Acessando um dado do tipo "uint8_t" -- inteiro de 8 bites sem sinal (0 à 255) da biblioteca stdinc.h**/
            {
                case SDL_QUIT: /**Sai do jogo -- SDL_QUIT retorna um inteiro (12) para event.type indicando a saída do jogo**/
                    game_done = 1;
                break;

                case SDL_KEYDOWN: /** acessa a tecla pressionada pelo usuário -- retorna 3 para event.type **/
                    switch (event.key.keysym.sym) /** muda o foco de SDL_Event para o tipo SDL_KeyboardEvent **/
                    {                             /** sym é um tipo de dado "enum" que parece uma struct porém tem todos os valores constantes **/
                        case SDLK_ESCAPE: /** tecla ESC para fechar -- retorna 27 para event.key em SDL_Event**/
                            game_done = 1;
                        break;

                        case SDLK_RETURN: /** tecla ENTER com mesmo efeito de ESPAÇO (abaixo) -- retorna 13 para event.key em SDL_Event **/
                        case SDLK_SPACE: /** tecla ESPAÇO para "rodar" o jogo -- retorna 32 para event.key em SDL_Event **/
                            snprintf(layout_file, pacWar_FILENAME_LENGTH, "world.layout",world_number);
                            snprintf(graphic_file, pacWar_FILENAME_LENGTH, "world.bmp",world_number);

                            game_result = pacWarGame(screen, layout_file, graphic_file,enemy_speed, &game_score);

                            if (game_result == pacWarGame_OK)
                            {
                                /* Atualizar pontuação*/
                                if (game_score > high_score[world_number - 1])
                                    high_score[world_number - 1] = game_score;
                            }
                            else
                            {
                                game_done = 1; /* falhou . */
                            }
                        break;


                        case SDLK_w:
                            world_number++;
                        if (world_number > pacWar_MAX_WORLD)
                            world_number = 1;
                        break;

                        case SDLK_s:
                            enemy_speed++;
                            if (enemy_speed > pacWar_MAX_ENEMY_SPEED)
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



        /** Desenhe gráficos **/
        SDL_BlitSurface(menu_surface, NULL, screen, NULL);
        //pacWarDesenhomenu_number(screen, number_surface, 385, 163, world_number);
        desenhaNum(screen, number_surface, 385, 211, enemy_speed); /** serve para escrever a velocidade dos fantasmas **/
        desenhaNum(screen, number_surface, 385, 259,high_score[world_number - 1]); /** serve para escrever a pontuação máxima **/

        SDL_Flip(screen);
        SDL_Delay(pacWar_GAME_SPEED);
    }


    /* Limpar */
    SDL_FreeSurface(menu_surface);
    SDL_FreeSurface(number_surface);
    SDL_FreeSurface(screen);
    pacWar_save_highscore(high_score, pacWar_HIGHSCORE_FILE);

    return 0;
}

/** iniciando o som **/
int iniSom(Mix_Music **music, Mix_Chunk **comendo, Mix_Chunk **menu)
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        fprintf(stderr, "Erro nao e possivel abrir o audio %s\n", SDL_GetError());
        Mix_CloseAudio();
        return -1;
    }

    if (! (*music = Mix_LoadMUS("music.wav"))) //ou use musicWars.wav
    {
        fprintf(stderr, "Erro nao foi possivel abrir o audio: %s\n",SDL_GetError());
        Mix_CloseAudio();
        return -1;
    }

    if (! (*comendo = Mix_LoadWAV("comendo.wav")))
    {
        fprintf(stderr, "Erro nao foi possivel abrir o audio: %s\n",SDL_GetError());
        Mix_CloseAudio();
        Mix_FreeMusic(*music);
        return -1;
    }
    if (! (*menu = Mix_LoadWAV("menuWars.wav")))
    {
        fprintf(stderr, "Erro nao foi possivel abrir o audio: %s\n",SDL_GetError());
        Mix_CloseAudio();
        //Mix_FreeMusic(*music);
        return -1;
    }

    return 0;
}


// tocaMusica (ativando musica)

void tocaMusica(Mix_Music *m)
{
    if (Mix_PlayMusic(m, 0) == -1)
        fprintf(stderr, "Não e possivel reproduzir o audio %s\n", SDL_GetError());
}


/** ativa os audios **/

void tocaSons(Mix_Chunk *a)
{
    /** pare todos os canais de audio **/
    Mix_HaltChannel(-1);

    if (Mix_PlayChannel(-1, a, 0) == -1)
        fprintf(stderr, "Aviso :não e possivel reproduzir o som %s\n", SDL_GetError());
}


/** escrever dentro do arquivo do mapa**/

int pacWarCarregaMapa(char *arqLayout, char *world)
{
    int c, w, h;
    FILE *arq;

    arq = fopen(arqLayout, "r");
    if (arq == NULL)
    {
        fprintf(stderr, "Erro nao e possivel abrir o'%s' para leitura \n", arqLayout);
        return -1;
    }

    w = 0;
    h = 2; // controle da posição da matriz
    while ((c = fgetc(arq)) != EOF)
    {
        if (c == '\n' || w >= pacWar_WORLD_X_SIZE)
        {
            w = 0;
            h++;
            if (h >= pacWar_WORLD_Y_SIZE+2)
                return 0; /** limites do mapa **/
        }
        else
        {
            world[(h * pacWar_WORLD_X_SIZE) + w] = c;
            w++;
        }
    }

    fclose(arq);
    return 0;
}


/** localizando jogador **/

int localizaPac(char *world, int *x, int *y)
{
    int i, j;
    for (i = 0; i < pacWar_WORLD_Y_SIZE; i++)
    {
        for (j = 0; j < pacWar_WORLD_X_SIZE; j++)
        {
            if (world[(i * pacWar_WORLD_X_SIZE) + j] == pacWar_WORLD_PLAYER)
            {
                *x = j * pacWar_BLOCK_SIZE;
                *y = i * pacWar_BLOCK_SIZE;
                return 0;
            }
        }
    }
    return -1; /* nao encontrado */
}


/** localizando os fantasmas **/

int localizaFantasmas(char *world, int *x, int *y, int n)
{
    int i, j, count;
    count = 0;
    for (i = 0; i < pacWar_WORLD_Y_SIZE; i++)
    {
        for (j = 0; j < pacWar_WORLD_X_SIZE; j++)
        {
            if (world[(i * pacWar_WORLD_X_SIZE) + j] == pacWar_WORLD_ENEMY)
            {
                if (count == n)
                {
                    *x = j * pacWar_BLOCK_SIZE;
                    *y = i * pacWar_BLOCK_SIZE;
                    return 0;
                }
                count++;
            }
        }
    }
    return -1; /* nao encontrado */
}

/** Inicializa o pacman **/

void pacWar_init_character(pacWar_character_t *c, int x, int y)
{
    /** iniciando coordenadas X e Y */
    c->x = x;
    c->y = y;
    c->moving_direction = c->looking_direction = pacWar_DIRECTION_NONE;
    c->speed = 0;
    c->draw_count = 0;
    c->killed = 0;
}


/** inicializando as bolinhas **/

void pacWar_init_pellets(char *world, pacWar_pellet_t *p, int *total)
{
    int i, j, n;
    n = 0;
    for (i = 2; i < pacWar_WORLD_Y_SIZE+2; i++)
    {
        for (j = 0; j < pacWar_WORLD_X_SIZE; j++)
        {
            if ((world[(i * pacWar_WORLD_X_SIZE) + j] == pacWar_WORLD_PELLET)||(world[(i * pacWar_WORLD_X_SIZE) + j] == pacWar_WORLD_BOOSTER)) /** controla as bolas **/
            {
                p[n].x = (j * pacWar_BLOCK_SIZE) + (pacWar_BLOCK_SIZE / 2);
                p[n].y = (i * pacWar_BLOCK_SIZE) + (pacWar_BLOCK_SIZE / 2);
                p[n].consumed = 0;

                if (world[(i * pacWar_WORLD_X_SIZE) + j] == pacWar_WORLD_BOOSTER)
                    p[n].boost_effect = 1;
                else
                    p[n].boost_effect = 0;

                n++;
                if (n >= pacWar_MAX_PELLET - 1)
                    break;
            }
        }
    }
    *total = n;
}

/** criando o mapa basico **/

/*void pacWarDesenhoworld_basic(SDL_Surface *s, char *world)
{
    int i, j;
    SDL_Rect r;

    r.x = 0;
    r.y = 0;
    r.w = pacWar_WORLD_X_SIZE * pacWar_BLOCK_SIZE;
    r.h = pacWar_WORLD_Y_SIZE * pacWar_BLOCK_SIZE;
    SDL_FillRect(s, &r, 0x0);

    r.w = pacWar_BLOCK_SIZE;
    r.h = pacWar_BLOCK_SIZE;
    for (i = 0; i < pacWar_WORLD_Y_SIZE; i++)
    {
        for (j = 0; j < pacWar_WORLD_X_SIZE; j++)
        {
            if (world[(i * pacWar_WORLD_X_SIZE) + j] == pacWar_WORLD_WALL)
            {
                r.x = j * pacWar_BLOCK_SIZE;
                r.y = i * pacWar_BLOCK_SIZE;
                SDL_FillRect(s, &r, 0xffffff);
            }
        }
    }
}*/


//pacWarDesenhoworld_bitmap(criando mapa do bitmap)

void desenhaMapa(SDL_Surface *s, SDL_Surface *ws)
{
    SDL_BlitSurface(ws, NULL, s, NULL);
}

// pacWarDesenhoplayer(jogador de empate)

void desenhaPac(pacWar_character_t *p,SDL_Surface *s, SDL_Surface *ps, int boosted)
{
    SDL_Rect src, dst;
    int direction;

    src.w = src.h = dst.w = dst.h = pacWar_CHARACTER_SIZE;
    dst.x = p->x;
    dst.y = p->y;

    if (boosted)
        src.y = pacWar_CHARACTER_SIZE;
    else
        src.y = 0;

    src.x = 0;

    if (p->moving_direction == pacWar_DIRECTION_NONE)
        direction = p->looking_direction;
    else
        direction = p->moving_direction;

    switch (direction) /** muda a animação do pac **/
    {
        case pacWar_DIRECTION_UP:
        break;
        case pacWar_DIRECTION_DOWN:
            src.x += (pacWar_CHARACTER_SIZE * 4);
        break;

        case pacWar_DIRECTION_LEFT:
            src.x += (pacWar_CHARACTER_SIZE * 8);
        break;

        case pacWar_DIRECTION_RIGHT:
            src.x += (pacWar_CHARACTER_SIZE * 12);
        break;

        default:
        break;
    }

    if (p->draw_count < 4)
    {
        src.x += (p->draw_count * pacWar_CHARACTER_SIZE);
        p->draw_count++;
    }
    else
        if (p->draw_count == 4)
        {
            src.x += (pacWar_CHARACTER_SIZE * 3);
            p->draw_count++;
        }
        else
            if (p->draw_count == 5)
            {
                src.x += (pacWar_CHARACTER_SIZE * 2);
                p->draw_count++;
            }
            else
                if (p->draw_count == 6)
                {
                    src.x += pacWar_CHARACTER_SIZE;
                    p->draw_count++;
                }
                else
                    p->draw_count = 0;

    SDL_BlitSurface(ps, &src, s, &dst); /**função que percorre a imagem e corta a partir do desejado **/
}

// pacWarDesenhoenemy( pac man estatico para atrair enemy)

void desenhaFantasmas(pacWar_character_t *e, SDL_Surface *s, SDL_Surface *es, int texture, int boosted)
{
    SDL_Rect src, dst;

    src.w = src.h = dst.w = dst.h = pacWar_CHARACTER_SIZE;
    dst.x = e->x;
    dst.y = e->y;

    if (boosted)
        src.y = pacWar_CHARACTER_SIZE;
    else
        src.y = 0;

    src.x = 0;

    switch (texture)
    {
        case 0:
                /* NAda para adicionar */
        break;
        case 1:
            src.x += (pacWar_CHARACTER_SIZE * 4);
        break;

        case 2:
            src.x += (pacWar_CHARACTER_SIZE * 8);
        break;

        case 3:
            src.x += (pacWar_CHARACTER_SIZE * 12);
        break;

        default:
        break;
    }

    src.x += (e->draw_count * pacWar_CHARACTER_SIZE);
    e->draw_count++;
    if (e->draw_count > 3)
        e->draw_count = 0;

    SDL_BlitSurface(es, &src, s, &dst);
}


/** desenha as bolas **/

void desenhaPastilhas(SDL_Surface *s, pacWar_pellet_t *p, int total)
{
    int i, size, color;
    SDL_Rect r;

    for (i = 0; i < total; i++)
    {
        if (p[i].consumed)
        continue;
        if (p[i].boost_effect)
        {
            size = pacWar_BOOSTER_SIZE;
            color = 0xffff00; /* Amarelo. */
        }
        else
        {
            size = pacWar_PELLET_SIZE;
            color = 0xffffff; /* Branco */
        }
        r.w = size;
        r.h = size;
        r.x = p[i].x - (size / 2);
        r.y = p[i].y - (size / 2);
        SDL_FillRect(s, &r, color);
    }
}


/** Colisao NO MAPA */

int colisaoMapa(pacWar_character_t *c, char *world)
{
    int cx1, cx2, cy1, cy2;

    /* Encontre em todos os blocos do mapa em que o caracter esta localizado*/
    if (c->x / pacWar_BLOCK_SIZE == (c->x + pacWar_CHARACTER_SIZE - 1) / pacWar_BLOCK_SIZE)
    {
        /* Permanente dentro do bloco na direcao X */
        cx1 = c->x / pacWar_BLOCK_SIZE;
        cx2 = -1;
    }
    else
    {
        /* Entre dois blocos na direcao X */
        cx1 = c->x / pacWar_BLOCK_SIZE;
        cx2 = cx1 + 1;
    }

    if (c->y / pacWar_BLOCK_SIZE == (c->y + pacWar_CHARACTER_SIZE - 1) / pacWar_BLOCK_SIZE)
    {
        /* Permanente dentro do bloco na direção Y. */
        cy1 = c->y / pacWar_BLOCK_SIZE;
        cy2 = -1;
    }
    else
    {
        /* Entre dois blocos na direção Y. */
        cy1 = c->y / pacWar_BLOCK_SIZE;
        cy2 = cy1 + 1;
    }


    /* Verifique todas as bordas de colisão em potencial. (A menos que os índices estejam fora dos limites.) */

    if ((cy1 >= 0 && cy1 < pacWar_WORLD_Y_SIZE) &&(cx1 >= 0 && cx1 < pacWar_WORLD_X_SIZE))
    {
        if (world[(cy1 * pacWar_WORLD_X_SIZE) + cx1] == pacWar_WORLD_WALL)
            return 1;
    }

    if ((cy2 >= 0 && cy2 < pacWar_WORLD_Y_SIZE) && (cx1 >= 0 && cx1 < pacWar_WORLD_X_SIZE))
    {
        if (world[(cy2 * pacWar_WORLD_X_SIZE) + cx1] == pacWar_WORLD_WALL)
            return 1;
    }

    if ((cy1 >= 0 && cy1 < pacWar_WORLD_Y_SIZE) && (cx2 >= 0 && cx2 < pacWar_WORLD_X_SIZE))
    {
        if (world[(cy1 * pacWar_WORLD_X_SIZE) + cx2] == pacWar_WORLD_WALL)
            return 1;
    }

    if ((cy2 >= 0 && cy2 < pacWar_WORLD_Y_SIZE) &&(cx2 >= 0 && cx2 < pacWar_WORLD_X_SIZE))
    {
        if (world[(cy2 * pacWar_WORLD_X_SIZE) + cx2] == pacWar_WORLD_WALL)
            return 1;
    }


    /* Manuseie o envolvimento se o caractere estiver completamente fora da área da tela. */

    if (c->y > (pacWar_WORLD_Y_SIZE * pacWar_BLOCK_SIZE) - 1)
        c->y = 0 - pacWar_CHARACTER_SIZE + 1;

    if (c->x > (pacWar_WORLD_X_SIZE * pacWar_BLOCK_SIZE) - 1)
        c->x = 0 - pacWar_CHARACTER_SIZE + 1;

    if (c->y < 0 - pacWar_CHARACTER_SIZE + 1)
        c->y = (pacWar_WORLD_Y_SIZE * pacWar_BLOCK_SIZE) - 1;

    if (c->x < 0 - pacWar_CHARACTER_SIZE + 1)
        c->x = (pacWar_WORLD_X_SIZE * pacWar_BLOCK_SIZE) - 1;


    return 0; /* Nao ha colisao */
}


/** colisao de personagem **/

int pacWar_character_collision(pacWar_character_t *c1,pacWar_character_t *c2)
{
    if (c1->y >= c2->y - pacWar_CHARACTER_SIZE && c1->y <= c2->y + pacWar_CHARACTER_SIZE)
        if (c1->x >= c2->x - pacWar_CHARACTER_SIZE && c1->x <= c2->x + pacWar_CHARACTER_SIZE)
            return 1;
    return 0; /* Nao ha colisao */
}


// pacWar_pellets_consumed (bolas consumidas )

int pastilhasConsumidas(pacWar_pellet_t *p, int total)
{
    int i, consumed;

    consumed = 0;
    for (i = 0; i < total; i++)
        if (p[i].consumed)
            consumed++;

    return consumed;
}



// pacWar_pellet_collision(colisao das bolas)

int comendoPastilhas(pacWar_character_t *c, pacWar_pellet_t *p,int total, int *all_pellets_consumed, int *boost_effect)
{
    int i, consumed, collision;

    *all_pellets_consumed = 0;
    *boost_effect = 0;

    collision = 0;
    consumed = 0;
    for (i = 0; i < total; i++)
    {
        if (p[i].consumed)
        {
            consumed++;
            continue;
        }

        if (c->y >= p[i].y - pacWar_CHARACTER_SIZE && c->y <= p[i].y)
        {
            if (c->x >= p[i].x - pacWar_CHARACTER_SIZE && c->x <= p[i].x)
            {
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


// pacWar_enemy_direction_player (direcao do jogador enemy)

void pacWar_enemy_direction_player(pacWar_character_t *e,pacWar_character_t *p) /** direção dos fantasmas **/
{
    /* Direção da base na localização do jogador. */
    if (e->y > p->y - pacWar_BLOCK_SIZE && e->y < p->y + pacWar_BLOCK_SIZE)
    {
        if (e->x > p->x)
            e->moving_direction = pacWar_DIRECTION_LEFT;
        else
            e->moving_direction = pacWar_DIRECTION_RIGHT;
    }
    else
    {
        if (e->y > p->y)
            e->moving_direction = pacWar_DIRECTION_UP;
        else
            e->moving_direction = pacWar_DIRECTION_DOWN;
    }
}



/* Encontre a abertura onde a direção oposta é uma parede.*/

int pacWar_enemy_direction_opening(pacWar_character_t *e, char *world) /** direção dos fantasmas **/
{
    int x, y;

    /* Verifique se está dentro do bloco em ambas as direções. */
    if (e->x / pacWar_BLOCK_SIZE == (e->x + pacWar_CHARACTER_SIZE - 1) / pacWar_BLOCK_SIZE)
    {
        x = e->x / pacWar_BLOCK_SIZE;
        if (e->y / pacWar_BLOCK_SIZE == (e->y + pacWar_CHARACTER_SIZE - 1) / pacWar_BLOCK_SIZE)
        {
            y = e->y / pacWar_BLOCK_SIZE;

            /* Permitir somente se dentro dos limites menos 1. */
            if (y < pacWar_WORLD_Y_SIZE - 1 && x < pacWar_WORLD_X_SIZE - 1)
            {
                if (world[(y * pacWar_WORLD_X_SIZE) + x + 1] != pacWar_WORLD_WALL &&world[(y * pacWar_WORLD_X_SIZE) + x - 1] == pacWar_WORLD_WALL)
                    return pacWar_DIRECTION_RIGHT;

                if (world[(y * pacWar_WORLD_X_SIZE) + x - 1] != pacWar_WORLD_WALL && world[(y * pacWar_WORLD_X_SIZE) + x + 1] == pacWar_WORLD_WALL)
                    return pacWar_DIRECTION_LEFT;

                if (world[((y + 1) * pacWar_WORLD_X_SIZE) + x] != pacWar_WORLD_WALL && world[((y - 1) * pacWar_WORLD_X_SIZE) + x] == pacWar_WORLD_WALL)
                    return pacWar_DIRECTION_DOWN;

                if (world[((y - 1) * pacWar_WORLD_X_SIZE) + x] != pacWar_WORLD_WALL && world[((y + 1) * pacWar_WORLD_X_SIZE) + x] == pacWar_WORLD_WALL)
                    return pacWar_DIRECTION_UP;
            }
        }
    }

    return 0; /* Nenhuma direcao encontrada */
}


/** desenha (anima) os número **/
void desenhaNum(SDL_Surface *s, SDL_Surface *ns, int x, int y, int n)
{
    int i, pow, digits;
    SDL_Rect src, dst; /** struct que determina a altura, largura, posição x e posição y **/

    src.w = dst.w = larguraNum;
    src.h = dst.h = alturaNum;

    dst.x = x;
    dst.y = y;
    src.y = 0;

    /** Encontre dígitos em número **/
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
        n = 0; /** Força para zero se for negativo **/
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
          src.x = larguraNum;
          break;
        case 3:
          src.x = larguraNum * 2;
          break;
        case 4:
          src.x = larguraNum * 3;
          break;
        case 5:
          src.x = larguraNum * 4;
          break;
        case 6:
          src.x = larguraNum * 5;
          break;
        case 7:
          src.x = larguraNum * 6;
          break;
        case 8:
          src.x = larguraNum * 7;
          break;
        case 9:
          src.x = larguraNum * 8;
          break;
        case 0:
          src.x = larguraNum * 9;
          break;
        default:
          return; /* Não encontrado, apenas retorne. */
    }

    SDL_BlitSurface(ns, &src, s, &dst);

    digits--;
    dst.x += larguraNum;
  }
}


void pacWar_load_highscore(int *table, char *filename)
{
  int i;
  FILE *fh;

  fh = fopen(filename, "r");
  if (fh == NULL) {
    fprintf(stderr, "Aviso: não é possível carregar o arquivo de recordes.\n");
    /* Define todos os recordes para zero.*/
    for (i = 0; i < pacWar_MAX_WORLD; i++) {
      table[i] = 0;
    }
  } else {
    fread(table, sizeof(int), pacWar_MAX_WORLD, fh);
    fclose(fh);
  }
}

void pacWar_save_highscore(int *table, char *filename)
{
  FILE *fh;

  fh = fopen(filename, "w");
  if (fh == NULL) {
    fprintf(stderr, "Aviso: Não é possível salvar o arquivo de recordes.\n");
  } else {
    fwrite(table, sizeof(int), pacWar_MAX_WORLD, fh);
    fclose(fh);
  }
}

/** função principal que controla o loop do jogo **/
int pacWarGame(SDL_Surface *screen, char *world_layout_file, char *world_graphic_file, int enemy_speed, int *score)
{
    int i, j, temp_x, temp_y, collision, direction, done_status;
    SDL_Event event;
    SDL_Surface *player_surface, *enemy_surface, *world_surface, *temp_surface, *number_surface;

    /** definindo variáveis de som **/
    Mix_Music *music;
    Mix_Chunk *comendo;
    Mix_Chunk *menu;

    char world[(pacWar_WORLD_X_SIZE +20)* (pacWar_WORLD_Y_SIZE + 20)] = {pacWar_WORLD_AIR};

    pacWar_character_t player, enemy[pacWar_ENEMY_COUNT];
    pacWar_pellet_t pellet[pacWar_MAX_PELLET];

    int total_pellets, boost_effect, all_pellets_consumed, booster_time, comeCome = 0;

    number_surface = SDL_DisplayFormat(SDL_LoadBMP("number.bmp"));

    //comeCome = pacWar_pellet_collision(&player, pellet, total_pellets,&all_pellets_consumed, &boost_effect);

    if (pacWarCarregaMapa(world_layout_file, world) != 0)
    {
        fprintf(stderr, "Erro: Não foi possível carregar o arquivo de layout do mapa.\n");
        return pacWarGame_FAIL;
    }

    if (localizaPac(world, &temp_x, &temp_y) != 0)
    {
        fprintf(stderr, "Erro: Não foi possível localizar o player no arquivo de layout do mapa.\n");
        return pacWarGame_FAIL;
    }
    else
    {
        pacWar_init_character(&player, temp_x, temp_y);
    }

    for (i = 0; i < pacWar_ENEMY_COUNT; i++)
    {
        if (localizaFantasmas(world, &temp_x, &temp_y, i) == 0)
            pacWar_init_character(&enemy[i], temp_x, temp_y);
            /* Comece com a direção de movimento aleatório. */
        enemy[i].moving_direction = (rand() % 4) + 1;
    }

    pacWar_init_pellets(world, pellet, &total_pellets);
    *score = 0;
    boost_effect = all_pellets_consumed = booster_time = 0;

    /* Carregar e converter arquivos gráficos. */
    temp_surface = SDL_LoadBMP("player.bmp"); /** carrega o pacman **/
    if (temp_surface == NULL)
    {
        fprintf(stderr, "Erro: Não é possível carregar gráficos do pacman: %s\n",SDL_GetError());
        return pacWarGame_FAIL;
    }
    else
    {
        player_surface = SDL_DisplayFormat(temp_surface);
        if (player_surface == NULL)
        {
            fprintf(stderr, "Erro: Não é possível converter os gráficos do pacman: %s\n",SDL_GetError());
            return pacWarGame_FAIL;
        }
        SDL_FreeSurface(temp_surface);
    }

    temp_surface = SDL_LoadBMP("enemy.bmp"); /** carrega os fantasmas **/
    if (temp_surface == NULL)
    {
        fprintf(stderr, "Erro: Não é possível carregar gráficos dos fantasmas: %s\n",SDL_GetError());
        SDL_FreeSurface(player_surface);
        return pacWarGame_FAIL;
    }
    else
    {
        enemy_surface = SDL_DisplayFormat(temp_surface);
        if (enemy_surface == NULL)
        {
            fprintf(stderr, "Erro: Não é possível converter gráficos dos fantasmas:%s\n",SDL_GetError());
            SDL_FreeSurface(player_surface);
            return pacWarGame_FAIL;
        }
        SDL_FreeSurface(temp_surface);
    }

    temp_surface = SDL_LoadBMP(world_graphic_file);
    if (temp_surface == NULL)
    {
        fprintf(stderr, "Aviso: não é possível carregar gráficos do mapa: %s\n",SDL_GetError());
        //fprintf(stderr, " Usando o desenho básico para gráficos do mapa.\n");
        world_surface = NULL;
    }
    else
    {
        world_surface = SDL_DisplayFormat(temp_surface);
        if (world_surface == NULL)
        {
            fprintf(stderr, "Aviso: não é possível converter gráficos do mapa: %s\n",SDL_GetError());
            //fprintf(stderr, "Usando o desenho básico para gráficos do mapa.\n");
        }
        SDL_FreeSurface(temp_surface);
    }

    /** Inicialize o som e musica. **/
    if (iniSom(&music, &comendo, &menu) != 0)
    {
        SDL_FreeSurface(player_surface);
        SDL_FreeSurface(enemy_surface);
        if (world_surface != NULL)
            SDL_FreeSurface(world_surface);
        return pacWarGame_FAIL;
    }
    tocaMusica(music);


    /** Loop do jogo **/
    done_status = 1;
    while (done_status == 1)
    {

        /** Pega a entrada do jogador, verifica os cliques e define as ações relativas **/
        if (SDL_PollEvent(&event) == 1)
        {
            switch(event.type)
            {
                case SDL_QUIT: /** Sai da tela do jogo e retorna para a tela inicial **/
                    done_status = pacWarGame_QUIT;
                break;

                case SDL_KEYDOWN: /** pega evento de pressão de botão **/
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_q:
                            /** Vai para o menu em vez de sair . **/
                            done_status = pacWarGame_OK;
                        break;

                        case SDLK_ESCAPE: /** Sai do jogo **/

                            /* Para SDL 2.0 (Caixa de msg que avisa a saida do jogo ---
                            SDL_MessageBoxButtonData button_data[1] = {0};
                            button_data[0].flags    = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
                            button_data[0].buttonid = 0;
                            button_data[0].text     = "OK";

                            SDL_MessageBoxData message_box_data = {0};
                            message_box_data.flags       = 0;
                            message_box_data.window      = NULL;
                            message_box_data.title       = "Saindo";
                            message_box_data.message     = "O jogo será fechado mas seu progresso está guardado!";
                            message_box_data.numbuttons  = 1;
                            message_box_data.buttons     = button_data;
                            message_box_data.colorScheme = NULL;

                            if (SDL_ShowMessageBox(&message_box_data, &buttonid) < 0 )
                                SDL_Log("%s", SDL_GetError()); */

                            done_status = -1; /** nenhuma tecla relativa ao valor, portanto sai **/
                        break;

                        case SDLK_w:
                        case SDLK_UP:
                            player.moving_direction = pacWar_DIRECTION_UP;
                        break;

                        case SDLK_s:
                        case SDLK_DOWN:
                            player.moving_direction = pacWar_DIRECTION_DOWN;
                        break;

                        case SDLK_a:
                        case SDLK_LEFT:
                            player.moving_direction = pacWar_DIRECTION_LEFT;
                        break;

                        case SDLK_d:
                        case SDLK_RIGHT:
                            player.moving_direction = pacWar_DIRECTION_RIGHT;
                        break;

                        default:
                        break;
                    }
                break;

                case SDL_KEYUP: /** pega evento de soltar botão **/
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_w:
                        case SDLK_UP:
                            if (player.moving_direction == pacWar_DIRECTION_UP)
                            {
                                player.moving_direction = pacWar_DIRECTION_NONE;
                                player.looking_direction = pacWar_DIRECTION_UP;
                                player.speed = 0;
                            }
                        break;

                        case SDLK_s:
                        case SDLK_DOWN:
                            if (player.moving_direction == pacWar_DIRECTION_DOWN)
                            {
                                player.moving_direction = pacWar_DIRECTION_NONE;
                                player.looking_direction = pacWar_DIRECTION_DOWN;
                                player.speed = 0;
                            }
                        break;

                        case SDLK_a:
                        case SDLK_LEFT:
                            if (player.moving_direction == pacWar_DIRECTION_LEFT)
                            {
                                player.moving_direction = pacWar_DIRECTION_NONE;
                                player.looking_direction = pacWar_DIRECTION_LEFT;
                                player.speed = 0;
                            }
                        break;

                        case SDLK_d:
                        case SDLK_RIGHT:
                            if (player.moving_direction == pacWar_DIRECTION_RIGHT)
                            {
                                player.moving_direction = pacWar_DIRECTION_NONE;
                                player.looking_direction = pacWar_DIRECTION_RIGHT;
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
        switch (player.moving_direction)
        {
            case pacWar_DIRECTION_UP:
                player.speed++;
                if (player.speed > pacWar_MAX_PLAYER_SPEED)
                    player.speed--;
                player.y -= player.speed;
                /* Continue se movendo de volta passo a passo, até a borda da parede. */
                while (colisaoMapa(&player, world) != 0)
                    player.y++;
            break;

            case pacWar_DIRECTION_DOWN:
                player.speed++;
                if (player.speed > pacWar_MAX_PLAYER_SPEED)
                    player.speed--;
                player.y += player.speed;
                while (colisaoMapa(&player, world) != 0)
                    player.y--;
            break;

            case pacWar_DIRECTION_LEFT:
                player.speed++;
                if (player.speed > pacWar_MAX_PLAYER_SPEED)
                    player.speed--;
                player.x -= player.speed;
                while (colisaoMapa(&player, world) != 0)
                    player.x++;
            break;

            case pacWar_DIRECTION_RIGHT:
                player.speed++;
                if (player.speed > pacWar_MAX_PLAYER_SPEED)
                    player.speed--;
                player.x += player.speed;
                while (colisaoMapa(&player, world) != 0)
                    player.x--;
            break;

            default:
            break;
        }


        /* Mova os enemys e verifique suas colisões no mapa. */
        for (i = 0; i < pacWar_ENEMY_COUNT; i++)
        {
            if (enemy[i].killed)
                continue;

            /* Tente passar por uma abertura. */
            if ((direction = pacWar_enemy_direction_opening(&enemy[i], world)) != 0)
            {
                if (rand() % 3 == 0)
                    enemy[i].moving_direction = direction;
            }

            collision = 0;

            switch (enemy[i].moving_direction)
            {
                case pacWar_DIRECTION_UP:
                    enemy[i].speed++;
                    if (enemy[i].speed > enemy_speed)
                        enemy[i].speed--;
                    enemy[i].y -= enemy[i].speed;
                    while (colisaoMapa(&enemy[i], world) != 0)
                    {
                        enemy[i].y++;
                        collision = 1;
                    }
                    /*Volte se colidir com outro enemy. */
                    for (j = 0; j < pacWar_ENEMY_COUNT; j++)
                    {
                        if (enemy[j].killed)
                            continue;
                        if (j != i)
                        {
                            while (pacWar_character_collision(&enemy[i], &enemy[j]))
                            {
                                enemy[i].y++;
                                collision = 1;
                            }
                        }
                    }
                break;

                case pacWar_DIRECTION_DOWN:
                    enemy[i].speed++;
                    if (enemy[i].speed > enemy_speed)
                        enemy[i].speed--;
                    enemy[i].y += enemy[i].speed;
                    while (colisaoMapa(&enemy[i], world) != 0)
                    {
                        enemy[i].y--;
                        collision = 1;
                    }
                    for (j = 0; j < pacWar_ENEMY_COUNT; j++)
                    {
                        if (enemy[j].killed)
                            continue;
                        if (j != i)
                        {
                            while (pacWar_character_collision(&enemy[i], &enemy[j]))
                            {
                                enemy[i].y--;
                                collision = 1;
                            }
                        }
                    }
                break;

                case pacWar_DIRECTION_LEFT:
                    enemy[i].speed++;
                    if (enemy[i].speed > enemy_speed)
                        enemy[i].speed--;
                    enemy[i].x -= enemy[i].speed;
                    while (colisaoMapa(&enemy[i], world) != 0)
                    {
                        enemy[i].x++;
                        collision = 1;
                    }
                    for (j = 0; j < pacWar_ENEMY_COUNT; j++)
                    {
                        if (enemy[j].killed)
                            continue;
                        if (j != i)
                        {
                            while (pacWar_character_collision(&enemy[i], &enemy[j]))
                            {
                                enemy[i].x++;
                                collision = 1;
                            }
                        }
                    }
                break;

                case pacWar_DIRECTION_RIGHT:
                    enemy[i].speed++;
                    if (enemy[i].speed > enemy_speed)
                            enemy[i].speed--;
                    enemy[i].x += enemy[i].speed;
                    while (colisaoMapa(&enemy[i], world) != 0)
                    {
                        enemy[i].x--;
                        collision = 1;
                    }
                    for (j = 0; j < pacWar_ENEMY_COUNT; j++)
                    {
                        if (enemy[j].killed)
                        continue;
                        if (j != i)
                        {
                            while (pacWar_character_collision(&enemy[i], &enemy[j]))
                            {
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
            if (collision)
            {
                if (rand() % 3 == 0)
                    enemy[i].moving_direction = (rand() % 4) + 1;
                else
                {
                    if (booster_time == 0) /* Apenas passe para o jogador se não estiver bugado. */
                        pacWar_enemy_direction_player(&enemy[i], &player);
                }
            }
        }


        /** Verifique as colisões entre o jogador e os enemys. **/
        for (i = 0; i < pacWar_ENEMY_COUNT; i++)
        {
            if (enemy[i].killed)
                continue;
            if (pacWar_character_collision(&player, &enemy[i]))
            {
                if (booster_time > 0)
                {
                    enemy[i].killed = 1;
                    tocaSons(comendo);
                    *score += pacWar_SCORE_ENEMY;
                }
                else
                {
                    fprintf(stderr, "Morto pelos fantasmas.\n");
                    done_status = pacWarGame_OK;
                }
            }
        }


        /** Verificando consumo -- saia se a última pastilha tiver sido consumida */
        if (comendoPastilhas(&player, pellet, total_pellets,&all_pellets_consumed, &boost_effect) == 1)
        {
            tocaSons(comendo);
            comeCome++;
            if (all_pellets_consumed)
            {
                fprintf(stderr, "Todas a bolas comidas.\n");
                *score += 100; /* Pontuação extra para consumir todos.. */
                done_status = pacWarGame_OK;
            }
            if (boost_effect)
                booster_time = pacWar_BOOSTER_TIME;
        }
        if (booster_time > 0)
            booster_time--;


        /** Desenha o mapa */
        if (world_surface != NULL)
            desenhaMapa(screen, world_surface);
        //else
            //pacWarDesenhoworld_basic(screen, world);

        desenhaPastilhas(screen, pellet, total_pellets);
        desenhaNum(screen, number_surface, 130, 10, comeCome); /** serve para escrever a velocidade dos fantasmas **/

        desenhaPac(&player, screen, player_surface, booster_time);


        for (i = 0; i < pacWar_ENEMY_COUNT; i++)
        {
            if (enemy[i].killed)
                continue;
            desenhaFantasmas(&enemy[i], screen, enemy_surface, i, booster_time);
        }

        SDL_Flip(screen);
        SDL_Delay(pacWar_GAME_SPEED);
    }


    /* Limpar. */
    SDL_FreeSurface(number_surface);
    SDL_FreeSurface(player_surface);
    SDL_FreeSurface(enemy_surface);
    if (world_surface != NULL)
        SDL_FreeSurface(world_surface);
    Mix_CloseAudio();
    Mix_FreeMusic(music);
    Mix_FreeChunk(comendo);
    Mix_FreeChunk(menu);

    /* Atualizar pontuação. */
    *score += pastilhasConsumidas(pellet, total_pellets) *pacWar_SCORE_PELLET;
    *score *= enemy_speed;
    if (*score < 0)
        *score = 0;

    if (done_status != pacWarGame_OK)
        *score = 0; /* Resetando pontuacao */

    return done_status;
}

