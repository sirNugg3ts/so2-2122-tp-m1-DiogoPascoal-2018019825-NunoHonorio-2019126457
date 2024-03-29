﻿// TP_SO2_2122.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>
#include <stdlib.h>
#include "Servidor.h"
#include "Registry.h"
#include "Utils.h"
#include <strsafe.h>

#define BUFSIZE 256
#define PIPE_TIMEOUT 5000

#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 

#define NOME_PIPE TEXT("\\\\.\\pipe\\TPSO2122_SERVER_CLIENTE")


void StartandDest(Tabuleiro* tab) {
    srand(time(NULL));
    tab->startCel = rand() % tab->tam;
    tab->destCel = tab->tam - tab->startCel;
    tab->tab[0][tab->startCel] = _T('╦');
    tab->tab[tab->tam - 1][tab->destCel - 1] = _T('╩');

}

void gamemap(Tabuleiro* tab) {
    //Função para inicializar o tabuleiro

    for (int i = 0; i < tab->tam; i++) {
        for (int j = 0; j < tab->tam; j++) {
            tab->tab[i][j] = _T('x');
        }
    }
}

void processCanos(Tabuleiro* tab, int modo) {
    int random, i;      // ═ ║ ╔ ╗ ╝ ╚
    if (modo == 1) {
        if (tab->queue[0] == _T('═')) {
            for (i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('═');
        }
        else if (tab->queue[0] == _T('║')) {
            for (i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('║');
        }
        else if (tab->queue[0] == _T('╔')) {
            for (i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('╔');
        }
        else if (tab->queue[0] == _T('╗')) {
            for (i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('╗');
        }
        else if (tab->queue[0] == _T('╝')) {
            for (i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('╝');
        }
        else if (tab->queue[0] == _T('╚')) {
            for (i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('╚');
        }
    }
    else if (modo == 2) {
        for (int i = 0; i < 5; i++) {
            tab->queue[i] = tab->queue[i + 1];
            if (i == 4) {
                random = rand() % 6;
                if (random == 0) {
                    tab->queue[5] = _T('═');
                }
                else if (random == 1) {
                    tab->queue[5] = _T('║');
                }
                else if (random == 2) {
                    tab->queue[5] = _T('╔');
                }
                else if (random == 3) {
                    tab->queue[5] = _T('╗');
                }
                else if (random == 4) {
                    tab->queue[5] = _T('╝');
                }
                else if (random == 5) {
                    tab->queue[5] = _T('╚');
                }
            }
        }
    }
}

int MoverAgua(Tabuleiro* tab) {
    if (tab->agua == 1) {
        if (tab->tab[tab->aguay][tab->aguax] == _T('┬')) {
            if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╔') && tab->aguax - 1 >= 0) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┏');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╗') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┓');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('║') && tab->aguay + 1 < tab->tam) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┃');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('═') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('━');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('═') && tab->aguax - 1 >= 0) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('━');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╚') && tab->aguay + 1 < tab->tam) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┗');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╝') && tab->aguay + 1 < tab->tam) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┛');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('━')) {   //   x+1 x-1 ━   x-1 ┗ ┏ x+1 ┛ ┓
            if (tab->tab[tab->aguay][tab->aguax + 1] == _T('═') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('━');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('═') && tab->aguax - 1 >= 0) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('━');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╔') && tab->aguax - 1 >= 0) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┏');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╗') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┓');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax] == _T('╚') && tab->aguax - 1 >= 0) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┗');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╝') && tab->aguay + 1 < tab->tam) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┛');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╩') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┴');
                _tprintf(_T("A agua chegou ao destino final!! \n"));
                return 2;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╩') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┴');
                _tprintf(_T("A agua chegou ao destino final!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('┃')) {// y+1 ┗ ┛  y-1 ┓ ┏  y+1 e y-1 ┃
            if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╚') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┗');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╝') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┛');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('║') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┃');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('║') && tab->aguay - 1 >= 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┃');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('╗') && tab->aguay - 1 >= 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┓');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('╔') && tab->aguay - 1 >= 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┏');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╩') && tab->aguay + 1 < tab->tam) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┴');
                _tprintf(_T("A agua chegou ao destino final!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('┏')) {// y+1 ┗ ┛ ┃ x+1 ┛ ┓ ━   
            if (tab->tab[tab->aguay][tab->aguax + 1] == _T('═') && tab->aguax + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('━');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('║') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┃');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╝') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┛');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╝') && tab->aguax + 1 < tab->tam)
            {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┛');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╚') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┗');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╗') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┓');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╩') && tab->aguay + 1 < tab->tam) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┴');
                _tprintf(_T("A agua chegou ao destino final!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('┓')) {// x-1 ┗ ┏ ━  y+1 ┛┗ ┃ 
            if (tab->tab[tab->aguay][tab->aguax - 1] == _T('═') && tab->aguax - 1 > 0)
            {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('━');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╚') && tab->aguax - 1 >= 0)
            {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┗');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╔') && tab->aguax - 1 >= 0)
            {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┏');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('║') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┃');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╚') && tab->aguay + 1 < tab->tam)
            {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┗');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╩') && tab->aguay + 1 < tab->tam) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┴');
                _tprintf(_T("A agua chegou ao destino final!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('┛')) { // y-1 ┓ ┏ ┃ x-1 ┏ ┗ ━
            if (tab->tab[tab->aguay][tab->aguax - 1] == _T('═') && tab->aguax - 1 >= 0)
            {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('━');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╚') && tab->aguax - 1 >= 0)
            {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┗');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╔') && tab->aguax - 1 >= 0)
            {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┏');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('║') && tab->aguay - 1 >= 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┃');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->agua] == _T('╔') && tab->aguay - 1 >= 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┏');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('╗') && tab->aguay - 1 >= 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┓');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╩') && tab->aguax - 1 >= 0) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┴');
                _tprintf(_T("A agua chegou ao destino final!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('┗')) { // y-1 ┓ ┏ ┃ x+1 ┛ ┓ ━
            if (tab->tab[tab->aguay][tab->aguax + 1] == _T('═') && tab->aguax + 1 < tab->tam)
            {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('━');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╗') && tab->aguax + 1 < tab->tam)
            {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┓');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╝') && tab->aguax + 1 < tab->tam)
            {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┛');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('║') && tab->aguay - 1 > 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┃');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('╗') && tab->aguay - 1 > 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┓');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->agua] == _T('╔') && tab->aguay - 1 >= 0)
            {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┏');
                tab->aguay = tab->aguay - 1;
                return 1;
            }

            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╩') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┴');
                _tprintf(_T("A agua chegou ao destino final!! \n"));
                return 2;
            }
        }
        else {
            _tprintf(_T("A agua transbordou!! \n"));
            return 3;
        }
    }


}

