#include <Windows.h>

#define TAM_BUFFER 100

#define TAM 200
#define MAX_TAM_MAPA 20
#define SEM_WRITE_NAME TEXT("SEM_WRITE")
#define SEM_READ_NAME TEXT("SEM_READ")
#define MUTEX_NAME TEXT("MUTEX")
#define SHM_NAME TEXT("SMH_PC")

typedef struct PipeInfo {
    HANDLE hPipes;
    OVERLAPPED overlap;
    BOOL active;
}PipeInfo;

typedef struct NAMEDPIPE_STRUCT {
    // handle do pipe
    PipeInfo pipeInfo[2];
    HANDLE hEvent[2];
    HANDLE hMutex[2];
}NAMEDPIPE_STRUCT;

typedef struct {
    int tam;
    TCHAR tab[20][20];
    int startCel;
    int destCel;
    int agua;
    int aguay;
    int aguax;
    int tempAgua;
}Tabuleiro;

//representa o uma celula no buffer
typedef struct {
    int id;
    TCHAR comando[3][100];
}CelulaBuffer;

//representa a nossa memoria partilhada
typedef struct {
    int posE;
    int posL;
  
    int win;
    Tabuleiro tabMem;
    CelulaBuffer buffer[TAM_BUFFER];
}MemoriaCircular;



//estrutura de apoio
typedef struct {
    MemoriaCircular* memPar; //ponteiro para a memoria partilhada
    HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
    HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
    HANDLE hMutex;
    HANDLE hEventoMapa;
    HANDLE Threads[4];
    TCHAR comando[10];
    NAMEDPIPE_STRUCT pipes;
    int terminar; // 1 para sair, 0 em caso contrário
    int id;
}DadosThreads;