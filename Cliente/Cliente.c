#include "framework.h"
#include "Cliente.h"

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <Windows.h>
#include <conio.h>
#define MENU_EXIT 1
#define MENU_STOP 2
#define MENU_CHANGE_THEME 3
#define MENU_BLOCK 4
#define HELP_RULES 5
#define BUTTON_MOD1 6
#define BUTTON_MOD2 7
#define BUTTON_START 8
//include "..//Servidor//comunicacao.h"
/* ===================================================== */
/* Programa base (esqueleto) para aplicações Windows     */
/* ===================================================== */
// Cria uma janela de nome "Janela Principal" e pinta fundo de branco
// Modelo para programas Windows:
//  Composto por 2 funções: 
//	WinMain()     = Ponto de entrada dos programas windows
//			1) Define, cria e mostra a janela
//			2) Loop de recepção de mensagens provenientes do Windows
//     TrataEventos()= Processamentos da janela (pode ter outro nome)
//			1) É chamada pelo Windows (callback) 
//			2) Executa código em função da mensagem recebida

infoServidor server;
infoCliente cliente;
HWND hWnd;
HDC hdc;
HDC bmpDC;
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

void WINAPI terminarTudo(LPVOID lpData) {
    DADOS_THREAD_ATUALIZA_TABULEIRO* dados = (DADOS_THREAD_ATUALIZA_TABULEIRO*)lpData;
    HANDLE hEventoTermianr = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("SO2_TERMINAR_TUDO"));
    WaitForSingleObject(hEventoTermianr, INFINITE);
    dados->terminar = 1;
    
    DestroyWindow(hWnd);
    exit(1);
    return;
}

DWORD WINAPI leResposta(LPVOID lpData) {
    DADOS_THREAD_ATUALIZA_TABULEIRO* dados = (DADOS_THREAD_ATUALIZA_TABULEIRO*)lpData;
    TCHAR szStr[256];
    DWORD dwLidos, res;

    OVERLAPPED ov;
    HANDLE hEvento;
    DWORD fSuccess;
    hEvento = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE hEventoMapa = OpenEvent(EVENT_ALL_ACCESS, TRUE, TEXT("SO2_EVENTO_MAPA"));
    if (hEventoMapa == NULL) {
        //nao abriu o evento
        return -1;
    }

    do {
        WaitForSingleObject(hEventoMapa, INFINITE);
        cliente.jogada = 0;
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEvento;
        fSuccess = WriteFile(
            dados->hPipe,                  // pipe handle 
            &cliente,             // message 
            sizeof(infoCliente),              // message length 
            &dwLidos,             // bytes written 
            &ov);                  // not overlapped 

        WaitForSingleObject(ov.hEvent, INFINITE);
        GetOverlappedResult(dados->hPipe, &ov, &dwLidos, FALSE);
        
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEvento;
         fSuccess = ReadFile(
             dados->hPipe,    // pipe handle 
            &server,    // buffer to receive reply 
            sizeof(infoServidor),  // size of buffer 
            &dwLidos,  // number of bytes read 
            &ov);    // not overlapped 

        WaitForSingleObject(ov.hEvent, INFINITE);
        GetOverlappedResult(dados->hPipe, &ov, &dwLidos, FALSE);
        Sleep(1000);
        InvalidateRect(dados->hWnd, NULL, TRUE); // requisita WM_PAINT 
    } while (!dados->terminar);
    return 0;
}

// Nome da classe da janela (para programas de uma só janela, normalmente este nome é 
// igual ao do próprio programa) "szprogName" é usado mais abaixo na definição das 
// propriedades do objecto janela
TCHAR szProgName[] = TEXT("Base");