void WINAPI ThreadTerminar(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    WaitForSingleObject(dados->hEventoTerminar, INFINITE);
    exit(1);
   
    
    return;
}

void WINAPI ThreadMoveAgua(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    int resultado = 0;
    dados->memPar->tabMem.agua = 1;
    dados->memPar->win = 1;
    
    
    Sleep(dados->memPar->tabMem.tempAgua * 1000);
    while (!dados->terminarAgua) {

        while (dados->memPar->win == 1 && dados->memPar->tabMem.agua == 1)
        {
            WaitForSingleObject(dados->hMutex, 1000);
            
            if (dados->memPar->tabMem.tab[dados->memPar->tabMem.aguay][dados->memPar->tabMem.aguax] == _T('╦')) {
                dados->memPar->tabMem.tab[dados->memPar->tabMem.aguay][dados->memPar->tabMem.aguax] = _T('┬');
            }
            else {
                dados->memPar->win = MoverAgua(&dados->memPar->tabMem);
            }
            
            SetEvent(dados->hEventoMapa); // quando o mapa é atualizado
            ResetEvent(dados->hEventoMapa);

            ReleaseMutex(dados->hMutex);
            if (dados->jogador->nivel < 7) {
                Sleep((10 - dados->jogador->nivel) * 1000);
            }
            else {
                Sleep(3000);
            }

        }

    }

    return;
}

