/*******************************************************************
* Projeto PacMan Wars Linguagem C - LAboratorio de Programacao ll  *
* Bibliotecas ultilizadas SDL e SDL_mixer                          *
* Autores : Wanderson Luan e Antonio Dionisio                      *
* Compilador CodeBlocks + mingw + sdl _ sdl_mix                    *
********************************************************************/

/***Bibliotecas básicas***/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/***Bibliotecas SDL***/
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h> // Sons

/** definicoes **/
//tamanho do cenário
#define tamanhoBloco 32
#define tamanhoEixoX 20
#define tamanhoEixoY 15

//funcionamento do jogo
#define gameOK 0
#define gameFalhou -1
#define gameFechar -2
#define gameVelocidade 20


//características dos personagens
#define velocidadeMaxJogador 5
#define tamanhoPac 26
#define contaFantasmas 4
#define velocidadeMaxFantasmas 9

//gerais
#define tamanhoPastilha 4
#define tamanhoPastilhaEnergia 6
#define numeroMaxPastilhas 300 /* 15 x 20 */
#define tempoEnergia 120 /** Em ciclos de loop */
#define pontoPastilha 2
#define pontoFantasma -25 /** Penalidade por matar fantasmas*/

//mapa
#define mapaEspaco ' '
#define mapaParede '#'
#define mapaPac 'P'
#define mapaFantasmas 'E'
#define mapaPastilha '.'
#define mapaEnergia '*'
#define larguraNum 26 /** define a largura dos números utilizados no menu e no game **/
#define alturaNum 32 /** define a altura dos números utilizados no menu e no game **/
#define tamanhoNomeArquivo 16
#define maxPalavrasMapa 5
#define arquivoRecorde "recorde.dat"

//direções
#define direcaoNenhuma 0
#define direcaoCima 1
#define direcaoBaixo 2
#define direcaoEsquerda 3
#define direcaoDireita 4

/** Pacman "fantasias" **/
typedef struct personagem {
    int x, y; /* Coordenadas*/
    int direcaoMovimento, direcaoApontada;
    int velocidade;
    int contaImagem; /** Usado para animacao **/
    int morto;
} pacWarPersonagem;

/** pastilhas **/
typedef struct pacWarPastilhas {
    int x, y;
    int consumido;
    int efeitoEnergia;
} pacPastilhas;

/** Protótipos **/
int pacWarGame(SDL_Surface *, char *,char *, int , int *); /** função principal que controla o loop do jogo **/
int iniciaSom(Mix_Music **, Mix_Chunk **, Mix_Chunk **); /** inicializa o som **/
void tocaMusica(Mix_Music *); /** ativa musicas **/
void tocaSons(Mix_Chunk *); /** ativando outros sons **/
int leMapa(char *, char *); /** lê o arquivo layout e verifica as configurações do layout em relação á ' ', '#', '*', '.' **/
int localizaPac(char *, int *, int *); /** lê o arquivo layout e verifica onde está posicionado o pacman **/
int localizaFantasmas(char *, int *, int *, int ); /** localizando os fantasmas **/
void iniciaPac(pacWarPersonagem *, int , int ); /** Inicializa o pacman **/
void iniciaPastilhas(char *, pacPastilhas *, int *); /** inicializando as pastilhas **/
//void pacWarDesenhoworld_basic(SDL_Surface *s, char *world);
void desenhaMapa(SDL_Surface *, SDL_Surface *); /** carrega mapa a partir do bitmap **/
void desenhaPac(pacWarPersonagem *,SDL_Surface *, SDL_Surface *, int ); /** desenha o jogador **/
void desenhaFantasmas(pacWarPersonagem *, SDL_Surface *, SDL_Surface *, int , int ); /** desenha fantasmas **/
void desenhaPastilhas(SDL_Surface *, pacPastilhas *, int ); /** desenha as pastilhas **/
int colisaoMapa(pacWarPersonagem *, char *); /** Colisao NO MAPA */
int colisaoPac(pacWarPersonagem *,pacWarPersonagem *); /** colisão do Pac **/
int pastilhasConsumidas(pacPastilhas *, int ); /** pastilhas consumidas **/
int comendoPastilhas(pacWarPersonagem *, pacPastilhas *,int , int *, int *); /** colisao das pastilhas **/
void direcaoFantasmas(pacWarPersonagem *,pacWarPersonagem *); /** direcão dos Fantasmas **/
int direcaoFantasmasAbertura(pacWarPersonagem *, char *); /** direção dos fantasmas -- aberturas**/
void desenhaNum(SDL_Surface *, SDL_Surface *, int , int , int ); /** serve para escrever a velocidade dos fantasmas **/
void leRecorde(int *, char *); /** le recorde em um arquivo **/
void salvaRecorde(int *, char *); /**salva recorde em um arquivo **/



