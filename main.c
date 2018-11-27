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
#define jogoOk 0
#define jagoFalhou -1
#define jogoFechar -2
#define jogoVelocidade 20

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
#define larguraNum 30 /** define a largura dos números utilizados no menu e no game **/
#define alturaNum 37 /** define a altura dos números utilizados no menu e no game **/
#define tamanhoNomeArquivo 16
#define maxMapas 2
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
void pausar()

int main(int argc, char *argv[])
{
    SDL_Event event;

    /** SDL_Event é interessante ==> É uma união... União é um tipo de dados que possui ("retorna") um único
     dado entre uma variedade disponível entre seus membros, ou seja, só um membro será usado de fato, excluindo
     a possibilidade dos outros. Seu tamanho é o tamanho do maior membro. É como uma estrutura na qual todos os
     membros são armazenados no mesmo endereço. É uma forma de dinamizar o dado - um único dado que pode assumir
     várias formas -, ou seja, é útil para criar um "tipo variável".**/

    SDL_Surface *screen, *areaMenu, *areaNum, *areaTemporaria, *areaCredito;
    int fimJogo, jogoResultado, numMapa, fantasma_velocidade, jogoPontos;
    char arquivoLayout[tamanhoNomeArquivo];
    char arquivoGrafico[tamanhoNomeArquivo];
    int maxPontos[maxMapas];

    srand((unsigned)time(NULL));

    /** Tente carregar recordes do arquivo. **/
    leRecorde(maxPontos, arquivoRecorde);

    /** Vídeo e inicialização de tela aqui, por causa do menu gráfico. **/
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        printf(stderr, "Não é possível inicializar o SDL: %s\n", SDL_GetError());
        return 1;
    }
    screen = SDL_SetVideoMode(tamanhoEixoX * 40/*tamanhoBloco*/,
                              tamanhoEixoY *40 /*tamanhoBloco*/, 24, /*SDL_DOUBLEBUF*/SDL_FULLSCREEN);
                              /** controla a janela aberta **/

    if (screen == NULL)
    {
        fprintf(stderr, "Não é possível definir o modo de vídeo: %s\n", SDL_GetError());
        return 1;
    }
    //SDL_WM_SetCaption("SDL-Man", "SDL-Man");


    /** Carregar e converter arquivos gráficos de menu. **/
    areaTemporaria = SDL_LoadBMP("menu.bmp");

    if (areaTemporaria == NULL)
    {
        fprintf(stderr, "Não é possível carregar os gráficos do menu: %s\n",SDL_GetError());
        return 1;
    }
    else
    {
        areaMenu = SDL_DisplayFormat(areaTemporaria);
        if (areaMenu == NULL)
        {
            fprintf(stderr, "Não é possível converter os gráficos do menu: %s\n", SDL_GetError());
            return 1;
        }
        SDL_FreeSurface(areaTemporaria);
    }

    areaTemporaria = SDL_LoadBMP("number.bmp");

    if (areaTemporaria == NULL)
    {
        fprintf(stderr, "Não é possível carregar gráficos numéricos: %s\n",SDL_GetError());
        SDL_FreeSurface(areaMenu);
        return 1;
    }
    else
    {
        areaNum = SDL_DisplayFormat(areaTemporaria);
        if (areaNum == NULL)
        {
            fprintf(stderr, "Não é possível converter gráficos numéricos: %s\n",SDL_GetError());
            SDL_FreeSurface(areaMenu);
            return 1;
        }
        SDL_FreeSurface(areaTemporaria);
    }



    /** Inicializando valores na tela de Menu **/
    fantasma_velocidade = 1;
    numMapa = 1;


    /** Menu **/
    fimJogo = 0;
    while (! fimJogo)
    {


        /** processo para entrada **/
        if (SDL_PollEvent(&event) == 1) /** SDL_PollEvent ==> Aguarda a definição do usuário: 0 se event for NULL **/
        {
            switch(event.type) /** Acessando um dado do tipo "uint8_t" -- inteiro de 8 bites sem sinal (0 à 255) da biblioteca stdinc.h**/
            {
                case SDL_QUIT: /**Sai do jogo -- SDL_QUIT retorna um inteiro (12) para event.type indicando a saída do jogo**/
                    fimJogo = 1;
                break;

                case SDL_KEYDOWN: /** acessa a tecla pressionada pelo usuário -- retorna 3 para event.type **/
                    switch (event.key.keysym.sym) /** muda o foco de SDL_Event para o tipo SDL_KeyboardEvent **/
                    {                             /** sym é um tipo de dado "enum" que parece uma struct porém tem todos os valores constantes **/
                        case SDLK_ESCAPE: /** tecla ESC para fechar -- retorna 27 para event.key em SDL_Event**/
                            fimJogo = 1;
                        break;

                        case SDLK_RETURN: /** tecla ENTER com mesmo efeito de ESPAÇO (abaixo) -- retorna 13 para event.key em SDL_Event **/
                        case SDLK_SPACE: /** tecla ESPAÇO para "rodar" o jogo -- retorna 32 para event.key em SDL_Event **/
                            snprintf(arquivoLayout, tamanhoNomeArquivo, "world.layout",numMapa);
                            snprintf(arquivoGrafico, tamanhoNomeArquivo, "world.bmp",numMapa);

                            jogoResultado = pacWarGame(screen, arquivoLayout, arquivoGrafico,fantasma_velocidade, &jogoPontos);

                            if (jogoResultado == jogoOk)
                            {
                                /* Atualizar pontuação*/
                                if (jogoPontos > maxPontos[numMapa - 1])
                                    maxPontos[numMapa - 1] = jogoPontos;
                            }
                            else
                            {
                                fimJogo = 1; /* falhou . */
                            }
                        break;


                        case SDLK_w: /** seleciona mapa **/
                            /*if(numMapa ==1)
                                numMapa = 2;
                            if (numMapa > maxMapas)
                                numMapa = 1;*/
                            areaTemporaria = SDL_LoadBMP("creditos.bmp");
                            //areaCredito = SDL_DisplayFormat(areaTemporaria);
                            SDL_BlitSurface(SDL_DisplayFormat(areaTemporaria), NULL, screen, NULL);

                        break;

                        case SDLK_s: /** seleciona mapa **/
                            fantasma_velocidade++;
                            if (fantasma_velocidade > velocidadeMaxFantasmas)
                                fantasma_velocidade = 1;
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
        SDL_BlitSurface(areaMenu, NULL, screen, NULL);
        //pacWarDesenhomenu_number(screen, areaNum, 385, 163, numMapa);
        desenhaNum(screen, areaNum, 480, 278, fantasma_velocidade); /** serve para escrever a velocidade dos fantasmas **/
        desenhaNum(screen, areaNum, 480, 328,maxPontos[numMapa - 1]); /** serve para escrever a pontuação máxima **/

        SDL_Flip(screen);
        SDL_Delay(jogoVelocidade);
    }
    /* Limpar */
    SDL_FreeSurface(areaMenu);
    SDL_FreeSurface(areaNum);
    SDL_FreeSurface(screen);
    salvaRecorde(maxPontos, arquivoRecorde);

    return 0;
}

void pausar(int flag)
{
    if(flag == 0)
        flag = 1;
    else
        flag =0;
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

/** localizando o Pac **/
int localizaPac(char *v, int *x, int *y)
{
    int i, j;
    for (i = 0; i < tamanhoEixoY; i++)
    {
        for (j = 0; j < tamanhoEixoX; j++)
        {
            if (v[(i * tamanhoEixoX) + j] == mapaPac)
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
/** criando mapa a partir do bitmap **/
void desenhaMapa(SDL_Surface *s, SDL_Surface *ws)
{
    SDL_BlitSurface(ws, NULL, s, NULL);
}

/** desenha o Pac **/
void desenhaPac(pacWarPersonagem *p,SDL_Surface *s, SDL_Surface *ps, int energizado)
{
    SDL_Rect src, dst;
    int direcao;

    src.w = src.h = dst.w = dst.h = tamanhoPac;
    dst.x = p->x;
    dst.y = p->y;

    if (energizado)
        src.y = tamanhoPac;
    else
        src.y = 0;

    src.x = 0;

    if (p->direcaoMovimento == direcaoNenhuma)
        direcao = p->direcaoApontada;
    else
        direcao = p->direcaoMovimento;

    switch (direcao) /** muda a animação do pac **/
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
    int i, tam, cor;
    SDL_Rect r;

    for (i = 0; i < total; i++)
    {
        if (p[i].consumido)
        continue;
        if (p[i].efeitoEnergia)
        {
            tam = tamanhoPastilhaEnergia;
            cor = 0xffff00; /* Amarelo. */
        }
        else
        {
            tam = tamanhoPastilha;
            cor = 0xffffff; /* Branco */
        }
        r.w = tam;
        r.h = tam;
        r.x = p[i].x - (tam / 2);
        r.y = p[i].y - (tam / 2);
        SDL_FillRect(s, &r, cor);
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
    int i, consumido, colisao;

    *todasPastilhasConsumidas = 0;
    *efeitoEnergia = 0;

    colisao = 0;
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
                consumido+=50;
                colisao = 1;
            }
        }
    }

    if (consumido == total)
        *todasPastilhasConsumidas = 1;

    if (colisao)
        return 1;
    else
        return 0;
}


/** direcão dos Fantasmas **/
void direcaoFantasmas(pacWarPersonagem *e,pacWarPersonagem *p)
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
int direcaoFantasmasAbertura(pacWarPersonagem *e, char *v)
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
                if (v[(y * tamanhoEixoX) + x + 1] != mapaParede &&v[(y * tamanhoEixoX) + x - 1] == mapaParede)
                    return direcaoDireita;

                if (v[(y * tamanhoEixoX) + x - 1] != mapaParede && v[(y * tamanhoEixoX) + x + 1] == mapaParede)
                    return direcaoEsquerda;

                if (v[((y + 1) * tamanhoEixoX) + x] != mapaParede && v[((y - 1) * tamanhoEixoX) + x] == mapaParede)
                    return direcaoBaixo;

                if (v[((y - 1) * tamanhoEixoX) + x] != mapaParede && v[((y + 1) * tamanhoEixoX) + x] == mapaParede)
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

/** teste **/
/** desenha (anima) a vida do Pac **/
void desenhaVida(SDL_Surface *s, SDL_Surface *ns, int x, int y, int vida)
{
    SDL_Rect src, dst; /** struct que determina a altura, largura, posição x e posição y **/

    src.w = dst.w = 111;
    src.h = dst.h = 37;

    dst.x = x;
    dst.y = y;
    src.y = 0;

    switch (vida)
    {
        case 3:
          src.x = 0;
          break;
        case 2:
          src.x = 37;
          break;
        case 1:
          src.x = 74;
          break;
        case 0:
          src.x = 111;
          break;
        default:
          return; /* Não encontrado, apenas retorne. */
    }

    SDL_BlitSurface(ns, &src, s, &dst);

    dst.x -= 37;
}



void leRecorde(int *v, char *arquivo)
{
    int i;
    FILE *arq;

    arq = fopen(arquivo, "r");
    if (arq == NULL)
    {
        fprintf(stderr, "Não foi carregar o arquivo de recordes.\n");
    }
    else
    {
        fread(v, sizeof(int), maxMapas, arq);
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
        fwrite(v, sizeof(int), maxMapas, arq);
        fclose(arq);
    }
}

/** função principal que controla o loop do jogo **/
int pacWarGame(SDL_Surface *screen, char *mapaArquivoLayout, char *arquivoMapaGrafico, int fantasma_velocidade, int *pontos)
{
    int i, j, temp_x, temp_y, colisao, direction, jogoStatus, vida = 3,aux_x, aux_y;
    SDL_Event event;
    SDL_Surface *areaPac, *areaFantasma, *areaMapa, *areaTemporaria, *areaNum, *areaNumVida;

    /** definindo variáveis de som **/
    Mix_Music *music;
    Mix_Chunk *comendo;
    Mix_Chunk *menu;

    char world[(tamanhoEixoX +20)* (tamanhoEixoY + 20)] = {mapaEspaco};

    pacWarPersonagem personagemPac, fantasma[contaFantasmas];
    pacPastilhas pastilha[numeroMaxPastilhas];

    int total_pastilhas, efeitoEnergia, todasPastilhasConsumidas, tempo_Energia, comeCome = 0;

    areaNum = SDL_DisplayFormat(SDL_LoadBMP("number.bmp"));
    areaNumVida = SDL_DisplayFormat(SDL_LoadBMP("numVida.bmp"));

    //comeCome = pacWar_pastilha_colisao(&personagemPac, pastilha, total_pastilhas,&todasPastilhasConsumidas, &efeitoEnergia);

    if (leMapa(mapaArquivoLayout, world) != 0)
    {
        fprintf(stderr, "Não foi possível carregar o arquivo de layout do mapa.\n");
        return jagoFalhou;
    }

    if (localizaPac(world, &temp_x, &temp_y) != 0)
    {
        fprintf(stderr, "Não foi possível localizar o pacman no arquivo de layout do mapa.\n");
        return jagoFalhou;
    }
    else
    {
        iniciaPac(&personagemPac, temp_x, temp_y);
        aux_x = temp_x;
        aux_y = temp_y;
    }

    for (i = 0; i < contaFantasmas; i++)
    {
        if (localizaFantasmas(world, &temp_x, &temp_y, i) == 0)
            iniciaPac(&fantasma[i], temp_x, temp_y);
            /* Comece com a direção de movimento aleatório. */
        fantasma[i].direcaoMovimento = (rand() % 4) + 1;
    }

    iniciaPastilhas(world, pastilha, &total_pastilhas);
    *pontos = 0;
    efeitoEnergia = todasPastilhasConsumidas = tempo_Energia = 0;

    /* Carregar e converter arquivos gráficos. */
    areaTemporaria = SDL_LoadBMP("personagemPac.bmp"); /** carrega o pacman **/
    if (areaTemporaria == NULL)
    {
        fprintf(stderr, "Não é possível carregar gráficos do pacman: %s\n",SDL_GetError());
        return jagoFalhou;
    }
    else
    {
        areaPac = SDL_DisplayFormat(areaTemporaria);
        if (areaPac == NULL)
        {
            fprintf(stderr, "Não é possível converter os gráficos do pacman: %s\n",SDL_GetError());
            return jagoFalhou;
        }
        SDL_FreeSurface(areaTemporaria);
    }

    areaTemporaria = SDL_LoadBMP("fantasma.bmp"); /** carrega os fantasmas **/
    if (areaTemporaria == NULL)
    {
        printf("Não é possível carregar gráficos dos fantasmas: %s\n",SDL_GetError());
        SDL_FreeSurface(areaPac);
        return jagoFalhou;
    }
    else
    {
        areaFantasma = SDL_DisplayFormat(areaTemporaria);
        if (areaFantasma == NULL)
        {
            fprintf(stderr, "Não é possível converter gráficos dos fantasmas:%s\n",SDL_GetError());
            SDL_FreeSurface(areaPac);
            return jagoFalhou;
        }
        SDL_FreeSurface(areaTemporaria);
    }

    areaTemporaria = SDL_LoadBMP(arquivoMapaGrafico);
    if (areaTemporaria == NULL)
    {
        fprintf(stderr, "Não é possível carregar gráficos do mapa: %s\n",SDL_GetError());
        areaMapa = NULL;
    }
    else
    {
        areaMapa = SDL_DisplayFormat(areaTemporaria);
        if (areaMapa == NULL)
        {
            fprintf(stderr, "Não é possível converter gráficos do mapa: %s\n",SDL_GetError());
        }
        SDL_FreeSurface(areaTemporaria);
    }

    /** Inicialize o som e musica. **/
    if (iniciaSom(&music, &comendo, &menu) != 0)
    {
        SDL_FreeSurface(areaPac);
        SDL_FreeSurface(areaFantasma);
        if (areaMapa != NULL)
            SDL_FreeSurface(areaMapa);
        return jagoFalhou;
    }
    tocaMusica(music);


    /** Loop do jogo **/
    jogoStatus = 1;

    while (jogoStatus == 1)
    {

        /** Pega a entrada do jogador, verifica os cliques e define as ações relativas **/
        if (SDL_PollEvent(&event) == 1)
        {
            switch(event.type)
            {
                case SDL_QUIT: /** Sai da tela do jogo e retorna para a tela inicial **/
                    jogoStatus = jogoFechar;
                break;

                case SDL_KEYDOWN: /** pega evento de pressão de botão **/
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_q:
                            /** Vai para o menu em vez de sair . **/
                            jogoStatus = jogoOk;
                        break;

                        case SDLK_p:
                            if(flag == 0)
                                flag = 1;
                            else
                                flag =0;
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

                            jogoStatus = -1; /** nenhuma tecla relativa ao valor, portanto sai **/
                        break;

                        case SDLK_w:
                        case SDLK_UP:
                            personagemPac.direcaoMovimento = direcaoCima;
                        break;

                        case SDLK_s:
                        case SDLK_DOWN:
                            personagemPac.direcaoMovimento = direcaoBaixo;
                        break;

                        case SDLK_a:
                        case SDLK_LEFT:
                            personagemPac.direcaoMovimento = direcaoEsquerda;
                        break;

                        case SDLK_d:
                        case SDLK_RIGHT:
                            personagemPac.direcaoMovimento = direcaoDireita;
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
                            if (personagemPac.direcaoMovimento == direcaoCima)
                            {
                                personagemPac.direcaoMovimento = direcaoNenhuma;
                                personagemPac.direcaoApontada = direcaoCima;
                                personagemPac.velocidade = 0;
                            }
                        break;

                        case SDLK_s:
                        case SDLK_DOWN:
                            if (personagemPac.direcaoMovimento == direcaoBaixo)
                            {
                                personagemPac.direcaoMovimento = direcaoNenhuma;
                                personagemPac.direcaoApontada = direcaoBaixo;
                                personagemPac.velocidade = 0;
                            }
                        break;

                        case SDLK_a:
                        case SDLK_LEFT:
                            if (personagemPac.direcaoMovimento == direcaoEsquerda)
                            {
                                personagemPac.direcaoMovimento = direcaoNenhuma;
                                personagemPac.direcaoApontada = direcaoEsquerda;
                                personagemPac.velocidade = 0;
                            }
                        break;

                        case SDLK_d:
                        case SDLK_RIGHT:
                            if (personagemPac.direcaoMovimento == direcaoDireita)
                            {
                                personagemPac.direcaoMovimento = direcaoNenhuma;
                                personagemPac.direcaoApontada = direcaoDireita;
                                personagemPac.velocidade = 0;
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
        switch (personagemPac.direcaoMovimento)
        {
            case direcaoCima:
                personagemPac.velocidade++;
                if (personagemPac.velocidade > velocidadeMaxJogador)
                    personagemPac.velocidade--;
                personagemPac.y -= personagemPac.velocidade;
                /* Continue se movendo de volta passo a passo, até a borda da parede. */
                while (colisaoMapa(&personagemPac, world) != 0)
                    personagemPac.y++;
            break;

            case direcaoBaixo:
                personagemPac.velocidade++;
                if (personagemPac.velocidade > velocidadeMaxJogador)
                    personagemPac.velocidade--;
                personagemPac.y += personagemPac.velocidade;
                while (colisaoMapa(&personagemPac, world) != 0)
                    personagemPac.y--;
            break;

            case direcaoEsquerda:
                personagemPac.velocidade++;
                if (personagemPac.velocidade > velocidadeMaxJogador)
                    personagemPac.velocidade--;
                personagemPac.x -= personagemPac.velocidade;
                while (colisaoMapa(&personagemPac, world) != 0)
                    personagemPac.x++;
            break;

            case direcaoDireita:
                personagemPac.velocidade++;
                if (personagemPac.velocidade > velocidadeMaxJogador)
                    personagemPac.velocidade--;
                personagemPac.x += personagemPac.velocidade;
                while (colisaoMapa(&personagemPac, world) != 0)
                    personagemPac.x--;
            break;

            default:
            break;
        }


        /* Mova os fantasmas e verifique suas colisões no mapa. */
        for (i = 0; i < contaFantasmas; i++)
        {
            if (fantasma[i].morto)
                continue;

            /* Tente passar por uma abertura. */
            if ((direction = direcaoFantasmasAbertura(&fantasma[i], world)) != 0)
            {
                if (rand() % 3 == 0)
                    fantasma[i].direcaoMovimento = direction;
            }

            colisao = 0;

            switch (fantasma[i].direcaoMovimento)
            {
                case direcaoCima:
                    fantasma[i].velocidade++;
                    if (fantasma[i].velocidade > fantasma_velocidade)
                        fantasma[i].velocidade--;
                    fantasma[i].y -= fantasma[i].velocidade;
                    while (colisaoMapa(&fantasma[i], world) != 0)
                    {
                        fantasma[i].y++;
                        colisao = 1;
                    }
                    /*Volte se colidir com outro fantasma. */
                    for (j = 0; j < contaFantasmas; j++)
                    {
                        if (fantasma[j].morto)
                            continue;
                        if (j != i)
                        {
                            while (colisaoPac(&fantasma[i], &fantasma[j]))
                            {
                                fantasma[i].y++;
                                colisao = 1;
                            }
                        }
                    }
                break;

                case direcaoBaixo:
                    fantasma[i].velocidade++;
                    if (fantasma[i].velocidade > fantasma_velocidade)
                        fantasma[i].velocidade--;
                    fantasma[i].y += fantasma[i].velocidade;
                    while (colisaoMapa(&fantasma[i], world) != 0)
                    {
                        fantasma[i].y--;
                        colisao = 1;
                    }
                    for (j = 0; j < contaFantasmas; j++)
                    {
                        if (fantasma[j].morto)
                            continue;
                        if (j != i)
                        {
                            while (colisaoPac(&fantasma[i], &fantasma[j]))
                            {
                                fantasma[i].y--;
                                colisao = 1;
                            }
                        }
                    }
                break;

                case direcaoEsquerda:
                    fantasma[i].velocidade++;
                    if (fantasma[i].velocidade > fantasma_velocidade)
                        fantasma[i].velocidade--;
                    fantasma[i].x -= fantasma[i].velocidade;
                    while (colisaoMapa(&fantasma[i], world) != 0)
                    {
                        fantasma[i].x++;
                        colisao = 1;
                    }
                    for (j = 0; j < contaFantasmas; j++)
                    {
                        if (fantasma[j].morto)
                            continue;
                        if (j != i)
                        {
                            while (colisaoPac(&fantasma[i], &fantasma[j]))
                            {
                                fantasma[i].x++;
                                colisao = 1;
                            }
                        }
                    }
                break;

                case direcaoDireita:
                    fantasma[i].velocidade++;
                    if (fantasma[i].velocidade > fantasma_velocidade)
                            fantasma[i].velocidade--;
                    fantasma[i].x += fantasma[i].velocidade;
                    while (colisaoMapa(&fantasma[i], world) != 0)
                    {
                        fantasma[i].x--;
                        colisao = 1;
                    }
                    for (j = 0; j < contaFantasmas; j++)
                    {
                        if (fantasma[j].morto)
                        continue;
                        if (j != i)
                        {
                            while (colisaoPac(&fantasma[i], &fantasma[j]))
                            {
                                fantasma[i].x--;
                                colisao = 1;
                            }
                        }
                    }
                break;

                default:
                break;
            }

            /** Mude a direção se bater em alguma coisa. */
            if (colisao)
            {
                if (rand() % 3 == 0)
                    fantasma[i].direcaoMovimento = (rand() % 4) + 1;
                else
                {
                    if (tempo_Energia == 0)
                        direcaoFantasmas(&fantasma[i], &personagemPac);
                }
            }
        }


        /** Verifique as colisões entre o jogador e os fantasmas. **/
        for (i = 0; i < contaFantasmas; i++)
        {
            if (fantasma[i].morto)
                continue;
            if (colisaoPac(&personagemPac, &fantasma[i]))
            {
                if (tempo_Energia > 0)
                {
                    fantasma[i].morto = 1;
                    tocaSons(comendo);
                    *pontos += pontoFantasma;
                }
                else
                {
                    fprintf(stderr, "Morto pelos fantasmas.\n");
                    //jogoStatus = jogoOk;
                    iniciaPac(&personagemPac, aux_x, aux_y);
                    vida--;

                }
                if(vida==0)
                    jogoStatus = jogoOk;
            }
        }



        /** Verificando consumo -- saia se a última pastilha tiver sido consumida */
        if (comendoPastilhas(&personagemPac, pastilha, total_pastilhas,&todasPastilhasConsumidas, &efeitoEnergia) == 1)
        {
            tocaSons(comendo);
            comeCome++;
            if (todasPastilhasConsumidas)
            {
                fprintf(stderr, "Todas a bolas comidas.\n");
                *pontos += 100; /* Pontuação extra para consumir todos.. */
                jogoStatus = jogoOk;
            }
            if (efeitoEnergia)
                tempo_Energia = tempoEnergia;
        }
        if (tempo_Energia > 0)
            tempo_Energia--;


        /** Desenha o mapa */
        if (areaMapa != NULL)
            desenhaMapa(screen, areaMapa);
        //else
            //pacWarDesenhoworld_basic(screen, world);

        desenhaPastilhas(screen, pastilha, total_pastilhas);
        desenhaNum(screen, areaNum, 190, 5, comeCome); /** serve para escrever a pontuação na tela do jogo**/
        desenhaVida(screen, areaNumVida, 480, 5, vida); /** serve para desenhar as vidas**/


        if(flag == 1)
            desenhaPac(&personagemPac, screen, areaPac, tempo_Energia);


            for (i = 0; i < contaFantasmas; i++)
            {
                if (fantasma[i].morto)
                    continue;
                desenhaFantasmas(&fantasma[i], screen, areaFantasma, i, tempo_Energia);
            }

            SDL_Flip(screen);
            SDL_Delay(jogoVelocidade);
    }


    /* Limpar. */
    SDL_FreeSurface(areaNum);
    SDL_FreeSurface(areaNumVida);
    SDL_FreeSurface(areaPac);
    SDL_FreeSurface(areaFantasma);
    if (areaMapa != NULL)
        SDL_FreeSurface(areaMapa);
    Mix_CloseAudio();
    Mix_FreeMusic(music);
    Mix_FreeChunk(comendo);
    Mix_FreeChunk(menu);

    /* Atualizar pontuação. */
    *pontos += pastilhasConsumidas(pastilha, total_pastilhas) *pontoPastilha;
    *pontos *= fantasma_velocidade;
    if (*pontos < 0)
        *pontos = 0;

    if (jogoStatus != jogoOk)
        *pontos = 0; /* Resetando pontuacao */

    return jogoStatus;
}