DWORD WINAPI ThreadServerClient(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    BOOL fSuccess=FALSE;
    DWORD nBytes;
    infoCliente mensagemCliente;
    infoServidor mensagemServidor;
    dados->jogador = &mensagemCliente;
    while (!dados->terminarClientes) {
       // _tprintf(TEXT("[THREAD] TOU DENTRO DA THREAD\n"));
        do {
            fSuccess = ReadFile(
                dados->pipes.pipeInfo[0].hPipes,
                &mensagemCliente,
                sizeof(infoCliente),
                &nBytes,
                NULL);
        } while (!fSuccess);
        //_tprintf(_T("Nome : %s e Modalidade: %d \n"),mensagemCliente.nome, mensagemCliente.modojogo);
        // JOGA TUBO , ANDA QUEUE
        //_tprintf(_T("Linha : %d e Coluna: %d Jogada: %d  Tubo: %c\n"), mensagemCliente.linhaJogada, mensagemCliente.colunaJogada, mensagemCliente.jogada, mensagemCliente.tubo);
        
        if (mensagemCliente.jogada == 1) {
            dados->memPar->tabMem.tab[mensagemCliente.linhaJogada][mensagemCliente.colunaJogada] = mensagemCliente.tubo;
            processCanos(&dados->memPar->tabMem, 1);
            for(int i=0; i<6; i++)
                _tprintf(_T("%c "), dados->memPar->tabMem.queue[i]);
        } else if (mensagemCliente.jogada == 2) {
            dados->memPar->tabMem.tab[mensagemCliente.linhaJogada][mensagemCliente.colunaJogada] = _T('x');
        }
        else if (dados->memPar->win==2) {
            mensagemCliente.score = mensagemCliente.score + mensagemCliente.nivel;
            mensagemCliente.nivel = mensagemCliente.nivel++;
            TerminateThread(dados->Threads[1],0);
            gamemap(&dados->memPar->tabMem); // limpa mapa
            StartandDest(&dados->memPar->tabMem); // calcula inicias
            dados->memPar->tabMem.aguax = dados->memPar->tabMem.startCel; // x inicial agua
            dados->memPar->tabMem.aguay = 0; // y inicial agua 
            dados->Threads[1] = CreateThread(NULL, 0, ThreadMoveAgua, &dados, 0, NULL);
            _tprintf(_T("O jogador passou para o nivel %d, digite comecar para a agua iniciar \n"),dados->jogador->nivel);
            dados->memPar = 1;
        }
        mensagemServidor.infoTab = dados->memPar->tabMem;
        if (!WriteFile(dados->pipes.pipeInfo[0].hPipes, &mensagemServidor, sizeof(infoServidor), &nBytes, NULL)) {
            _tprintf(_T("Erro ao enviar mensagem para o Cliente!! \n"));
        }
        else {
            _tprintf(_T("Enviou mensagem para o Cliente com sucesso!! \n"));
        }
        
    }
    
}



