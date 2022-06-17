#ifndef __CLIENTE_H__
#define __CLIENTE_H__

#pragma once

#include "resource.h"
#define NOME_PIPE TEXT("\\\\.\\pipe\\TPSO2122_SERVER_CLIENTE")

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

typedef struct {
    int xPos, yPos;
    TCHAR c;
}PosChar;

typedef struct {
    BOOL terminar;
    infoServidor* infoServidor;
    infoCliente* infoCliente;
    
}DADOS_THREAD_COMS;

//Estruturas para pipes
typedef struct NAMEDPIPE_STRUCT {
    HANDLE hPipe; // handle do pipe

    OVERLAPPED overlap;

    BOOL active; //representa se a instancia do named pipe esta ou nao ativa, se ja tem um cliente ou nao
    BOOL fPendingIO;
    DWORD dwState;
    TCHAR chRequest[256];
    DWORD cbRead;
    TCHAR chReply[256];
    DWORD cbToWrite;
}NAMEDPIPE_STRUCT;


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





#endif