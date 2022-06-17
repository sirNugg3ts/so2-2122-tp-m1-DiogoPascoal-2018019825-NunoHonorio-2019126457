#include <windows.h>
#include <tchar.h>
#include <math.h>

#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include "Monitor.h"



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
            else if (j == tab->tam - 1) {
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

void pedirComando(CelulaBuffer* buffer, int tam) {
    TCHAR* token=NULL;
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
            if (comandos[1] != NULL && comandos[1] >0) {
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

DWORD WINAPI ThreadComandos(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    CelulaBuffer cel;
    int contador = 0;

    while (dados->memPar->win!=2) {
        cel.id = dados->id;
        
        pedirComando(&cel, dados->memPar->tabMem.tam);

        //esperamos por uma posicao para escrevermos
        WaitForSingleObject(dados->hSemEscrita, INFINITE);

        //esperamos que o mutex esteja livre
        WaitForSingleObject(dados->hMutex, INFINITE);
        
        //vamos copiar a variavel cel para a memoria partilhada (para a posição de escrita)
        CopyMemory(&dados->memPar->buffer[dados->memPar->posE], &cel, sizeof(CelulaBuffer));
        dados->memPar->posE++; //incrementamos a posicao de escrita para o proximo produtor escrever na posicao seguinte
       
        //se apos o incremento a posicao de escrita chegar ao fim, tenho de voltar ao inicio
        if (dados->memPar->posE == 2)
            dados->memPar->posE = 0;

        //libertamos o mutex
        ReleaseMutex(dados->hMutex);

        //libertamos o semaforo. temos de libertar uma posicao de leitura
        ReleaseSemaphore(dados->hSemLeitura, 1, NULL);
    }
    return 0;
}

DWORD WINAPI ThreadMap(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
   
    int contador = 0;

    while (dados->memPar->win!=2) {
        WaitForSingleObject(dados->hEventoMapa, INFINITE);
        //esperamos que o mutex esteja livre
        WaitForSingleObject(dados->hMutex, INFINITE);

        showmap(&dados->memPar->tabMem);
        if (dados->memPar->win == 2) {
            _tprintf(TEXT("O jogo terminou\n"));
        }
        
        //libertamos o mutex
        ReleaseMutex(dados->hMutex);

        //ResetEvent(dados->hEventoMapa);
        //Sleep(4 * 1000);
    }
    return 0;
}


int _tmain(int argc, TCHAR* argv[])
{
    HANDLE hFileMap; //handle para o file map
    HANDLE Threads[2];
  
    DadosThreads dados;
    BOOL  primeiroProcesso = FALSE;
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    
    dados.hSemEscrita = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, TEXT("SO2_SEMAFORO_ESCRITA"));

    //criar semaforo que conta as leituras
    //0 porque nao ha nada para ser lido e depois podemos ir até um maximo de 10 posicoes para serem lidas
    dados.hSemLeitura = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, TEXT("SO2_SEMAFORO_LEITURA"));
    dados.hEventoMapa = OpenEvent(EVENT_ALL_ACCESS, TRUE, TEXT("SO2_EVENTO_MAPA"));
    //criar mutex para os produtores
    dados.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_PRODUTOR"));

    if (dados.hSemEscrita == NULL || dados.hSemLeitura == NULL || dados.hMutex == NULL || dados.hEventoMapa == NULL) {
        _tprintf(TEXT("Não foi possível iniciar o monitor, o servidor está a correr?\n")); 
            return -1;
    }

    

    //o openfilemapping vai abrir um filemapping com o nome que passamos no lpName
    //se devolver um HANDLE ja existe e nao fazemos a inicializacao
    //se devolver NULL nao existe e vamos fazer a inicializacao

    hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"));
    if (hFileMap == NULL) {
        primeiroProcesso = TRUE;
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
    dados.memPar = (MemoriaCircular*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (dados.memPar == NULL) {
        _tprintf(TEXT("Erro no MapViewOfFile\n"));
        return -1;
    }

    if (primeiroProcesso == TRUE) {
        dados.memPar->posE = 0;
        dados.memPar->posL = 0;
    }


    dados.terminar = 0;

  /*  WaitForSingleObject(dados.hMutex, INFINITE);

    ReleaseMutex(dados.hMutex);*/


    //lancamos a thread
    Threads[0] = CreateThread(NULL, 0, ThreadComandos, &dados, 0, NULL);
    Threads[1] = CreateThread(NULL, 0, ThreadMap, &dados, 0, NULL);
    if (Threads[0] != NULL && Threads[1] != NULL) {
        WaitForMultipleObjects(2, Threads, TRUE, INFINITE);
    }
        

    UnmapViewOfFile(dados.memPar);
    //CloseHandles ... mas é feito automaticamente quando o processo termina
    CloseHandle(hFileMap);
    CloseHandle(dados.hSemEscrita);
    CloseHandle(dados.hSemLeitura);
    CloseHandle(dados.hEventoMapa);
    CloseHandle(dados.hMutex);
    
    return 0;

}