// ============================================================================
// FUNÇÃO DE INÍCIO DO PROGRAMA: WinMain()
// ============================================================================
// Em Windows, o programa começa sempre a sua execução na função WinMain()que desempenha
// o papel da função main() do C em modo consola WINAPI indica o "tipo da função" (WINAPI
// para todas as declaradas nos headers do Windows e CALLBACK para as funções de
// processamento da janela)
// Parâmetros:
//   hInst: Gerado pelo Windows, é o handle (número) da instância deste programa 
//   hPrevInst: Gerado pelo Windows, é sempre NULL para o NT (era usado no Windows 3.1)
//   lpCmdLine: Gerado pelo Windows, é um ponteiro para uma string terminada por 0
//              destinada a conter parâmetros para o programa 
//   nCmdShow:  Parâmetro que especifica o modo de exibição da janela (usado em  
//        	   ShowWindow()
static HANDLE hPipe;
#define NOME_PIPE TEXT("\\\\.\\pipe\\TPSO2122_SERVER_CLIENTE")
int WINAPI _tWinMain(HINSTANCE hInst, // instancia atual app
    HINSTANCE hPrevInst,//
    LPSTR lpCmdLine, int nCmdShow) {
    		// hWnd é o handler da janela, gerado mais abaixo por CreateWindow()
    MSG lpMsg;		// MSG é uma estrutura definida no Windows para as mensagens
    WNDCLASSEX wcApp;	// WNDCLASSEX é uma estrutura cujos membros servem para 
              // definir as características da classe da janela
    // ============================================================================
    // 1. Definição das características da janela "wcApp" 
    //    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
    // ============================================================================
    wcApp.cbSize = sizeof(WNDCLASSEX);      // Tamanho da estrutura WNDCLASSEX
    wcApp.hInstance = hInst;		         // Instância da janela actualmente exibida 
                                   // ("hInst" é parâmetro de WinMain e vem 
                                         // inicializada daí)
    wcApp.lpszClassName = szProgName;       // Nome da janela (neste caso = nome do programa)
    wcApp.lpfnWndProc = TrataEventos;       // Endereço da função de processamento da janela
                                            // ("TrataEventos" foi declarada no início e
                                            // encontra-se mais abaixo)
    wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela: Fazer o redraw se for
                                            // modificada horizontal ou verticalmente

    wcApp.hIcon = LoadIcon(NULL, IDI_WARNING);   // "hIcon" = handler do ícon normal
                                           // "NULL" = Icon definido no Windows
                                           // "IDI_AP..." Ícone "aplicação"
    wcApp.hIconSm = LoadIcon(NULL, IDI_WARNING); // "hIconSm" = handler do ícon pequeno
                                           // "NULL" = Icon definido no Windows
                                           // "IDI_INF..." Ícon de informação
    wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);	// "hCursor" = handler do cursor (rato) 
                              // "NULL" = Forma definida no Windows
                              // "IDC_ARROW" Aspecto "seta" 
    wcApp.lpszMenuName = NULL;			// Classe do menu que a janela pode ter
                              // (NULL = não tem menu)
    wcApp.cbClsExtra = 0;				// Livre, para uso particular
    wcApp.cbWndExtra = 0;				// Livre, para uso particular
    wcApp.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    BOOL fSuccess=FALSE;
    DWORD dwMode;

    
    while (1)
    {
        hPipe = CreateFile(
            NOME_PIPE,   // pipe name 
            GENERIC_READ |  // read and write access 
            GENERIC_WRITE,
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            FILE_FLAG_OVERLAPPED,              // default attributes 
            NULL);          // no template file 

      // Break if the pipe handle is valid. 
        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
            return -1;
        }

        // All pipe instances are busy, so wait for 20 seconds. 

        if (!WaitNamedPipe(NOME_PIPE, NMPWAIT_USE_DEFAULT_WAIT))
        {
            _tprintf(TEXT("Could not open pipe"));
            return -1;
        }


    }
    //dwMode = PIPE_READMODE_MESSAGE;
    //fSuccess = SetNamedPipeHandleState(
    //    hPipe,    // pipe handle 
    //    &dwMode,  // new pipe mode 
    //    NULL,     // don't set maximum bytes 
    //    NULL);    // don't set maximum time 
    //if (!fSuccess)
    //{
    //    _tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
    //    return -1;
    //}

    // "hbrBackground" = handler para "brush" de pintura do fundo da janela. Devolvido por
    // "GetStockObject".Neste caso o fundo será branco

    // ============================================================================
    // 2. Registar a classe "wcApp" no Windows
    // ============================================================================
    if (!RegisterClassEx(&wcApp))
        return(0);

    // ============================================================================
    // 3. Criar a janela
    // ============================================================================
    hWnd = CreateWindow(
        szProgName,			// Nome da janela (programa) definido acima
        TEXT("Jogo dos Tubos"),// Texto que figura na barra do título
        WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
        600,		// Posição x pixels (default=à direita da última) 
        300,		// Posição y pixels (default=abaixo da última)
        800,		// Largura da janela (em pixels)
        700,		// Altura da janela (em pixels)
        (HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir de
                        // outra) ou HWND_DESKTOP se a janela for a primeira, 
                        // criada a partir do "desktop"
        (HMENU)NULL,			// handle do menu da janela (se tiver menu)
        (HINSTANCE)hInst,		// handle da instância do programa actual ("hInst" é 
                        // passado num dos parâmetros de WinMain()
        0);				// Não há parâmetros adicionais para a janela
      // ============================================================================
      // 4. Mostrar a janela
      // ============================================================================
    ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido por 
                      // "CreateWindow"; "nCmdShow"= modo de exibição (p.e. 
                      // normal/modal); é passado como parâmetro de WinMain()
    UpdateWindow(hWnd);		// Refrescar a janela (Windows envia à janela uma 
                      // mensagem para pintar, mostrar dados, (refrescar)… 
                      // 

    HANDLE hThreadReads;
    DWORD dwTid;
    DADOS_THREAD_ATUALIZA_TABULEIRO dados;
    dados.hPipe = hPipe;
    dados.hWnd = hWnd;
    dados.terminar = 0;
    hThreadReads = CreateThread(NULL, 0, leResposta, &dados , 0, &dwTid);
    if (hThreadReads == NULL) {
        return -1;
    }
    HANDLE threadTermina = CreateThread(NULL, 0, terminarTudo, &dados, 0, NULL);
    if (hThreadReads == NULL) {
        return -1;
    }
    // ============================================================================
    // 5. Loop de Mensagens
    // ============================================================================
    // O Windows envia mensagens às janelas (programas). Estas mensagens ficam numa fila de
    // espera até que GetMessage(...) possa ler "a mensagem seguinte"	
    // Parâmetros de "getMessage":
    // 1)"&lpMsg"=Endereço de uma estrutura do tipo MSG ("MSG lpMsg" ja foi declarada no  
    //   início de WinMain()):
    //			HWND hwnd		handler da janela a que se destina a mensagem
    //			UINT message		Identificador da mensagem
    //			WPARAM wParam		Parâmetro, p.e. código da tecla premida
    //			LPARAM lParam		Parâmetro, p.e. se ALT também estava premida
    //			DWORD time		Hora a que a mensagem foi enviada pelo Windows
    //			POINT pt		Localização do mouse (x, y) 
    // 2)handle da window para a qual se pretendem receber mensagens (=NULL se se pretendem
    //   receber as mensagens para todas as
    // janelas pertencentes à thread actual)
    // 3)Código limite inferior das mensagens que se pretendem receber
    // 4)Código limite superior das mensagens que se pretendem receber

    // NOTA: GetMessage() devolve 0 quando for recebida a mensagem de fecho da janela,
    // 	  terminando então o loop de recepção de mensagens, e o programa 

    while (GetMessage(&lpMsg, NULL, 0, 0)) {
        TranslateMessage(&lpMsg);	// Pré-processamento da mensagem (p.e. obter código 
                       // ASCII da tecla premida)
        DispatchMessage(&lpMsg);	// Enviar a mensagem traduzida de volta ao Windows, que
                       // aguarda até que a possa reenviar à função de 
                       // tratamento da janela, CALLBACK TrataEventos (abaixo)
    }

    // ============================================================================
    // 6. Fim do programa
    // ============================================================================
    return((int)lpMsg.wParam);	// Retorna sempre o parâmetro wParam da estrutura lpMsg
}

