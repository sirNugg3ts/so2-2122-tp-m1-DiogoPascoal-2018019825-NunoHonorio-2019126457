// TP_SO2_2122.cpp : This file contains the 'main' function. Program execution begins and ends there.
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
NAMEDPIPE_STRUCT Pipes[2];


BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
    BOOL fConnected, fPendingIO = FALSE;

    // Start an overlapped connection for this pipe instance. 
    fConnected = ConnectNamedPipe(hPipe, lpo);

    // Overlapped ConnectNamedPipe should return zero. 
    if (fConnected)
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    switch (GetLastError())
    {
        // The overlapped connection in progress. 
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;

        // Client is already connected, so signal an event. 

    case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent))
            break;

        // If an error occurs during the connect operation... 
    default:
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }
    }

    return fPendingIO;
}

VOID DisconnectAndReconnect(DWORD i)
{
    // Disconnect the pipe instance. 

    if (!DisconnectNamedPipe(Pipes[i].hPipe))
    {
        printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    // Call a subroutine to connect to the new client. 

    Pipes[i].fPendingIO = ConnectToNewClient(
        Pipes[i].hPipe,
        &Pipes[i].overlap);

    Pipes[i].dwState = Pipes[i].fPendingIO ?
        CONNECTING_STATE : // still connecting 
        READING_STATE;     // ready to read 
}


DWORD WINAPI ThreadComsPipes(LPVOID param){
    DadosThreadComsClientes* dados = (DadosThreadComsClientes*) param;

    HANDLE hEvents[2]; //Handle para eventos overlap
    BOOL fSuccess; //Para verificar se uma operação foi efetuada com sucesso

    

    DWORD i,dwWait, cbRet,dwErr;
        DWORD nBytesTransferidos;

    for(i = 0;i < MAX_CLIENTS;i++){

        hEvents[i] = CreateEvent(
        NULL,
        TRUE,
        TRUE,
        NULL);

        if (hEvents[i] == NULL)
        {
            _tprintf(_T("CreateEvent failed with %d.\n"), GetLastError());
            return 0;
        }

        Pipes[i].overlap.hEvent = hEvents[i];
        Pipes[i].hPipe = CreateNamedPipe(
            NOME_PIPE,            // pipe name 
            PIPE_ACCESS_DUPLEX |     // read/write access 
            FILE_FLAG_OVERLAPPED,    // overlapped mode 
            PIPE_TYPE_MESSAGE |      // message-type pipe 
            PIPE_READMODE_MESSAGE |  // message-read mode 
            PIPE_WAIT,               // blocking mode 
            2,               // number of instances 
            BUFSIZE * sizeof(TCHAR),   // output buffer size 
            BUFSIZE * sizeof(TCHAR),   // input buffer size 
            PIPE_TIMEOUT,            // client time-out 
            NULL);                   // default security attributes

        if (Pipes[i].hPipe == INVALID_HANDLE_VALUE)
        {
            _tprintf(_T("CreateNamedPipe failed with %d.\n"), GetLastError());
            return 0;
        }

         // Call the subroutine to connect to the new client

        Pipes[i].fPendingIO = ConnectToNewClient(
            Pipes[i].hPipe,
            &Pipes[i].overlap);

        Pipes[i].dwState = Pipes[i].fPendingIO ?
            CONNECTING_STATE : // still connecting 
            READING_STATE;     // ready to read 
    }

    while(!dados->terminar){
        

        dwWait = WaitForMultipleObjects(
            MAX_CLIENTS,    // number of event objects 
            hEvents,      // array of event objects 
            FALSE,        // does not wait for all 
            INFINITE);    // waits indefinitely 

        i = dwWait - WAIT_OBJECT_0;  // determines which pipe 
            if (i < 0 || i >(MAX_CLIENTS - 1))
                {
                    _tprintf(_T("Index out of range.\n"));
                    return 0;
                }

        //verificar se há alguma operação por concluir
        if(Pipes[i].fPendingIO){
            fSuccess = GetOverlappedResult(
                Pipes[i].hPipe, //Pipe a ler
                &Pipes[i].overlap, //Estrutura overlapped
                &nBytesTransferidos, //Bytes transferidos
                FALSE); //Não esperar

             switch(Pipes[i].dwState){

            case CONNECTING_STATE: //pending connect
                if(!fSuccess){
                    _tprintf(TEXT("ERRO %d.\n"),GetLastError());
                    return 0;
                }

                Pipes[i].dwState = READING_STATE;
                break;

            // Pending read operation 
            case READING_STATE:
                if (!fSuccess || cbRet == 0)
                {
                    DisconnectAndReconnect(i);
                    continue;
                }
                Pipes[i].cbRead = cbRet;
                Pipes[i].dwState = WRITING_STATE;
                //_tprintf(TEXT("\n READING_STATE -> WRITING_STATE"));
                break;

            // Pending write operation 
            case WRITING_STATE:
                if (!fSuccess || cbRet != Pipes[i].cbToWrite)
                {
                    DisconnectAndReconnect(i);
                    continue;
                }
                //_tprintf(TEXT("\n WRITING_STATE -> READING_STATE"));
                Pipes[i].dwState = READING_STATE;
                break;

            default:
                {
                    _tprintf(_T("Invalid pipe state.\n"));
                    return 0;
                }
            }
        }


        //O estado do pipe decide o que fazer aseguir
        switch (Pipes[i].dwState)
        {
            
            // READING_STATE: 
            // The pipe instance is connected to the client 
            // and is ready to read a request from the client. 

            case READING_STATE:
                //_tprintf(TEXT("\n estou a ler algo no pipe"));
                fSuccess = ReadFile(
                    Pipes[i].hPipe,
                    &Pipes[i].chRequest,
                    BUFSIZE * sizeof(TCHAR),
                    &Pipes[i].cbRead,
                    &Pipes[i].overlap);

          /*  DWORD nBytes;
            DWORD ret2 = GetOverlappedResultEx(
                Pipes[i].hPipe,
                &Pipes[i].overlap,
                &nBytes,
                INFINITE,
                TRUE
            );*/

            // The read operation completed successfully. 
           
            if (fSuccess && Pipes[i].cbRead != 0)
            {
                Pipes[i].fPendingIO = FALSE;
                Pipes[i].dwState = WRITING_STATE;
                
                continue;
            }

            // The read operation is still pending. 

            dwErr = GetLastError();
            if (!fSuccess && (dwErr == ERROR_IO_PENDING))
            {
                Pipes[i].fPendingIO = TRUE;
                continue;
            }

            // An error occurred; disconnect from the client. 

            DisconnectAndReconnect(i);
            break;

            // WRITING_STATE: 
            // The request was successfully read from the client. 
            // Get the reply data and write it to the client. 

            case WRITING_STATE:
                StringCchCopy(Pipes[i].chReply, BUFSIZE, TEXT("TESTE"));
                Pipes[i].cbToWrite = (lstrlen(Pipes[i].chReply) + 1) * sizeof(TCHAR);

                fSuccess = WriteFile(
                    Pipes[i].hPipe,
                    Pipes[i].chReply,
                    Pipes[i].cbToWrite,
                    &cbRet,
                    &Pipes[i].overlap);

                // The write operation completed successfully. 

                if (fSuccess && cbRet == Pipes[i].cbToWrite)
                {

                    Pipes[i].fPendingIO = FALSE;
                    Pipes[i].dwState = READING_STATE;
                    
                    continue;
                }

                // The write operation is still pending. 

                dwErr = GetLastError();
                if (!fSuccess && (dwErr == ERROR_IO_PENDING))
                {
                    Pipes[i].fPendingIO = TRUE;
                    continue;
                }

                // An error occurred; disconnect from the client. 

                DisconnectAndReconnect(i);
                break;

        default:
            {
                _tprintf(_T("Invalid pipe state.\n"));
                return 0;
            }
        }
    }

    return 0;
}

int MoverAgua(Tabuleiro *tab) {
    if (tab->agua == 1) {
        if (tab->tab[tab->aguay][tab->aguax] == _T('┬')) {
            if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╔') && tab->aguax - 1 < tab->tam) {
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
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('═') && tab->aguax - 1 < tab->tam) {
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
            else {
                _tprintf(_T("A agua transbordou!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('┃') && tab->aguay + 1 < tab->tam || tab->tab[tab->aguay][tab->aguax] == _T('┏') && tab->aguay + 1 < tab->tam || tab->tab[tab->aguay][tab->aguax] == _T('┓') && tab->aguay + 1 < tab->tam ) {
            if (tab->tab[tab->aguay + 1][tab->aguax] == _T('║')) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┃');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╝')) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┛');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╚')) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┗');
                tab->aguay = tab->aguay + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╗') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┓');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('═') && tab->aguax + 1 < tab->tam) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('━');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay + 1][tab->aguax] == _T('╩')) {
                tab->tab[tab->aguay + 1][tab->aguax] = _T('┴');
                tab->aguay = tab->aguay + 1;
                return 1;
            }else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╔')) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┏');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay-1][tab->aguax] == _T('╔')) {
                tab->tab[tab->aguay-1][tab->aguax] = _T('┏');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else {
                _tprintf(_T("A agua transbordou!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('┛') || tab->tab[tab->aguay][tab->aguax] == _T('━') ) {
          
           if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╚')) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┗');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('═')) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('━');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax - 1] == _T('╩')) {
                tab->tab[tab->aguay][tab->aguax - 1] = _T('┴');
                tab->aguax = tab->aguax - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('╔')) {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┏');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax-1] == _T('╔')) {
               tab->tab[tab->aguay][tab->aguax-1] = _T('┏');
               tab->aguax = tab->aguax-1;
               return 1;
           }
            else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('╗')) {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┓');
                tab->aguay = tab->aguay - 1;
                return 1;
            }else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╝')) {
                tab->tab[tab->aguay][tab->aguax] = _T('┛');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╝')) {
               tab->tab[tab->aguay][tab->aguax + 1] = _T('┛');
               tab->aguax = tab->aguax + 1;
               return 1;
           }else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('║')) {
               tab->tab[tab->aguay - 1][tab->aguax] = _T('┃');
               tab->aguay = tab->aguay - 1;
               return 1;
           }
            else {
                _tprintf(_T("A agua transbordou!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('━') && tab->aguax + 1 < tab->tam || tab->tab[tab->aguay][tab->aguax] == _T('┗')  && tab->aguax + 1 < tab->tam) {
            if (tab->tab[tab->aguay][tab->aguax + 1] == _T('═')) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('━');
                tab->aguax = tab->aguax + 1;
                return 1; 
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╗')) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┓');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╩')) {
                tab->tab[tab->aguay][tab->aguax + 1] = _T('┴');
                tab->aguax = tab->aguax + 1;
                return 1;
            }
            else if (tab->tab[tab->aguay][tab->aguax + 1] == _T('╝')) {
                tab->tab[tab->aguay][tab->aguax+1] = _T('┛');
                tab->aguax = tab->aguax + 1;
                return 1;
            }else if (tab->tab[tab->aguay - 1][tab->aguax] == _T('╗')) {
                tab->tab[tab->aguay - 1][tab->aguax] = _T('┓');
                tab->aguay = tab->aguay - 1;
                return 1;
            }
            
            else {
                _tprintf(_T("A agua transbordou!! \n"));
                return 2;
            }
        }
        else if (tab->tab[tab->aguay][tab->aguax] == _T('┴')) {
            _tprintf(_T("A agua chegou ao destino final!! \n"));
            return 2;
        }
    }
}