void funcaodebug(Tabuleiro *tab, int i) {
    //Função com mapas prefeitos para efeitos de debugging e testagem

    if (i == 1) {
        tab->tab[0][3] = _T('╦');
        tab->tab[1][3] = _T('║');
        tab->tab[2][3] = _T('╝');
        tab->tab[2][2] = _T('═');
        tab->tab[2][1] = _T('╔');
        tab->tab[3][1] = _T('║');
        tab->tab[4][1] = _T('╩');
    }
    else if(i==2) {
        tab->tab[0][3] = _T('╦');
        tab->tab[0][2] = _T('╔');
        tab->tab[1][2] = _T('╚');
        tab->tab[1][3] = _T('╗');
        tab->tab[2][3] = _T('║');
        tab->tab[3][3] = _T('╝');
        tab->tab[3][2] = _T('╔');
        tab->tab[4][2] = _T('╝');
        tab->tab[4][1] = _T('╩');
    }
    else {
        tab->tab[0][3] = _T('╦');
        tab->tab[0][2] = _T('╔');
        tab->tab[1][2] = _T('╝');
        tab->tab[1][1] = _T('╚');
        tab->tab[0][1] = _T('╗');
        tab->tab[0][0] = _T('╔');
        tab->tab[1][0] = _T('║');
        tab->tab[2][0] = _T('║');
        tab->tab[3][0] = _T('║');
        tab->tab[4][0] = _T('╚');
        tab->tab[4][1] = _T('╝');
        tab->tab[3][1] = _T('║');
        tab->tab[2][1] = _T('╔');
        tab->tab[2][2] = _T('╗');
        tab->tab[3][2] = _T('║');
        tab->tab[4][2] = _T('╩');
    }
}
void queueCanos(Tabuleiro* tab, int modo) {
    int random;
    
    if (modo == 1) {
        tab->queue[0] = _T('═'); 
        tab->queue[1] = _T('║');
        tab->queue[2] = _T('╔');
        tab->queue[3] = _T('╗');
        tab->queue[4] = _T('╝');
        tab->queue[5] = _T('╚');
    }
    else if (modo == 2) {
        for (int i = 0; i < 6; i++) {
            random = rand() % 6;
            if (random == 0) {
                tab->queue[i] = _T('═');
            }
            else if (random == 1) {
                tab->queue[i] = _T('║');
            }
            else if (random == 2) {
                tab->queue[i] = _T('╔');
            }
            else if (random == 3) {
                tab->queue[i] = _T('╗');
            }
            else if (random == 4) {
                tab->queue[i] = _T('╝');
            }
            else if (random == 5) {
                tab->queue[i] = _T('╚');
            }
        }

    }

}

void showmap(Tabuleiro* tab) {
    for (int c = 0; c < tab->tam + 2; c++) {
        if (c == 0) {
            _tprintf(_T("┌"));
        }
        else if (c == tab->tam + 1) {
            _tprintf(_T("┐"));
        }
        else {
            _tprintf(_T("─"));
        }
    }
    _tprintf(_T("\n"));
    for (int i = 0; i < tab->tam; i++) { 
        for (int j = 0; j < tab->tam; j++) {
            if (j == 0) {
                _tprintf(_T("|%c"), tab->tab[i][j]);
            }
            else if (j == tab->tam-1) {
                _tprintf(_T("%c|"), tab->tab[i][j]);
            }
            else {
                _tprintf(_T("%c"), tab->tab[i][j]);
            }
        }
        _tprintf(_T("\n"));
    }
    for (int c = 0; c < tab->tam + 2; c++) {
        if (c == 0) {
            _tprintf(_T("└"));
        }
        else if (c == tab->tam + 1) {
            _tprintf(_T("┘"));
        }
        else {
            _tprintf(_T("─"));
        }
    }
    _tprintf(_T("\n"));
}

void processarComando(CelulaBuffer cel, DadosThreads* dados) {
    if (_tcscmp(cel.comando[0], _T("pausa"))==0) {
        dados->memPar->tabMem.agua = 0;
        Sleep(_wtoi(cel.comando[1]) * 1000);
        dados->memPar->tabMem.agua = 1;
    }
    else if (_tcscmp(cel.comando[0], _T("bloco")) == 0) { 
        // cel.comando[1]- linha cel.comando[2]- coluna
        
        TCHAR y = *cel.comando[1];
        TCHAR x = *cel.comando[2];
        y = _wtoi(cel.comando[1]);
        x = _wtoi(cel.comando[2]);
        dados->memPar->tabMem.tab[y][x] = _T('@');   
        SetEvent(dados->hEventoMapa); // quando o mapa é atualizado
        ResetEvent(dados->hEventoMapa);

    }
    else if (_tcscmp(cel.comando[0], _T("aleatorio")) == 0) {
        if (_tcscmp(cel.comando[1], _T("ativar")) == 0) {
            dados->memPar->tabMem.modoqueue = 2;
        }
        else if (_tcscmp(cel.comando[1], _T("desativar")) == 0) {
            dados->memPar->tabMem.modoqueue = 1;
        }
    }
}


