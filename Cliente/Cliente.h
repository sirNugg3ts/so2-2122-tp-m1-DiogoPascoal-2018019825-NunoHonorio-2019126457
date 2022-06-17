#ifndef __CLIENTE_H__
#define __CLIENTE_H__

#pragma once

#include "resource.h"

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
    BOOL validada; //Se a jogada é valida
}infoServidor;



//Cliente -> Servidor AKA Resposta
typedef struct {
    TCHAR comando[256];
    int colunaJogada; // coluna carregada
    int linhaJogada; // linha carregada
}infoCliente;

#endif