void WINAPI ThreadMoveAgua(LPVOID param) {
    DadosThreads* dados = (DadosThreads*)param;
    int resultado=0;
    dados->memPar->tabMem.agua = 1;
    
    dados->memPar->tabMem.tab[dados->memPar->tabMem.aguay][dados->memPar->tabMem.aguax] = _T('┬');
    Sleep(dados->memPar->tabMem.tempAgua * 1000);
    while (1) {
        while (resultado != 2 && dados->memPar->tabMem.agua==1) {

            WaitForSingleObject(dados->hMutex, INFINITE);
            resultado = MoverAgua(&dados->memPar->tabMem);
            SetEvent(dados->hEventoMapa); // quando o mapa é atualizado
            ResetEvent(dados->hEventoMapa);
           // showmap(&dados->memPar->tabMem);
            ReleaseMutex(dados->hMutex);

            Sleep(1 * 1000); //De momento move uma por segundo, atualizar para velocidade da agua na segunda meta
        }
    }
    dados->memPar->win = resultado;
    return;
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
void processCanos(Tabuleiro* tab, int modo) {
    int random;
    if (modo == 1) {
        if (tab->queue[0] = _T('═')) {
            for (int i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('═');
        }
        else if (tab->queue[0] = _T('║')) {
            for (int i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('║');
        }
        else if (tab->queue[0] = _T('╔')) {
            for (int i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('╔');
        }
        else if (tab->queue[0] = _T('╗')) {
            for (int i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('╗');
        }
        else if (tab->queue[0] = _T('╝')) {
            for (int i = 0; i < 5; i++) {
                tab->queue[i] = tab->queue[i + 1];
            }
            tab->queue[5] = _T('╝');
        }
        else if (tab->queue[0] = _T('╚')) {
            for (int i = 0; i < 5; i++) {
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
void gamemap(Tabuleiro* tab) {
    //Função para inicializar o tabuleiro


    for (int i = 0; i < tab->tam; i++) {
        for (int j = 0; j < tab->tam; j++) {
            tab->tab[i][j] = _T('x');
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

    while (dados->memPar->win!=2) {
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

        
        if (_tcscmp(cel.comando[0], _T("bloco")) == 0) {
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

    return 0;
}
void StartandDest(Tabuleiro* tab) {
    srand(time(NULL));
    tab->startCel = rand() % tab->tam; 
    tab->destCel = tab->tam - tab->startCel; 
    tab->tab[0][tab->startCel] = _T('╦');
    tab->tab[tab->tam-1][tab->destCel - 1] = _T('╩');

}

int _tmain(int argc, TCHAR* argv[])
{
    DadosThreadComsClientes dadosThreadCliente;
    DWORD tamMapa, waterspeed, velocidadeAgua;
    HANDLE hFileMap; //handle para o file map
    HANDLE Threads[2];
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
    dados.memPar->tabMem.aguax = 3; // mudar para dinamico na segunda meta
    dados.memPar->tabMem.aguay = 0;
    gamemap(&dados.memPar->tabMem);
    // StartandDest(&tab); para a 2a meta 
    funcaodebug(&dados.memPar->tabMem, 1);
    showmap(&dados.memPar->tabMem);
    TCHAR comando[100];

   
    

    Threads[0] = CreateThread(NULL, 0, ThreadComandos, &dados, CREATE_SUSPENDED, NULL);
    Threads[1] = CreateThread(NULL, 0, ThreadMoveAgua, &dados, CREATE_SUSPENDED, NULL);
    ThreadPipes = CreateThread(NULL, 0, ThreadComsPipes, &dadosThreadCliente, 0, NULL);
    //lancamos a thread
    if (Threads[0] != NULL && Threads[1] != NULL) {
        _tprintf(_T("--- Digite comecar para iniciar o jogo !! ---\n"));
        while (1) {
            _fgetts(comando, 100, stdin);
            if (_tcscmp(comando, _T("comecar\n")) == 0) {
                ResumeThread(Threads[0]);
                ResumeThread(Threads[1]);
                break;
            }
            else {
                _tprintf(_T("--- Comando inválido, digite comecar ---\n"));
            }
        }
        WaitForMultipleObjects(2, Threads, TRUE, INFINITE);
    }

    UnmapViewOfFile(dados.memPar);
    CloseHandle(hFileMap);
    CloseHandle(dados.hSemEscrita);
    CloseHandle(dados.hSemLeitura);
    CloseHandle(dados.hEventoMapa);
    CloseHandle(dados.hMutex);


    return 0;


}