DWORD WINAPI ThreadComandos(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    CelulaBuffer cel;
    int contador = 0;
    int soma = 0;
    while (1) {
        while (dados->memPar->win != 2) {
            //aqui entramos na logica da aula teorica

            //esperamos por uma posicao para lermos
            WaitForSingleObject(dados->hSemLeitura, INFINITE);

            //esperamos que o mutex esteja livre
            WaitForSingleObject(dados->hMutex, INFINITE);


            //vamos copiar da proxima posicao de leitura do buffer circular para a nossa variavel cel
            CopyMemory(&cel, &dados->memPar->buffer[dados->memPar->posL], sizeof(CelulaBuffer));
            dados->memPar->posL++; //incrementamos a posicao de leitura para o proximo consumidor ler na posicao seguinte

            //se apos o incremento a posicao de leitura chegar ao fim, tenho de voltar ao inicio
            if (dados->memPar->posL == TAM_BUFFER)
                dados->memPar->posL = 0;

            _tprintf(_T("%s "),cel.comando[0]);
            if (_tcscmp(cel.comando[0], _T("bloco")) == 0) {
                _tprintf(_T("bloco recebido "));
                for (int i = 0; i < 3; i++) {
                    _tprintf(_T("%s "), cel.comando[i]);
                }
            }
            else if (_tcscmp(cel.comando[0], _T("pausa")) == 0) {
                for (int i = 0; i < 2; i++) {
                    _tprintf(_T("%s "), cel.comando[i]);
                }
            }
            else if (_tcscmp(cel.comando[0], _T("aleatorio")) == 0) {
                for (int i = 0; i < 2; i++) {
                    _tprintf(_T("%s "), cel.comando[i]);
                }
            }
            processarComando(cel, dados);
            //libertamos o mutex
            ReleaseMutex(dados->hMutex);

            //libertamos o semaforo. temos de libertar uma posicao de escrita
            ReleaseSemaphore(dados->hSemEscrita, 1, NULL);
        }
    }

   

    return 0;
}


void pedirComando(CelulaBuffer* buffer, int tam) {
    TCHAR* token = NULL;
    TCHAR comando[100], comandos[3][20], proxComando[100];
    int n;
    while (1) {
        n = 0;
        _fgetts(comando, 20, stdin);
        token = _tcstok_s(comando, _T(" ,\t\n"), &proxComando);

        while (token != NULL) {
            if (token != NULL) {
                _tcscpy_s(comandos[n], 20, token);
                token = _tcstok_s(NULL, _T(" ,\t\n"), &proxComando);
                n++;
            }
        }
        for (int i = 0; i < n; i++) {
            _tprintf(_T("Comandos : %s indice %d\n"), comandos[i], i);
        }
        if (_tcscmp(comandos[0], _T("bloco")) == 0 && n == 3) {
            if (comandos[1] != NULL && comandos[2] != NULL) {
                if (_wtoi(comandos[1]) >= 0 && comandos[2] >= 0 && _wtoi(comandos[1]) < tam && _wtoi(comandos[2]) < tam) {
                    break;
                }
            }
        }
        else if (_tcscmp(comandos[0], _T("pausa")) == 0 && n == 2) {
            if (comandos[1] != NULL && comandos[1] > 0) {
                break;
            }
        }
        else if (_tcscmp(comandos[0], _T("aleatorio")) == 0 && n == 2) {
            if (_tcscmp(comandos[1], _T("ativar")) == 0 && comandos[1] != NULL || _tcscmp(comandos[1], _T("desativar")) == 0 && comandos[1] != NULL) {
                break;
            }
        }
        else if (_tcscmp(comandos[0], _T("comandos")) == 0 && n == 1) {
            _tprintf(_T("Comandos do Monitor: \n"));
            _tprintf(_T("1. bloco linha coluna \n"));
            _tprintf(_T("2. pausa segundos \n"));
            _tprintf(_T("3. aleatorio ativar \n"));
        }
        else {
            _tprintf(_T("Comando inválido! \n"));
        }
    }
    for (int i = 0; i < n; i++) {
        _tcscpy_s(buffer->comando[i], 20, comandos[i]);
    }
}
DWORD WINAPI ThreadComandosServer(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    while (!dados->terminarComandosServidor) {
        _fgetts(dados->comando, 20, stdin);
        if (_tcscmp(dados->comando, _T("comecar\n")) == 0) {
            _tprintf(_T("[Servidor.C/Main] Vou iniciar a Água !!\n"));
            dados->memPar->win = 1;
            ResumeThread(dados->Threads[1]);
        }
        else if (_tcscmp(dados->comando, _T("stopwater\n")) == 0) {
            _tprintf(_T("[Servidor.C/Main] Vou pausar a Água !!\n"));

            dados->memPar->tabMem.agua = 0;
        }
        else if (_tcscmp(dados->comando, _T("startwater\n")) == 0) {
            _tprintf(_T("[Servidor.C/Main] Vou iniciar a Água !!\n"));

            dados->memPar->tabMem.agua = 1;
        }
        else if (_tcscmp(dados->comando, _T("encerra\n")) == 0) {
            SetEvent(dados->hEventoTerminar);
            return 0;
        }
        else if (_tcscmp(dados->comando, _T("listar\n")) == 0) {
            _tprintf(_T("Jogadores:\n%s - %d"), dados->jogador->nome, dados->jogador->score);
        }
            
        
    }

    return 0;
}