int main(int argc, char *argv[])
{
    SDL_Event event;

    /** SDL_Event é interessante ==> É uma união... União é um tipo de dados que possui ("retorna") um único
     dado entre uma variedade disponível entre seus membros, ou seja, só um membro será usado de fato, excluindo
     a possibilidade dos outros. Seu tamanho é o tamanho do maior membro. É como uma estrutura na qual todos os
     membros são armazenados no mesmo endereço. É uma forma de dinamizar o dado - um único dado que pode assumir
     várias formas -, ou seja, é útil para criar um "tipo variável".**/

    SDL_Surface *screen, *menu_surface, *number_surface, *temp_surface;
    int game_done, game_result, world_number, enemy_velocidade, game_score;
    char layout_file[tamanhoNomeArquivo];
    char graphic_file[tamanhoNomeArquivo];
    int high_score[maxPalavrasMapa];

    srand((unsigned)time(NULL));

    /** Tente carregar recordes do arquivo. **/
    leRecorde(high_score, arquivoRecorde);

    /** Vídeo e inicialização de tela aqui, por causa do menu gráfico. **/
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        printf(stderr, "Erro: não é possível inicializar o SDL: %s\n", SDL_GetError());
        return 1;
    }
    screen = SDL_SetVideoMode(tamanhoEixoX * 40/*tamanhoBloco*/,
                              tamanhoEixoY *40 /*tamanhoBloco*/, 24, SDL_DOUBLEBUF/*SDL_FULLSCREEN*/);
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
    enemy_velocidade = 1;
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
                            snprintf(layout_file, tamanhoNomeArquivo, "world.layout",world_number);
                            snprintf(graphic_file, tamanhoNomeArquivo, "world.bmp",world_number);

                            game_result = pacWarGame(screen, layout_file, graphic_file,enemy_velocidade, &game_score);

                            if (game_result == gameOK)
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
                        if (world_number > maxPalavrasMapa)
                            world_number = 1;
                        break;

                        case SDLK_s:
                            enemy_velocidade++;
                            if (enemy_velocidade > velocidadeMaxFantasmas)
                                enemy_velocidade = 1;
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
        desenhaNum(screen, number_surface, 385, 211, enemy_velocidade); /** serve para escrever a velocidade dos fantasmas **/
        desenhaNum(screen, number_surface, 385, 259,high_score[world_number - 1]); /** serve para escrever a pontuação máxima **/

        SDL_Flip(screen);
        SDL_Delay(gameVelocidade);
    }


    /* Limpar */
    SDL_FreeSurface(menu_surface);
    SDL_FreeSurface(number_surface);
    SDL_FreeSurface(screen);
    salvaRecorde(high_score, arquivoRecorde);

    return 0;
}

/** iniciando o som **/
int iniciaSom(Mix_Music **music, Mix_Chunk **comendo, Mix_Chunk **menu)
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        fprintf(stderr, "Não foi possivel abrir o áudio %s\n", SDL_GetError());
        Mix_CloseAudio();
        return -1;
    }

    if (! (*music = Mix_LoadMUS("music.wav"))) //ou use musicWars.wav
    {
        fprintf(stderr, "Não foi possível abrir o áudio: %s\n",SDL_GetError());
        Mix_CloseAudio();
        return -1;
    }

    if (! (*comendo = Mix_LoadWAV("comendo.wav")))
    {
        fprintf(stderr, "Não foi possível abrir o áudio: %s\n",SDL_GetError());
        Mix_CloseAudio();
        Mix_FreeMusic(*music);
        return -1;
    }
    if (! (*menu = Mix_LoadWAV("menuWars.wav")))
    {
        fprintf(stderr, "Não foi possível abrir o áudio: %s\n",SDL_GetError());
        Mix_CloseAudio();
        //Mix_FreeMusic(*music);
        return -1;
    }
    return 0;
}

/** ativando musica **/
void tocaMusica(Mix_Music *m)
{
    if (Mix_PlayMusic(m, 0) == -1)
        fprintf(stderr, "Não é possivel reproduzir o audio %s\n", SDL_GetError());
}


