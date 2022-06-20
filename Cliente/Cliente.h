#ifndef __CLIENTE_H__
#define __CLIENTE_H__

#pragma once

#include "resource.h"

typedef struct {
    HANDLE hPipe;
    HANDLE hWnd;
    int terminar;
}DADOS_THREAD_ATUALIZA_TABULEIRO;

typedef struct {
    int xPos, yPos;
    TCHAR c;
}PosChar;


//Tabuleiro do jogo
typedef struct {
    int tam;
    TCHAR tab[20][20];
    int startCel;
    int destCel;
    int agua;
    int aguay;
    int aguax;
    int tempAgua;
    TCHAR queue[6];
    int modoqueue;
}Tabuleiro;



//Estruturas para coms
//Servidor -> Cliente  AKA Pedido
typedef struct {
    Tabuleiro infoTab;  // tabuleiro para enviar para o cliente
}infoServidor;



//Cliente -> Servidor AKA Resposta
typedef struct {
    TCHAR nome[256];
    int nivel;
    int score;
    int win;

    int modojogo;
    int colunaJogada; // coluna carregada
    int linhaJogada; // linha carregada
    TCHAR tubo;
    BOOL jogada;
}infoCliente;

#endif