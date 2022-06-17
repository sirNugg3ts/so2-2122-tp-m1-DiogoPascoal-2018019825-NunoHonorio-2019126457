#include <Windows.h>

#define TAM_BUFFER 10

#define TAM 200
#define MAX_TAM_MAPA 20
#define SEM_WRITE_NAME TEXT("SEM_WRITE")
#define SEM_READ_NAME TEXT("SEM_READ")
#define MUTEX_NAME TEXT("MUTEX")
#define SHM_NAME TEXT("SMH_PC")
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



//estrutura de apoio
typedef struct {
    MemoriaCircular* memPar; //ponteiro para a memoria partilhada
    HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
    HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
    HANDLE hMutex;
    HANDLE hEventoMapa;
    int terminar; // 1 para sair, 0 em caso contrário
    int id;
}DadosThreads;