/** ativa os audios **/
void tocaSons(Mix_Chunk *a)
{
    /** pare todos os canais de audio **/
    Mix_HaltChannel(-1);

    if (Mix_PlayChannel(-1, a, 0) == -1)
        fprintf(stderr, "Não é possível reproduzir o áudio %s\n", SDL_GetError());
}


/** le o arquivo do mapa**/
int leMapa(char *arqLayout, char *world)
{
    int c, w, h;
    FILE *arq;

    arq = fopen(arqLayout, "r");
    if (arq == NULL)
    {
        fprintf(stderr, "Não é possivel abrir o'%s' para leitura \n", arqLayout);
        return -1;
    }

    w = 0;
    h = 2; // controle da posição da matriz
    while ((c = fgetc(arq)) != EOF)
    {
        if (c == '\n' || w >= tamanhoEixoX)
        {
            w = 0;
            h++;
            if (h >= tamanhoEixoY+2)
                return 0; /** limites do mapa **/
        }
        else
        {
            world[(h * tamanhoEixoX) + w] = c;
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
    for (i = 0; i < tamanhoEixoY; i++)
    {
        for (j = 0; j < tamanhoEixoX; j++)
        {
            if (world[(i * tamanhoEixoX) + j] == mapaPac)
            {
                *x = j * tamanhoBloco;
                *y = i * tamanhoBloco;
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
    for (i = 0; i < tamanhoEixoY; i++)
    {
        for (j = 0; j < tamanhoEixoX; j++)
        {
            if (world[(i * tamanhoEixoX) + j] == mapaFantasmas)
            {
                if (count == n)
                {
                    *x = j * tamanhoBloco;
                    *y = i * tamanhoBloco;
                    return 0;
                }
                count++;
            }
        }
    }
    return -1; /* nao encontrado */
}

/** Inicializa o pacman **/
void iniciaPac(pacWarPersonagem *c, int x, int y)
{
    /** iniciando coordenadas X e Y */
    c->x = x;
    c->y = y;
    c->direcaoMovimento = c->direcaoApontada = direcaoNenhuma;
    c->velocidade = 0;
    c->contaImagem = 0;
    c->morto = 0;
}

/** inicializando as pastilhas **/
void iniciaPastilhas(char *world, pacPastilhas *p, int *total)
{
    int i, j, n;
    n = 0;
    for (i = 2; i < tamanhoEixoY+2; i++)
    {
        for (j = 0; j < tamanhoEixoX; j++)
        {
            if ((world[(i * tamanhoEixoX) + j] == mapaPastilha)||(world[(i * tamanhoEixoX) + j] == mapaEnergia)) /** controla as bolas **/
            {
                p[n].x = (j * tamanhoBloco) + (tamanhoBloco / 2);
                p[n].y = (i * tamanhoBloco) + (tamanhoBloco / 2);
                p[n].consumido = 0;

                if (world[(i * tamanhoEixoX) + j] == mapaEnergia)
                    p[n].efeitoEnergia = 1;
                else
                    p[n].efeitoEnergia = 0;

                n++;
                if (n >= numeroMaxPastilhas - 1)
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
    r.w = tamanhoEixoX * tamanhoBloco;
    r.h = tamanhoEixoY * tamanhoBloco;
    SDL_FillRect(s, &r, 0x0);

    r.w = tamanhoBloco;
    r.h = tamanhoBloco;
    for (i = 0; i < tamanhoEixoY; i++)
    {
        for (j = 0; j < tamanhoEixoX; j++)
        {
            if (world[(i * tamanhoEixoX) + j] == mapaParede)
            {
                r.x = j * tamanhoBloco;
                r.y = i * tamanhoBloco;
                SDL_FillRect(s, &r, 0xffffff);
            }
        }
    }
}*/


/** criando mapa a partir do bitmap **/
void desenhaMapa(SDL_Surface *s, SDL_Surface *ws)
{
    SDL_BlitSurface(ws, NULL, s, NULL);
}

/** desenha o jogador **/
void desenhaPac(pacWarPersonagem *p,SDL_Surface *s, SDL_Surface *ps, int boosted)
{
    SDL_Rect src, dst;
    int direction;

    src.w = src.h = dst.w = dst.h = tamanhoPac;
    dst.x = p->x;
    dst.y = p->y;

    if (boosted)
        src.y = tamanhoPac;
    else
        src.y = 0;

    src.x = 0;

    if (p->direcaoMovimento == direcaoNenhuma)
        direction = p->direcaoApontada;
    else
        direction = p->direcaoMovimento;

    switch (direction) /** muda a animação do pac **/
    {
        case direcaoCima:
        break;
        case direcaoBaixo:
            src.x += (tamanhoPac * 4);
        break;

        case direcaoEsquerda:
            src.x += (tamanhoPac * 8);
        break;

        case direcaoDireita:
            src.x += (tamanhoPac * 12);
        break;

        default:
        break;
    }

    if (p->contaImagem < 4)
    {
        src.x += (p->contaImagem * tamanhoPac);
        p->contaImagem++;
    }
    else
        if (p->contaImagem == 4)
        {
            src.x += (tamanhoPac * 3);
            p->contaImagem++;
        }
        else
            if (p->contaImagem == 5)
            {
                src.x += (tamanhoPac * 2);
                p->contaImagem++;
            }
            else
                if (p->contaImagem == 6)
                {
                    src.x += tamanhoPac;
                    p->contaImagem++;
                }
                else
                    p->contaImagem = 0;

    SDL_BlitSurface(ps, &src, s, &dst); /**função que percorre a imagem e corta a partir do desejado **/
}

/** desenha fantasmas **/
void desenhaFantasmas(pacWarPersonagem *e, SDL_Surface *s, SDL_Surface *es, int texture, int boosted)
{
    SDL_Rect src, dst;

    src.w = src.h = dst.w = dst.h = tamanhoPac;
    dst.x = e->x;
    dst.y = e->y;

    if (boosted)
        src.y = tamanhoPac;
    else
        src.y = 0;

    src.x = 0;

    switch (texture)
    {
        case 0:
                /* NAda para adicionar */
        break;
        case 1:
            src.x += (tamanhoPac * 4);
        break;

        case 2:
            src.x += (tamanhoPac * 8);
        break;

        case 3:
            src.x += (tamanhoPac * 12);
        break;

        default:
        break;
    }

    src.x += (e->contaImagem * tamanhoPac);
    e->contaImagem++;
    if (e->contaImagem > 3)
        e->contaImagem = 0;

    SDL_BlitSurface(es, &src, s, &dst);
}


/** desenha as pastilhas **/
void desenhaPastilhas(SDL_Surface *s, pacPastilhas *p, int total)
{
    int i, size, color;
    SDL_Rect r;

    for (i = 0; i < total; i++)
    {
        if (p[i].consumido)
        continue;
        if (p[i].efeitoEnergia)
        {
            size = tamanhoPastilhaEnergia;
            color = 0xffff00; /* Amarelo. */
        }
        else
        {
            size = tamanhoPastilha;
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
int colisaoMapa(pacWarPersonagem *c, char *world)
{
    int cx1, cx2, cy1, cy2;

    /* Encontre em todos os blocos do mapa em que o caracter esta localizado*/
    if (c->x / tamanhoBloco == (c->x + tamanhoPac - 1) / tamanhoBloco)
    {
        /* Permanente dentro do bloco na direcao X */
        cx1 = c->x / tamanhoBloco;
        cx2 = -1;
    }
    else
    {
        /* Entre dois blocos na direcao X */
        cx1 = c->x / tamanhoBloco;
        cx2 = cx1 + 1;
    }

    if (c->y / tamanhoBloco == (c->y + tamanhoPac - 1) / tamanhoBloco)
    {
        /* Permanente dentro do bloco na direção Y. */
        cy1 = c->y / tamanhoBloco;
        cy2 = -1;
    }
    else
    {
        /* Entre dois blocos na direção Y. */
        cy1 = c->y / tamanhoBloco;
        cy2 = cy1 + 1;
    }


    /* Verifique todas as bordas de colisão em potencial. (A menos que os índices estejam fora dos limites.) */

    if ((cy1 >= 0 && cy1 < tamanhoEixoY) &&(cx1 >= 0 && cx1 < tamanhoEixoX))
    {
        if (world[(cy1 * tamanhoEixoX) + cx1] == mapaParede)
            return 1;
    }

    if ((cy2 >= 0 && cy2 < tamanhoEixoY) && (cx1 >= 0 && cx1 < tamanhoEixoX))
    {
        if (world[(cy2 * tamanhoEixoX) + cx1] == mapaParede)
            return 1;
    }

    if ((cy1 >= 0 && cy1 < tamanhoEixoY) && (cx2 >= 0 && cx2 < tamanhoEixoX))
    {
        if (world[(cy1 * tamanhoEixoX) + cx2] == mapaParede)
            return 1;
    }

    if ((cy2 >= 0 && cy2 < tamanhoEixoY) &&(cx2 >= 0 && cx2 < tamanhoEixoX))
    {
        if (world[(cy2 * tamanhoEixoX) + cx2] == mapaParede)
            return 1;
    }

    /* Manuseie o envolvimento se o caractere estiver completamente fora da área da tela. */

    if (c->y > (tamanhoEixoY * tamanhoBloco) - 1)
        c->y = 0 - tamanhoPac + 1;

    if (c->x > (tamanhoEixoX * tamanhoBloco) - 1)
        c->x = 0 - tamanhoPac + 1;

    if (c->y < 0 - tamanhoPac + 1)
        c->y = (tamanhoEixoY * tamanhoBloco) - 1;

    if (c->x < 0 - tamanhoPac + 1)
        c->x = (tamanhoEixoX * tamanhoBloco) - 1;

    return 0; /* Nao ha colisao */
}

/** colisão do Pac **/
int colisaoPac(pacWarPersonagem *c1,pacWarPersonagem *c2)
{
    if (c1->y >= c2->y - tamanhoPac && c1->y <= c2->y + tamanhoPac)
        if (c1->x >= c2->x - tamanhoPac && c1->x <= c2->x + tamanhoPac)
            return 1;
    return 0; /* Nao ha colisao */
}

/** pastilhas consumidas **/
int pastilhasConsumidas(pacPastilhas *p, int total)
{
    int i, consumido;

    consumido = 0;
    for (i = 0; i < total; i++)
        if (p[i].consumido)
            consumido++;

    return consumido;
}



/** colisao das pastilhas **/
int comendoPastilhas(pacWarPersonagem *c, pacPastilhas *p,int total, int *todasPastilhasConsumidas, int *efeitoEnergia)
{
    int i, consumido, collision;

    *todasPastilhasConsumidas = 0;
    *efeitoEnergia = 0;

    collision = 0;
    consumido = 0;
    for (i = 0; i < total; i++)
    {
        if (p[i].consumido)
        {
            consumido++;
            continue;
        }

        if (c->y >= p[i].y - tamanhoPac && c->y <= p[i].y)
        {
            if (c->x >= p[i].x - tamanhoPac && c->x <= p[i].x)
            {
                p[i].consumido = 1;
                if (p[i].efeitoEnergia)
                    *efeitoEnergia = 1;
                consumido++;
                collision = 1;
            }
        }
    }

    if (consumido == total)
        *todasPastilhasConsumidas = 1;

    if (collision)
        return 1;
    else
        return 0;
}


/** direcão dos Fantasmas **/
void direcaoFantasmas(pacWarPersonagem *e,pacWarPersonagem *p) /** direção dos fantasmas **/
{
    /* Direção da base na localização do jogador. */
    if (e->y > p->y - tamanhoBloco && e->y < p->y + tamanhoBloco)
    {
        if (e->x > p->x)
            e->direcaoMovimento = direcaoEsquerda;
        else
            e->direcaoMovimento = direcaoDireita;
    }
    else
    {
        if (e->y > p->y)
            e->direcaoMovimento = direcaoCima;
        else
            e->direcaoMovimento = direcaoBaixo;
    }
}



/** direção dos fantasmas -- aberturas**/
int direcaoFantasmasAbertura(pacWarPersonagem *e, char *world)
{
    int x, y;

    /* Verifique se está dentro do bloco em ambas as direções. */
    if (e->x / tamanhoBloco == (e->x + tamanhoPac - 1) / tamanhoBloco)
    {
        x = e->x / tamanhoBloco;
        if (e->y / tamanhoBloco == (e->y + tamanhoPac - 1) / tamanhoBloco)
        {
            y = e->y / tamanhoBloco;

            /* Permitir somente se dentro dos limites menos 1. */
            if (y < tamanhoEixoY - 1 && x < tamanhoEixoX - 1)
            {
                if (world[(y * tamanhoEixoX) + x + 1] != mapaParede &&world[(y * tamanhoEixoX) + x - 1] == mapaParede)
                    return direcaoDireita;

                if (world[(y * tamanhoEixoX) + x - 1] != mapaParede && world[(y * tamanhoEixoX) + x + 1] == mapaParede)
                    return direcaoEsquerda;

                if (world[((y + 1) * tamanhoEixoX) + x] != mapaParede && world[((y - 1) * tamanhoEixoX) + x] == mapaParede)
                    return direcaoBaixo;

                if (world[((y - 1) * tamanhoEixoX) + x] != mapaParede && world[((y + 1) * tamanhoEixoX) + x] == mapaParede)
                    return direcaoCima;
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


void leRecorde(int *v, char *arquivo)
{
    int i;
    FILE *arq;

    arq = fopen(arquivo, "r");
    if (arq == NULL)
    {
        fprintf(stderr, "Não foi carregar o arquivo de recordes.\n");
        /* zera todos os recordes */
        for (i = 0; i < maxPalavrasMapa; i++)
        {
            v[i] = 0;
        }
    }
    else
    {
        fread(v, sizeof(int), maxPalavrasMapa, arq);
        fclose(arq);
    }
}

void salvaRecorde(int *v, char *arquivo)
{
    FILE *arq;

    arq = fopen(arquivo, "w");
    if (arq == NULL)
    {
        fprintf(stderr, "Erro ao salvar o arquivo de recordes.\n");
    }
    else
    {
        fwrite(v, sizeof(int), maxPalavrasMapa, arq);
        fclose(arq);
    }
}

/** função principal que controla o loop do jogo **/
int pacWarGame(SDL_Surface *screen, char *world_layout_file, char *world_graphic_file, int enemy_velocidade, int *score)
{
    int i, j, temp_x, temp_y, collision, direction, done_status;
    SDL_Event event;
    SDL_Surface *player_surface, *enemy_surface, *world_surface, *temp_surface, *number_surface;

    /** definindo variáveis de som **/
    Mix_Music *music;
    Mix_Chunk *comendo;
    Mix_Chunk *menu;

    char world[(tamanhoEixoX +20)* (tamanhoEixoY + 20)] = {mapaEspaco};

    pacWarPersonagem player, enemy[contaFantasmas];
    pacPastilhas pellet[numeroMaxPastilhas];

    int total_pellets, efeitoEnergia, todasPastilhasConsumidas, booster_time, comeCome = 0;

    number_surface = SDL_DisplayFormat(SDL_LoadBMP("number.bmp"));

    //comeCome = pacWar_pellet_collision(&player, pellet, total_pellets,&todasPastilhasConsumidas, &efeitoEnergia);

    if (leMapa(world_layout_file, world) != 0)
    {
        fprintf(stderr, "Erro: Não foi possível carregar o arquivo de layout do mapa.\n");
        return gameFalhou;
    }

    if (localizaPac(world, &temp_x, &temp_y) != 0)
    {
        fprintf(stderr, "Não foi possível localizar o pacman no arquivo de layout do mapa.\n");
        return gameFalhou;
    }
    else
    {
        iniciaPac(&player, temp_x, temp_y);
    }

    for (i = 0; i < contaFantasmas; i++)
    {
        if (localizaFantasmas(world, &temp_x, &temp_y, i) == 0)
            iniciaPac(&enemy[i], temp_x, temp_y);
            /* Comece com a direção de movimento aleatório. */
        enemy[i].direcaoMovimento = (rand() % 4) + 1;
    }

    iniciaPastilhas(world, pellet, &total_pellets);
    *score = 0;
    efeitoEnergia = todasPastilhasConsumidas = booster_time = 0;

    /* Carregar e converter arquivos gráficos. */
    temp_surface = SDL_LoadBMP("player.bmp"); /** carrega o pacman **/
    if (temp_surface == NULL)
    {
        fprintf(stderr, "Erro: Não é possível carregar gráficos do pacman: %s\n",SDL_GetError());
        return gameFalhou;
    }
    else
    {
        player_surface = SDL_DisplayFormat(temp_surface);
        if (player_surface == NULL)
        {
            fprintf(stderr, "Erro: Não é possível converter os gráficos do pacman: %s\n",SDL_GetError());
            return gameFalhou;
        }
        SDL_FreeSurface(temp_surface);
    }

    temp_surface = SDL_LoadBMP("enemy.bmp"); /** carrega os fantasmas **/
    if (temp_surface == NULL)
    {
        fprintf(stderr, "Erro: Não é possível carregar gráficos dos fantasmas: %s\n",SDL_GetError());
        SDL_FreeSurface(player_surface);
        return gameFalhou;
    }
    else
    {
        enemy_surface = SDL_DisplayFormat(temp_surface);
        if (enemy_surface == NULL)
        {
            fprintf(stderr, "Erro: Não é possível converter gráficos dos fantasmas:%s\n",SDL_GetError());
            SDL_FreeSurface(player_surface);
            return gameFalhou;
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
    if (iniciaSom(&music, &comendo, &menu) != 0)
    {
        SDL_FreeSurface(player_surface);
        SDL_FreeSurface(enemy_surface);
        if (world_surface != NULL)
            SDL_FreeSurface(world_surface);
        return gameFalhou;
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
                    done_status = gameFechar;
                break;

                case SDL_KEYDOWN: /** pega evento de pressão de botão **/
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_q:
                            /** Vai para o menu em vez de sair . **/
                            done_status = gameOK;
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
                            player.direcaoMovimento = direcaoCima;
                        break;

                        case SDLK_s:
                        case SDLK_DOWN:
                            player.direcaoMovimento = direcaoBaixo;
                        break;

                        case SDLK_a:
                        case SDLK_LEFT:
                            player.direcaoMovimento = direcaoEsquerda;
                        break;

                        case SDLK_d:
                        case SDLK_RIGHT:
                            player.direcaoMovimento = direcaoDireita;
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
                            if (player.direcaoMovimento == direcaoCima)
                            {
                                player.direcaoMovimento = direcaoNenhuma;
                                player.direcaoApontada = direcaoCima;
                                player.velocidade = 0;
                            }
                        break;

                        case SDLK_s:
                        case SDLK_DOWN:
                            if (player.direcaoMovimento == direcaoBaixo)
                            {
                                player.direcaoMovimento = direcaoNenhuma;
                                player.direcaoApontada = direcaoBaixo;
                                player.velocidade = 0;
                            }
                        break;

                        case SDLK_a:
                        case SDLK_LEFT:
                            if (player.direcaoMovimento == direcaoEsquerda)
                            {
                                player.direcaoMovimento = direcaoNenhuma;
                                player.direcaoApontada = direcaoEsquerda;
                                player.velocidade = 0;
                            }
                        break;

                        case SDLK_d:
                        case SDLK_RIGHT:
                            if (player.direcaoMovimento == direcaoDireita)
                            {
                                player.direcaoMovimento = direcaoNenhuma;
                                player.direcaoApontada = direcaoDireita;
                                player.velocidade = 0;
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
        switch (player.direcaoMovimento)
        {
            case direcaoCima:
                player.velocidade++;
                if (player.velocidade > velocidadeMaxJogador)
                    player.velocidade--;
                player.y -= player.velocidade;
                /* Continue se movendo de volta passo a passo, até a borda da parede. */
                while (colisaoMapa(&player, world) != 0)
                    player.y++;
            break;

            case direcaoBaixo:
                player.velocidade++;
                if (player.velocidade > velocidadeMaxJogador)
                    player.velocidade--;
                player.y += player.velocidade;
                while (colisaoMapa(&player, world) != 0)
                    player.y--;
            break;

            case direcaoEsquerda:
                player.velocidade++;
                if (player.velocidade > velocidadeMaxJogador)
                    player.velocidade--;
                player.x -= player.velocidade;
                while (colisaoMapa(&player, world) != 0)
                    player.x++;
            break;

            case direcaoDireita:
                player.velocidade++;
                if (player.velocidade > velocidadeMaxJogador)
                    player.velocidade--;
                player.x += player.velocidade;
                while (colisaoMapa(&player, world) != 0)
                    player.x--;
            break;

            default:
            break;
        }


        /* Mova os enemys e verifique suas colisões no mapa. */
        for (i = 0; i < contaFantasmas; i++)
        {
            if (enemy[i].morto)
                continue;

            /* Tente passar por uma abertura. */
            if ((direction = direcaoFantasmasAbertura(&enemy[i], world)) != 0)
            {
                if (rand() % 3 == 0)
                    enemy[i].direcaoMovimento = direction;
            }

            collision = 0;

            switch (enemy[i].direcaoMovimento)
            {
                case direcaoCima:
                    enemy[i].velocidade++;
                    if (enemy[i].velocidade > enemy_velocidade)
                        enemy[i].velocidade--;
                    enemy[i].y -= enemy[i].velocidade;
                    while (colisaoMapa(&enemy[i], world) != 0)
                    {
                        enemy[i].y++;
                        collision = 1;
                    }
                    /*Volte se colidir com outro enemy. */
                    for (j = 0; j < contaFantasmas; j++)
                    {
                        if (enemy[j].morto)
                            continue;
                        if (j != i)
                        {
                            while (colisaoPac(&enemy[i], &enemy[j]))
                            {
                                enemy[i].y++;
                                collision = 1;
                            }
                        }
                    }
                break;

                case direcaoBaixo:
                    enemy[i].velocidade++;
                    if (enemy[i].velocidade > enemy_velocidade)
                        enemy[i].velocidade--;
                    enemy[i].y += enemy[i].velocidade;
                    while (colisaoMapa(&enemy[i], world) != 0)
                    {
                        enemy[i].y--;
                        collision = 1;
                    }
                    for (j = 0; j < contaFantasmas; j++)
                    {
                        if (enemy[j].morto)
                            continue;
                        if (j != i)
                        {
                            while (colisaoPac(&enemy[i], &enemy[j]))
                            {
                                enemy[i].y--;
                                collision = 1;
                            }
                        }
                    }
                break;

                case direcaoEsquerda:
                    enemy[i].velocidade++;
                    if (enemy[i].velocidade > enemy_velocidade)
                        enemy[i].velocidade--;
                    enemy[i].x -= enemy[i].velocidade;
                    while (colisaoMapa(&enemy[i], world) != 0)
                    {
                        enemy[i].x++;
                        collision = 1;
                    }
                    for (j = 0; j < contaFantasmas; j++)
                    {
                        if (enemy[j].morto)
                            continue;
                        if (j != i)
                        {
                            while (colisaoPac(&enemy[i], &enemy[j]))
                            {
                                enemy[i].x++;
                                collision = 1;
                            }
                        }
                    }
                break;

                case direcaoDireita:
                    enemy[i].velocidade++;
                    if (enemy[i].velocidade > enemy_velocidade)
                            enemy[i].velocidade--;
                    enemy[i].x += enemy[i].velocidade;
                    while (colisaoMapa(&enemy[i], world) != 0)
                    {
                        enemy[i].x--;
                        collision = 1;
                    }
                    for (j = 0; j < contaFantasmas; j++)
                    {
                        if (enemy[j].morto)
                        continue;
                        if (j != i)
                        {
                            while (colisaoPac(&enemy[i], &enemy[j]))
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
                    enemy[i].direcaoMovimento = (rand() % 4) + 1;
                else
                {
                    if (booster_time == 0) /* Apenas passe para o jogador se não estiver bugado. */
                        direcaoFantasmas(&enemy[i], &player);
                }
            }
        }


        /** Verifique as colisões entre o jogador e os enemys. **/
        for (i = 0; i < contaFantasmas; i++)
        {
            if (enemy[i].morto)
                continue;
            if (colisaoPac(&player, &enemy[i]))
            {
                if (booster_time > 0)
                {
                    enemy[i].morto = 1;
                    tocaSons(comendo);
                    *score += pontoFantasma;
                }
                else
                {
                    fprintf(stderr, "Morto pelos fantasmas.\n");
                    done_status = gameOK;
                }
            }
        }


        /** Verificando consumo -- saia se a última pastilha tiver sido consumida */
        if (comendoPastilhas(&player, pellet, total_pellets,&todasPastilhasConsumidas, &efeitoEnergia) == 1)
        {
            tocaSons(comendo);
            comeCome++;
            if (todasPastilhasConsumidas)
            {
                fprintf(stderr, "Todas a bolas comidas.\n");
                *score += 100; /* Pontuação extra para consumir todos.. */
                done_status = gameOK;
            }
            if (efeitoEnergia)
                booster_time = tempoEnergia;
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


        for (i = 0; i < contaFantasmas; i++)
        {
            if (enemy[i].morto)
                continue;
            desenhaFantasmas(&enemy[i], screen, enemy_surface, i, booster_time);
        }

        SDL_Flip(screen);
        SDL_Delay(gameVelocidade);
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
    *score += pastilhasConsumidas(pellet, total_pellets) *pontoPastilha;
    *score *= enemy_velocidade;
    if (*score < 0)
        *score = 0;

    if (done_status != gameOK)
        *score = 0; /* Resetando pontuacao */

    return done_status;
}

