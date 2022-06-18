#ifndef __SERVIDOR_H__
#define __SERVIDOR_H__

#pragma once

#include <Windows.h>

#define TAM_BUFFER 10
#define MAX_CLIENTS 2

//Tabuleiro do jogo
typedef struct {
    int tam;
    TCHAR tab[20][20]; /// alterar para dinamico
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
    int modojogo;
    int colunaJogada; // coluna carregada
    int linhaJogada; // linha carregada
}infoCliente;

typedef struct PipeInfo {
    HANDLE hPipes;
    OVERLAPPED overlap;
    BOOL active;
}PipeInfo;

//Estruturas para pipes
typedef struct NAMEDPIPE_STRUCT {
     // handle do pipe
    PipeInfo pipeInfo[2];
    HANDLE hEvent[2];
    HANDLE hMutex[2];
}NAMEDPIPE_STRUCT;



//Estruturas para memória circular


//representa uma celula no buffer
typedef struct {
    int id;
    TCHAR comando[3][20];
}CelulaBuffer;

//representa a nossa memoria partilhada
typedef struct {
    int posE;
    int posL;
   
    int win;
    Tabuleiro tabMem;
    CelulaBuffer buffer[TAM_BUFFER];
}MemoriaCircular;



//Estruturas para threads


typedef struct {
    MemoriaCircular* memPar; //ponteiro para a memoria partilhada
    HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
    HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
    HANDLE hMutex;
    HANDLE hEventoMapa;
    NAMEDPIPE_STRUCT pipes;
    int terminar; // 1 para sair, 0 em caso contrário
    int id;
}DadosThreads;



//Informação para funcionamento da thread pipes 
typedef struct {
   
    int terminar; // 1 para sair, 0 em caso contrário
   
}
DadosThreadComsClientes;

#endif