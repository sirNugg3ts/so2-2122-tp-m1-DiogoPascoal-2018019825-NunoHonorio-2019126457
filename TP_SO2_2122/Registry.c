#include <windows.h>
#include <winreg.h>
#include <tchar.h>
#include <fcntl.h>
#include "Utils.h"
#include "Registry.h"
#include <synchapi.h>

BOOL serverRegistry(DWORD* tamMapa, DWORD* waterspeed) {
	HKEY chave = NULL;

	TCHAR chave_nome[TAM] = TEXT("SOFTWARE\\TP_SO2_2122");
	TCHAR par_nome[TAM] = TEXT("TAM_MAPA");
	DWORD par_valor = MAX_TAM_MAPA;
	TCHAR par_nome_waterspeed[TAM] = TEXT("WATERSPEED");
	DWORD par_valor_waterspeed = 5;
	DWORD resposta;

	//Tentar abrir a chave
	if (RegOpenKeyEx(HKEY_CURRENT_USER, chave_nome,0,KEY_ALL_ACCESS,&chave) != ERROR_SUCCESS) {
		//Erro significa que a chave ainda não foi criada
		_tprintf(_T("[Registry.c/serverRegistry] A chave ainda não foi criada\n"));

		//Tentar criar chave
		if(RegCreateKeyEx(HKEY_CURRENT_USER, chave_nome, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &resposta) != ERROR_SUCCESS){
			//Não foi possível criar chave, mostrar o erro
			DWORD error = GetLastError();
            _tprintf(TEXT("[Registry.c/serverRegistry] Erro ao criar a chave: [%d]\n"), error);
			return FALSE;
		}else{
			if (resposta == REG_CREATED_NEW_KEY) {
				_tprintf(_T("[Registry.c/serverRegistry] Chave criada\n"));

				//Criar registo par chave-valor tamMapa
				if (RegSetValueEx(chave, par_nome, 0, REG_DWORD, &par_valor, sizeof(par_valor)) == ERROR_SUCCESS) {
					_tprintf(_T("[Registry.c/serverRegistry] Par criado: chave %s | valor %i\n"),par_nome,par_valor);
					*tamMapa = par_valor;
				}
				else {
					_tprintf(_T("[Registry.c/serverRegistry] Houve um erro ao criar o par chave-valor\n"));
					DWORD error = GetLastError();
            		_tprintf(TEXT("Error [%d]\n"), error);
					return FALSE;
				}
				//Criar registo par chave-valor waterspeed
				if (RegSetValueEx(chave, par_nome_waterspeed, 0, REG_DWORD, &par_valor_waterspeed, sizeof(par_valor_waterspeed)) == ERROR_SUCCESS) {
					_tprintf(_T("[Registry.c/serverRegistry] Par criado: chave %s | valor %i\n"), par_nome_waterspeed, par_valor_waterspeed);
					*waterspeed = par_valor_waterspeed;
				}
				else {
					_tprintf(_T("[Registry.c/serverRegistry] Houve um erro ao criar o par chave-valor\n"));
					DWORD error = GetLastError();
					_tprintf(TEXT("Error [%d]\n"), error);
					return FALSE;
				}
			}
			else { //impossível criar a chave
				_tprintf(_T("[Registry.c/serverRegistry] Não foi possível criar a chave\n"));
				DWORD error = GetLastError();
            	_tprintf(TEXT("Error [%d]\n"), error);
				return FALSE;
			}
		}
	}
	else {
		_tprintf(TEXT("[Registry.c/serverRegistry]Chave aberta\n"));

		// ler valor da chave
		_tprintf(_T("[Registry.c/serverRegistry] A obter valor do registry...\n"));
		DWORD val = 0;
		DWORD len = sizeof(DWORD);

		if (RegQueryValueEx(chave, par_nome, NULL, NULL, (LPBYTE)(&val), &len) != ERROR_SUCCESS) {
			_tprintf(_T("[Registry.c/serverRegistry] Não foi possível obter o valor a partir da chave\n"));
			//Criar registo par chave-valor tamMapa
			if (RegSetValueEx(chave, par_nome, 0, REG_DWORD, &par_valor, sizeof(par_valor)) == ERROR_SUCCESS) {
				_tprintf(_T("[Registry.c/serverRegistry] Par criado: chave %s | valor %i\n"), par_nome, par_valor);
				*tamMapa = par_valor;
			}
			else {
				_tprintf(_T("[Registry.c/serverRegistry] Houve um erro ao criar o par chave-valor\n"));
				DWORD error = GetLastError();
				_tprintf(TEXT("Error [%d]\n"), error);
				return FALSE;
			}
		
		}
		else {
			_tprintf(_T("[Registry.c/serverRegistry] Valor obtido do registry: %ld\n"), val);

			if (val < 0 || val > 20) {
				_tprintf(_T("[Registry.c/serverRegistry] O Valor obtido do registry é invalido. Será aplicado o valor default de 20\n"));
				val = 20;
			}

			*tamMapa = val;
			if (RegQueryValueEx(chave, par_nome_waterspeed, NULL, NULL, (LPBYTE)(&val), &len) != ERROR_SUCCESS) {
				_tprintf(_T("[Registry.c/serverRegistry] Não foi possível obter o valor a partir da chave\n"));
				//Criar registo par chave-valor tamMapa
				if (RegSetValueEx(chave, par_nome_waterspeed, 0, REG_DWORD, &par_valor_waterspeed, sizeof(par_valor_waterspeed)) == ERROR_SUCCESS) {
					_tprintf(_T("[Registry.c/serverRegistry] Par criado: chave %s | valor %i\n"), par_nome_waterspeed, par_valor_waterspeed);
					*waterspeed = val;
				}
				else {
					_tprintf(_T("[Registry.c/serverRegistry] Houve um erro ao criar o par chave-valor\n"));
					DWORD error = GetLastError();
					_tprintf(TEXT("Error [%d]\n"), error);
					return FALSE;
				}

			}
			RegCloseKey(chave);
			return TRUE;
		}
	}
		
	


	
}