int _tmain(int argc, TCHAR* argv[])
{
    DadosThreadComsClientes dadosThreadCliente;
    DWORD tamMapa, waterspeed, velocidadeAgua;
    HANDLE hFileMap; //handle para o file map
    HANDLE ThreadPipes;
    HANDLE hPipe;
    DadosThreads dados;
    BOOL  primeiroProcesso = FALSE;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    //Verificar se o processo servidor é unico

    dados.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_SERVER"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        _tprintf(_T("[Servidor.C/Main] Já Existe uma instância do servidor a correr\n"));
        return 1;

    }

    //Obter valores para o jogo
    if (argc > 4 || (argc < 3 && argc != 1)) {
        _tprintf(_T("[Servidor.C/Main] Servidor.exe <Tamanho Mapa> <Velocidade Água>\n"));
        return 1;
    }
    else if (argc == 1) {
        _tprintf(_T("[Servidor.C/Main] A tentar obter valores através do registo...\n"));
        if (!serverRegistry(&tamMapa, &waterspeed)) {
            _tprintf(_T("[Servidor.C/Main] Não foi possível obter valores a partir do registo, a terminar...\n"));
            return 1;
        }
        else
            _tprintf(_T("[Servidor.C/Main] Valores obtidos com sucesso\n"));

    }
    else if (argc == 3) {
        tamMapa = _wtoi(argv[1]);
        waterspeed = _wtoi(argv[2]);
        if (tamMapa > 20 || tamMapa <= 0 || waterspeed <= 0) {
            _tprintf(_T("[Servidor.C/Main] Tamanho do mapa ou velocidade da água indicado por argumento é inválido\nA tentar obter através do registo...\n"));
            if (!serverRegistry(&tamMapa, &waterspeed)) {
                _tprintf(_T("[Servidor.C/Main] Não foi possível obter valores a partir do registo, a terminar...\n"));
                return 1;
            }
        }
        else {
            _tprintf(_T("[Servidor.C/Main] Tamanho do mapa indicado por argumento : %lu\n"), tamMapa);
            _tprintf(_T("[Servidor.C/Main] Velocidade da água indicado por argumento : %lu\n"), waterspeed);
        }
    }



    //criar semaforo que conta as escritas
    dados.hSemEscrita = CreateSemaphore(NULL, 2, 2, TEXT("SO2_SEMAFORO_ESCRITA"));

    //criar semaforo que conta as leituras
    //0 porque nao ha nada para ser lido e depois podemos ir até um maximo de 2 posicoes para serem lidas
    dados.hSemLeitura = CreateSemaphore(NULL, 0, 2, TEXT("SO2_SEMAFORO_LEITURA"));

    // FALSE nao fazer reset automatico -- TRUE começar assinalado o ev
    dados.hEventoMapa = CreateEvent(NULL, TRUE, FALSE, TEXT("SO2_EVENTO_MAPA"));

    


    if (dados.hSemEscrita == NULL || dados.hSemLeitura == NULL || dados.hMutex == NULL || dados.hEventoMapa == NULL) {
        _tprintf(TEXT("Erro no CreateSemaphore ou no CreateMutex\n"));
        return -1;
    }

    //o openfilemapping vai abrir um filemapping com o nome que passamos no lpName
    //se devolver um HANDLE ja existe e nao fazemos a inicializacao
    //se devolver NULL nao existe e vamos fazer a inicializacao

    hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"));
    if (hFileMap == NULL) {
        //criamos o bloco de memoria partilhada
        hFileMap = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(MemoriaCircular), //tamanho da memoria partilhada
            TEXT("SO2_MEM_PARTILHADA"));//nome do filemapping. nome que vai ser usado para partilha entre processos

        if (hFileMap == NULL) {
            _tprintf(TEXT("Erro no CreateFileMapping\n"));
            return -1;
        }
    }

    //mapeamos o bloco de memoria para o espaco de enderaçamento do nosso processo
    dados.memPar = (MemoriaCircular*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MemoriaCircular));


    if (dados.memPar == NULL) {
        _tprintf(TEXT("Erro no MapViewOfFile\n"));
        return -1;
    }

   
   
    dadosThreadCliente.terminar = 0;

    /* FUNÇÕES DOS CANOS --- FUNCIONAL
    queueCanos(&dados.memPar->tabMem, 2);
    for (int i = 0; i < 6; i++) {
        _tprintf(_T("Cano %d %c \n"), i,dados.memPar->tabMem.queue[i]);
    }
    processCanos(&dados.memPar->tabMem, 2);
    for (int i = 0; i < 6; i++) {
        _tprintf(_T("Cano %d %c \n"), i, dados.memPar->tabMem.queue[i]);
    }
    */
    dados.terminar = 0;
    dados.memPar->tabMem.tam = tamMapa;
    dados.memPar->tabMem.tempAgua = waterspeed;
    queueCanos(&dados.memPar->tabMem, 1);
    gamemap(&dados.memPar->tabMem);
    StartandDest(&dados.memPar->tabMem); //para a 2a meta
    dados.memPar->tabMem.aguax = dados.memPar->tabMem.startCel;
    dados.memPar->tabMem.aguay = 0;
    //funcaodebug(&dados.memPar->tabMem, 1);
    showmap(&dados.memPar->tabMem);
    TCHAR comando[100];

    /// COMUNICAÇÃO SERVER CLIENTE
    int i;
    HANDLE hEventTemp;
    int numClientes = 0;
    TCHAR buf[256];
    BOOL ret;
    DWORD n;
    DWORD offset, nBytes;
    BOOL firstTime = FALSE;

    for (i = 0; i < 2; i++) {
        // aqui passamos a constante FILE_FLAG_OVERLAPPED para o named pipe aceitar comunicações assincronas
        hPipe = CreateNamedPipe(NOME_PIPE, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            2,
            256 * sizeof(TCHAR),
            256 * sizeof(TCHAR),
            1000,
            NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            _tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
            exit(-1);
        }

        // criar evento que vai ser associado à estrutura overlaped
        // os eventos aqui tem de ter sempre reset manual e nao automático porque temos de delegar essas responsabilidades ao sistema operativo
        hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (hEventTemp == NULL) {
            _tprintf(TEXT("[ERRO] ao criar evento\n"));
            return -1;
        }

        HANDLE eventoTerminar = CreateEvent(NULL, TRUE, FALSE, TEXT("SO2_TERMINAR_TUDO"));
        if (eventoTerminar == NULL) {
            _tprintf(TEXT("Não foi possível criar evento para terminar"));
            return -1;
        }
        dados.hEventoTerminar = eventoTerminar;

        dados.pipes.pipeInfo[i].hPipes = hPipe;
        dados.pipes.pipeInfo[i].active = FALSE;
        
        //temos de garantir que a estrutura overlap está limpa
        ZeroMemory(&dados.pipes.pipeInfo[i].overlap, sizeof(dados.pipes.pipeInfo[i].overlap));
        //preenchemos agora o evento
        dados.pipes.pipeInfo[i].overlap.hEvent = hEventTemp;
        dados.pipes.hEvent[i] = hEventTemp;

        _tprintf(TEXT("[ESCRITOR] Esperar ligação de um leitor... (ConnectNamedPipe)\n"));

        // aqui passamos um ponteiro para a estrutura overlap
        ConnectNamedPipe(hPipe, &dados.pipes.pipeInfo[i].overlap);
    }

    dados.terminar = 0;
    dados.terminarMonitor = 0;  dados.terminarComandosServidor = 0;  dados.terminarAgua = 0;  dados.terminarClientes = 0;


    dados.Threads[0] = CreateThread(NULL, 0, ThreadComandos, &dados, CREATE_SUSPENDED, NULL);
    dados.Threads[1] = CreateThread(NULL, 0, ThreadMoveAgua, &dados, CREATE_SUSPENDED, NULL);
    dados.Threads[3] = CreateThread(NULL, 0, ThreadComandosServer, &dados, 0, NULL);
    HANDLE hThreadTerminarTudo = CreateThread(NULL, 0, ThreadTerminar, &dados, 0, NULL);
    if (hThreadTerminarTudo == NULL) {
        _tprintf(TEXT("Não foi possível iniciar thread terminar tudo"));
        return -1;
    }
    //ThreadPipes = CreateThread(NULL, 0, ThreadComsPipes, &dadosThreadCliente, 0, NULL);
    //lancamos a thread
    if (dados.Threads[0] != NULL && dados.Threads[1] != NULL) {
               
                while (numClientes < 2) {
                    //permite estar bloqueado , à espera que 1 evento do array de enventos seja assinalado
                    offset = WaitForMultipleObjects(2, dados.pipes.hEvent, FALSE, INFINITE);
                    i = offset - WAIT_OBJECT_0; // devolve o indice da instancia do named pipe que está ativa, aqui sabemos em que indice o cliente se ligou

                    // se é um indice válido ...
                    if (i >= 0 && i < 2) {
                        if (!firstTime) {
                            dados.Threads[2] = CreateThread(NULL, 0, ThreadServerClient, &dados, 0, NULL);
                            ResumeThread(dados.Threads[0]);
                           
                            firstTime = TRUE;
                        }
                        _tprintf(TEXT("[ESCRITOR] Leitor %d chegou\n"), i);

                        if (GetOverlappedResult(dados.pipes.pipeInfo[i].hPipes, &dados.pipes.pipeInfo[i].overlap, &nBytes, FALSE)) {
                            // se entrarmos aqui significa que a funcao correu tudo bem
                            // fazemos reset do evento porque queremos que o WaitForMultipleObject desbloqueio com base noutro evento e nao neste
                            ResetEvent(dados.pipes.hEvent[i]);

                            //vamos esperar que o mutex esteja livre
                            WaitForSingleObject(dados.hMutex, INFINITE);
                            dados.pipes.pipeInfo[i].active = TRUE; // dizemos que esta instancia do named pipe está ativa
                            ReleaseMutex(dados.hMutex);
                            numClientes++;
                        }
                    }
                }     
        WaitForMultipleObjects(4, dados.Threads, TRUE, INFINITE);
    }
    for (int i = 0; i < 2; i++) {
        _tprintf(TEXT("[ESCRITOR] Desligar o pipe (DisconnectNamedPipe)\n"));
        //desliga todas as instancias de named pipes
        if (!DisconnectNamedPipe(dados.pipes.pipeInfo[i].hPipes)) {
            _tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
            exit(-1);
        }
    }
    UnmapViewOfFile(dados.memPar);
    CloseHandle(hFileMap);
    
    CloseHandle(dados.hSemEscrita);
    CloseHandle(dados.hSemLeitura);
    CloseHandle(dados.hEventoMapa);
    CloseHandle(dados.hMutex);
    return 0;
}