// ============================================================================
// FUNÇÃO DE PROCESSAMENTO DA JANELA
// Esta função pode ter um nome qualquer: Apenas é necesário que na inicialização da
// estrutura "wcApp", feita no início de // WinMain(), se identifique essa função. Neste
// caso "wcApp.lpfnWndProc = WndProc"
//
// WndProc recebe as mensagens enviadas pelo Windows (depois de lidas e pré-processadas
// no loop "while" da funçãvo WinMain()
// Parâmetros:
//		hWnd	O handler da janela, obtido no CreateWindow()
//		messg	Ponteiro para a estrutura mensagem (ver estrutura em 5. Loop...
//		wParam	O parâmetro wParam da estrutura messg (a mensagem)
//		lParam	O parâmetro lParam desta mesma estrutura
//
// NOTA:Estes parâmetros estão aqui acessíveis o que simplifica o acesso aos seus valores
//
// A função EndProc é sempre do tipo "switch..." com "cases" que descriminam a mensagem
// recebida e a tratar.
// Estas mensagens são identificadas por constantes (p.e. 
// WM_DESTROY, WM_CHAR, WM_KEYDOWN, WM_PAINT...) definidas em windows.h
// ============================================================================
void addMenu(HWND hWnd);
void addControls(HWND hWnd);
void addGameControls(HWND hWnd);
int oX(int x);
int oY(int y);
HWND hName, hMenu, hButton1, hButton2, hwCano;
RECT rect;
PAINTSTRUCT ps;
LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
    
   
    static HBITMAP hCanoA1, hCanoA2, hCanoA3, hCanoA4, hCanoA5, hCanoA6,hCanoAT,hCanoATI; // Canos agua de jogo
    int i;
    int modojogo;
    static  int numClicks = 1;
    static int totalPos = 0;
    static BITMAP bmp;
    PAINTSTRUCT ps;
    static int aux = 0;
    static TCHAR c = '_';
    static PosChar posicoes[1000];
    TCHAR playerName[100];
    static HBITMAP hCano1, hCano2, hCano3, hCano4, hCano5, hCano6,hCanoT,hCantoTI; // Canos vazios de jogo
    static HBITMAP hBloco; //posicao bloqueada de jogo

    //static ThreadData td;
    static int xBitmap;
    static int yBitmap;
    static HBITMAP hBmp;
    
    static HDC memDC = NULL;
    static TCHAR lpvMessage[256] = TEXT("Default message from client.");
    static BOOL   fSuccess = FALSE, start = FALSE;
    static DWORD  cbRead, cbToWrite, cbWritten, dwMode;
    static TCHAR cbS[256];
    static int next = 0;
    int updateQueue = 0;
    static BOOL theme = FALSE;
 
    DWORD nBytes;
    static BOOL jogada = FALSE;


    OVERLAPPED ov;
    HANDLE hEventOV;
    hEventOV = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (hEventOV == NULL) {
        //reportar erro
        exit(-1);
    }

    switch (messg) {
    case WM_CREATE:
    {
       
        
        //CANOS SEM AGUA
        hCano2 = (HBITMAP)LoadImage(NULL, _T("6.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCano3 = (HBITMAP)LoadImage(NULL, _T("2.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCano4 = (HBITMAP)LoadImage(NULL, _T("1.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCano5 = (HBITMAP)LoadImage(NULL, _T("3.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCano6 = (HBITMAP)LoadImage(NULL, _T("4.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCano1 = (HBITMAP)LoadImage(NULL, _T("5.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCanoT = (HBITMAP)LoadImage(NULL, _T("T.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCantoTI = (HBITMAP)LoadImage(NULL, _T("Tinverso.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hBloco = (HBITMAP)LoadImage(NULL, _T("17.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        //CANOS COM AGUA
        hCanoA2 = (HBITMAP)LoadImage(NULL, _T("12.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCanoA3 = (HBITMAP)LoadImage(NULL, _T("8.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCanoA4 = (HBITMAP)LoadImage(NULL, _T("7.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCanoA5 = (HBITMAP)LoadImage(NULL, _T("9.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCanoA6 = (HBITMAP)LoadImage(NULL, _T("10.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCanoA1 = (HBITMAP)LoadImage(NULL, _T("11.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCanoAT = (HBITMAP)LoadImage(NULL, _T("TAgua.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        hCanoATI = (HBITMAP)LoadImage(NULL, _T("TInversoAgua.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
        cliente.nivel = 0;
        cliente.score = 0;
        hdc = GetDC(hWnd);
        bmpDC = CreateCompatibleDC(hdc);
        addMenu(hWnd);
        addControls(hWnd); 
        break;
    }
    case WM_COMMAND:
    {
        switch (wParam) {
            case MENU_CHANGE_THEME:
                theme = !theme;
            if(theme){
                //CANOS SEM AGUA
            hCano2 = (HBITMAP)LoadImage(NULL, _T("6p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano3 = (HBITMAP)LoadImage(NULL, _T("2p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano4 = (HBITMAP)LoadImage(NULL, _T("1p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano5 = (HBITMAP)LoadImage(NULL, _T("3p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano6 = (HBITMAP)LoadImage(NULL, _T("4p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano1 = (HBITMAP)LoadImage(NULL, _T("5p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoT = (HBITMAP)LoadImage(NULL, _T("Tp.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCantoTI = (HBITMAP)LoadImage(NULL, _T("Tinversop.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hBloco = (HBITMAP)LoadImage(NULL, _T("17p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            //CANOS COM AGUA
            hCanoA2 = (HBITMAP)LoadImage(NULL, _T("12p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA3 = (HBITMAP)LoadImage(NULL, _T("8p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA4 = (HBITMAP)LoadImage(NULL, _T("7p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA5 = (HBITMAP)LoadImage(NULL, _T("9p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA6 = (HBITMAP)LoadImage(NULL, _T("10p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA1 = (HBITMAP)LoadImage(NULL, _T("11p.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoAT = (HBITMAP)LoadImage(NULL, _T("TAguap.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoATI = (HBITMAP)LoadImage(NULL, _T("TInversoAguap.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            }
            else{
            //CANOS SEM AGUA
            hCano2 = (HBITMAP)LoadImage(NULL, _T("6.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano3 = (HBITMAP)LoadImage(NULL, _T("2.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano4 = (HBITMAP)LoadImage(NULL, _T("1.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano5 = (HBITMAP)LoadImage(NULL, _T("3.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano6 = (HBITMAP)LoadImage(NULL, _T("4.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCano1 = (HBITMAP)LoadImage(NULL, _T("5.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoT = (HBITMAP)LoadImage(NULL, _T("T.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCantoTI = (HBITMAP)LoadImage(NULL, _T("Tinverso.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hBloco = (HBITMAP)LoadImage(NULL, _T("17.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            //CANOS COM AGUA
            hCanoA2 = (HBITMAP)LoadImage(NULL, _T("12.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA3 = (HBITMAP)LoadImage(NULL, _T("8.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA4 = (HBITMAP)LoadImage(NULL, _T("7.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA5 = (HBITMAP)LoadImage(NULL, _T("9.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA6 = (HBITMAP)LoadImage(NULL, _T("10.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoA1 = (HBITMAP)LoadImage(NULL, _T("11.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoAT = (HBITMAP)LoadImage(NULL, _T("TAgua.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            hCanoATI = (HBITMAP)LoadImage(NULL, _T("TInversoAgua.bmp"), IMAGE_BITMAP, 18, 18, LR_LOADFROMFILE);
            }
                 
                
            InvalidateRect(hWnd, NULL, TRUE); // requisita WM_PAINT 
            
            break;
            case MENU_EXIT:
                DestroyWindow(hWnd);
                break;

            case BUTTON_MOD1:
                modojogo = 1;
                GetWindowText(hName, &playerName[0], 100);
                DestroyWindow(hName);
                DestroyWindow(hMenu);
                DestroyWindow(hButton1);
                DestroyWindow(hButton2);
                //addGameControls(hWnd);
                aux = 1;
                //TODO: Iniciar modo individual
                cliente.modojogo = modojogo;
                fSuccess = FALSE;
                
                ZeroMemory(&ov, sizeof(ov));
                ov.hEvent = hEventOV;
                fSuccess = WriteFile(
                    hPipe,                  // pipe handle 
                    &cliente,             // message 
                    sizeof(infoCliente),              // message length 
                    &nBytes,             // bytes written 
                    &ov);                  // not overlapped 

                WaitForSingleObject(ov.hEvent, INFINITE);
                GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);

                if (!fSuccess)
                {
                    MessageBox(hWnd, TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError(), _T("Erro"), MB_OK);
                    return -1;
                }
                MessageBox(hWnd, TEXT("Ligação com sucesso \n"), _T("Erro"), MB_OK);
                //
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            case BUTTON_MOD2:
                modojogo = 2;
                GetWindowText(hName, &playerName[0], 100);
                //TODO: Iniciar modo competitivo
               // MessageBox(hWnd, TEXT("COMIA A TUA PRIMA \n"), _T("Erro"), MB_OK);
                DestroyWindow(hName);
                DestroyWindow(hMenu);
                DestroyWindow(hButton1);
                DestroyWindow(hButton2);
                //addGameControls(hWnd);
                
                aux = 1;
                _tcscpy_s(cliente.nome, 256, playerName);
                cliente.modojogo = modojogo;
                fSuccess = FALSE;
                ZeroMemory(&ov, sizeof(ov));
                ov.hEvent = hEventOV;
                fSuccess = WriteFile(
                    hPipe,                  // pipe handle 
                    &cliente,             // message 
                    sizeof(infoCliente),              // message length 
                    &nBytes,             // bytes written 
                    &ov);                  // overlapped 

                WaitForSingleObject(ov.hEvent, INFINITE);
                GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);

                if (!fSuccess)
                {
                    MessageBox(hWnd, TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError(), _T("Erro"), MB_OK);
                    return -1;
                }
                //MessageBox(hWnd, TEXT("COMIA A TUA PRIMA 2 \n"), _T("Erro"), MB_OK);
                //Ler do servidor
                ZeroMemory(&ov, sizeof(ov));
                ov.hEvent = hEventOV;
                fSuccess = ReadFile(
                    hPipe,    // pipe handle 
                    &server,    // buffer to receive reply 
                    sizeof(infoServidor),  // size of buffer 
                    &nBytes,  // number of bytes read 
                    NULL);    // not overlapped 

                WaitForSingleObject(ov.hEvent, INFINITE);
                GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);


                if (!fSuccess)
                {
                    MessageBox(hWnd, TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError(), _T("Erro"), MB_OK);
                    return -1;
                }
                

                //DrawText(hdc, &c, 1, &rect, DT_SINGLELINE | DT_NOCLIP);
                start = TRUE;
                InvalidateRect(hWnd, NULL, TRUE); // requisita WM_PAINT 
                break;
          
        }
        break;
        
    }
    case WM_LBUTTONDOWN: // <-BOTAO ESQUERDO, BOTADO DIREITO -> WM_RBUTTONDOWN
    {
        if (start ) {
            int xParam = GET_X_LPARAM(lParam); // (26*tam)+15 -> max || 15-> min   -------- 15
            int yParam = GET_Y_LPARAM(lParam); // (26*tam)+65 -> max || 65 -> min; -------
            int x = oX(xParam); // 
            int y = oY(yParam); // 
            cliente.linhaJogada = y;
            cliente.colunaJogada = x;
            if (xParam > 15 && xParam < (30 * server.infoTab.tam) + 15 && yParam > 65 && yParam < (30 * server.infoTab.tam) + 65 && server.infoTab.tab[y][x]!=_T('━')
                && server.infoTab.tab[y][x] != _T('┬')
                && server.infoTab.tab[y][x] != _T('┃')
                && server.infoTab.tab[y][x] != _T('┏')
                && server.infoTab.tab[y][x] != _T('┓')
                && server.infoTab.tab[y][x] != _T('┛')
                && server.infoTab.tab[y][x] != _T('┗')
                && server.infoTab.tab[y][x] != _T('┴') 
                && server.infoTab.tab[y][x] != _T('╩')
                ) {
               
                //xBitmap =  x * 26 + 19;
                //yBitmap =  y * 26 + 70;
                

            if (server.infoTab.queue[0] == _T('═')) {  
                cliente.tubo = _T('═');
            }
            else if (server.infoTab.queue[0] == _T('║')) {
                cliente.tubo = _T('║');
            }
            else if (server.infoTab.queue[0] == _T('╔')) {
                cliente.tubo = _T('╔');
            }
            else if (server.infoTab.queue[0] == _T('╗')) {
                cliente.tubo = _T('╗');
            }
            else if (server.infoTab.queue[0] == _T('╝')) {
                cliente.tubo = _T('╝');
            }
            else if (server.infoTab.queue[0] == _T('╚')) {
                cliente.tubo = _T('╚');
            }
            cliente.jogada = 1;
            fSuccess = FALSE;
            ZeroMemory(&ov, sizeof(ov));
            ov.hEvent = hEventOV;
            fSuccess = WriteFile(
                hPipe,                  // pipe handle 
                &cliente,             // message 
                sizeof(infoCliente),              // message length 
                &nBytes,             // bytes written 
                &ov);                  // not overlapped 

            WaitForSingleObject(ov.hEvent, INFINITE);
            GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);

            if (!fSuccess)
            {
                MessageBox(hWnd, TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError(), _T("Erro"), MB_OK);
                return -1;
            }
            //MessageBox(hWnd, TEXT("COMIA A TUA PRIMA 2 \n"), _T("Erro"), MB_OK);
            //Ler do servidor
            ZeroMemory(&ov, sizeof(ov));
            ov.hEvent = hEventOV;
            fSuccess = ReadFile(
                hPipe,    // pipe handle 
                &server,    // buffer to receive reply 
                sizeof(infoServidor),  // size of buffer 
                &nBytes,  // number of bytes read 
                NULL);    // not overlapped 

            WaitForSingleObject(ov.hEvent, INFINITE);
            GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);


            if (!fSuccess)
            {
                MessageBox(hWnd, TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError(), _T("Erro"), MB_OK);
                return -1;
            }        
            cliente.jogada = 0;
            InvalidateRect(hWnd, NULL,  TRUE); // requisita WM_PAINT
           }
        }     
        break;
    }case WM_RBUTTONDOWN:
    {
        if (start) {
            int xParam = GET_X_LPARAM(lParam); // (26*tam)+15 -> max || 15-> min   -------- 15
            int yParam = GET_Y_LPARAM(lParam); // (26*tam)+65 -> max || 65 -> min; -------
            int x = oX(xParam); // 
            int y = oY(yParam); // 
            cliente.linhaJogada = y;
            cliente.colunaJogada = x;
            if (xParam > 15 && xParam < (26 * server.infoTab.tam) + 15 && yParam > 65 && yParam < (26 * server.infoTab.tam) + 65 && server.infoTab.tab[y][x] != _T('━')
                && server.infoTab.tab[y][x] != _T('┬')
                && server.infoTab.tab[y][x] != _T('┃')
                && server.infoTab.tab[y][x] != _T('┏')
                && server.infoTab.tab[y][x] != _T('┓')
                && server.infoTab.tab[y][x] != _T('┛')
                && server.infoTab.tab[y][x] != _T('┗')
                && server.infoTab.tab[y][x] != _T('┴')
                && server.infoTab.tab[y][x] != _T('╩')
                ) {
                cliente.jogada = 2;
                fSuccess = FALSE;
                ZeroMemory(&ov, sizeof(ov));
                ov.hEvent = hEventOV;
                fSuccess = WriteFile(
                    hPipe,                  // pipe handle 
                    &cliente,             // message 
                    sizeof(infoCliente),              // message length 
                    &nBytes,             // bytes written 
                    &ov);                  // not overlapped 

                WaitForSingleObject(ov.hEvent, INFINITE);
                GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);

                if (!fSuccess)
                {
                    MessageBox(hWnd, TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError(), _T("Erro"), MB_OK);
                    return -1;
                }
                //MessageBox(hWnd, TEXT("COMIA A TUA PRIMA 2 \n"), _T("Erro"), MB_OK);
                //Ler do servidor
                ZeroMemory(&ov, sizeof(ov));
                ov.hEvent = hEventOV;
                fSuccess = ReadFile(
                    hPipe,    // pipe handle 
                    &server,    // buffer to receive reply 
                    sizeof(infoServidor),  // size of buffer 
                    &nBytes,  // number of bytes read 
                    NULL);    // not overlapped 

                WaitForSingleObject(ov.hEvent, INFINITE);
                GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);
                if (!fSuccess)
                {
                    MessageBox(hWnd, TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError(), _T("Erro"), MB_OK);
                    return -1;
                }
                cliente.jogada = 0;
                InvalidateRect(hWnd, NULL, TRUE); // requisita WM_PAINT
            }
        }
        break;
        
    }
    case WM_CLOSE:
        if (MessageBox(hWnd, _T("Queres sair da janela?"), _T("Sair"), MB_ICONQUESTION | MB_YESNO | MB_HELP) == IDYES)
        {
            DestroyWindow(hWnd);
        }
        break;
    case WM_PAINT:


        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 96, 255)));
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        if (aux == 1) {

            if (!fSuccess)
            {
                MessageBox(hWnd, TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError(), _T("Erro"), MB_OK);
                return -1;
            }
           
            int i = 0, j;
            int Lx1 = 15, Lx2 = 45;
            int Cy1 = 65, Cy2 = 95;
            for (j = 0; j < server.infoTab.tam; j++) {
                for (i = 0; i < server.infoTab.tam; i++) {
                    Rectangle(hdc, Lx1, Cy1, Lx2, Cy2);
                    Lx1 = Lx2;
                    Lx2 = Lx2 + 30;
                }
                Cy1 = Cy2;
                Cy2 = Cy2 + 30;
                Lx1 = 15, Lx2 = 45;
                
            }
           Rectangle(hdc, 600, 125, 700, 450);
            
          // BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
           
           //pintar queue
           BOOL existeCano = FALSE;
           BOOL queueCano = FALSE;
               for (int i = 0; i < 6; i++) {
                   if (server.infoTab.queue[i] == _T('═')) {
                       GetObject(hCano1, sizeof(bmp), &bmp);
                       SelectObject(bmpDC, hCano1);
                       queueCano = TRUE;
                   }
                   else if (server.infoTab.queue[i] == _T('║')) {
                       GetObject(hCano2, sizeof(bmp), &bmp);
                       SelectObject(bmpDC, hCano2);
                       queueCano = TRUE;
                   }
                   else if (server.infoTab.queue[i] == _T('╔')) {
                       GetObject(hCano3, sizeof(bmp), &bmp);
                       SelectObject(bmpDC, hCano3);
                       queueCano = TRUE;
                   }
                   else if (server.infoTab.queue[i] == _T('╗')) {
                       GetObject(hCano4, sizeof(bmp), &bmp);
                       SelectObject(bmpDC, hCano4);
                       queueCano = TRUE;
                   }
                   else if (server.infoTab.queue[i] == _T('╝')) {
                       GetObject(hCano5, sizeof(bmp), &bmp);
                       SelectObject(bmpDC, hCano5);
                       queueCano = TRUE;
                   }
                   else if (server.infoTab.queue[i] == _T('╚')) {
                       GetObject(hCano6, sizeof(bmp), &bmp);
                       SelectObject(bmpDC, hCano6);
                       queueCano = TRUE;
                   }
                   next = 50 * (i + 1);
                   if(queueCano){
                        BitBlt(hdc, 640, 100 + next, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
                        queueCano = FALSE;
                   }   
               }    

            //pintar tabuleiro
               for (int i = 0; i < server.infoTab.tam; i++) {
                   for (int j = 0; j < server.infoTab.tam; j++) {
                       if(server.infoTab.tab[i][j] == _T('╦')){
                           existeCano = TRUE;
                           GetObject(hCanoT, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoT);
                       }
                       else if (server.infoTab.tab[i][j] == _T('═')) {
                           existeCano = TRUE;
                           GetObject(hCano1, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCano1);
                       }
                       else if (server.infoTab.tab[i][j] == _T('║')) {
                           existeCano = TRUE;
                           GetObject(hCano2, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCano2);
                       }
                       else if (server.infoTab.tab[i][j] == _T('╔')) {
                           existeCano = TRUE;
                           GetObject(hCano3, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCano3);
                       }
                       else if (server.infoTab.tab[i][j] == _T('╗')) {
                           existeCano = TRUE;
                           GetObject(hCano4, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCano4);
                       }
                       else if (server.infoTab.tab[i][j] == _T('╝')) {
                           existeCano = TRUE;
                           GetObject(hCano5, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCano5);
                       }
                       else if (server.infoTab.tab[i][j] == _T('╚')) {
                           existeCano = TRUE;
                           GetObject(hCano6, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCano6);
                       }
                       else if (server.infoTab.tab[i][j] == _T('╩')) {
                           existeCano = TRUE;
                           GetObject(hCantoTI, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCantoTI);
                       }else if(server.infoTab.tab[i][j] == _T('@')){
                           existeCano = TRUE;
                           GetObject(hBloco, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hBloco);
                       }else if(server.infoTab.tab[i][j] == _T('┬')){
                           existeCano = TRUE;
                           GetObject(hCanoAT, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoAT);
                       }
                       else if(server.infoTab.tab[i][j] == _T('━')){
                           existeCano = TRUE;
                           GetObject(hCanoA1, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoA1);
                       }else if(server.infoTab.tab[i][j] == _T('┃')){
                           existeCano = TRUE;
                           GetObject(hCanoA2, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoA2);
                       }else if(server.infoTab.tab[i][j] == _T('┏')){
                           existeCano = TRUE;
                           GetObject(hCanoA3, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoA3);
                       }else if(server.infoTab.tab[i][j] == _T('┓')){
                           existeCano = TRUE;
                           GetObject(hCanoA4, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoA4);
                       }else if(server.infoTab.tab[i][j] == _T('┛')){
                           existeCano = TRUE;
                           GetObject(hCanoA5, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoA5);
                       }else if(server.infoTab.tab[i][j] == _T('┗')){
                           existeCano = TRUE;
                           GetObject(hCanoA6, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoA6);
                       }else if(server.infoTab.tab[i][j] == _T('┴')){
                           existeCano = TRUE;
                           GetObject(hCanoATI, sizeof(bmp), &bmp);
                           SelectObject(bmpDC, hCanoATI);
                       }
                       if (existeCano) {
                           BitBlt(hdc, (30 * j) + 17, (30 * i) + 67, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
                           existeCano = FALSE;
                       }
                       
                   }
               }


        }
        EndPaint(hWnd, &ps);
        break;
    case WM_CHAR:
        c = (TCHAR)wParam;
        break;

    case WM_HELP:
        MessageBox(hWnd, _T("janela de ajuda"), _T("Sair"), MB_OK);
        break;
    case WM_DESTROY:    // Destruir a janela e terminar o programa 
                        // "PostQuitMessage(Exit Status)"
        PostQuitMessage(0);
        break;

    default:
        // Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
        // não é efectuado nenhum processamento, apenas se segue o "default" do Windows
        return(DefWindowProc(hWnd, messg, wParam, lParam));
        break;  // break tecnicamente desnecessário por causa do return
    }
    return(0);
}



void addMenu(HWND hWnd) {
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    HMENU hHelp = CreateMenu();
    AppendMenu(hFileMenu, MF_STRING, MENU_CHANGE_THEME, _T("Mudar Tema"));
    AppendMenu(hFileMenu, MF_STRING, MENU_EXIT, _T("Exit"));
    AppendMenu(hHelp, MF_STRING, HELP_RULES, _T("Rules"));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu , _T("Menu"));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, _T("Help"));
    SetMenu(hWnd, hMenu);
}
int oX(int x) {
    x = x - 15;
    int oX;
    oX = x / 30;
    return oX;
}

int oY(int y) {
    y = y - 65;
    int oY;
    oY = y / 30;
    return oY;
}
void addControls(HWND hWnd) {
    hMenu = CreateWindow(_T("static"),_T("Menu Inicial"),WS_VISIBLE | WS_CHILD | SS_CENTER | ES_AUTOHSCROLL | WS_BORDER, 300, 25, 210, 300, hWnd,NULL, NULL, NULL);
    hName = CreateWindow(_T("edit"), _T("Insira aqui o nome"), WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | SS_CENTER | WS_BORDER, 340, 80, 125,30, hWnd, NULL, NULL, NULL);
    hButton1 = CreateWindow(_T("Button"), _T("Jogar Individualmente"), WS_VISIBLE | WS_CHILD, 305, 125, 200, 30, hWnd, (HMENU)BUTTON_MOD1, NULL, NULL);
    hButton2 = CreateWindow(_T("Button"), _T("Jogar Competitivo"), WS_VISIBLE | WS_CHILD, 305, 185, 200, 30, hWnd, (HMENU)BUTTON_MOD2, NULL, NULL);
    
}

void addGameControls(HWND hWnd) {
    hMenu = CreateWindow(L"static", _T("Próximas peças"), WS_VISIBLE | WS_CHILD | SS_CENTER | ES_AUTOHSCROLL | WS_BORDER, 500, 65, 210, 400, hWnd, NULL, NULL, NULL);   
}