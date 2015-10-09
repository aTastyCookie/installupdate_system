#include "stdafx.h"
#include <iostream>
#include "windows.h"
#include <wininet.h>
#include <fstream>
#pragma comment(lib,"wininet")

//Определение битности. Взял кусок кода отсюда: https://support.microsoft.com/en-gb/kb/2060044
#pragma region Is64BitOperatingSystem()
//   FUNCTION: DoesWin32MethodExist(PCWSTR, PCSTR)
//
//   PURPOSE: The function determins whether a method exists in the export 
//   table of a certain module.
//
//   PARAMETERS:
//   * pszModuleName - the name of the module.
//   * pszMethodName - the name of the method.
//
//   RETURN VALUE: The function returns TRUE if the method specified by 
//   methodName exists in the export table of the module specified by 
//   moduleName.
//
BOOL DoesWin32MethodExist(PCWSTR pszModuleName, PCSTR pszMethodName)
{
	HMODULE hModule = GetModuleHandle(pszModuleName);
	if (hModule == NULL)
	{
		return FALSE;
	}
	return (GetProcAddress(hModule, pszMethodName) != NULL);
}


//
//   FUNCTION: Is64BitOperatingSystem()
//
//   PURPOSE: The function determines whether the current operating system is 
//   a 64-bit operating system.
//
//   RETURN VALUE: The function returns TRUE if the operating system is 
//   64-bit; otherwise, it returns FALSE.
//
BOOL Is64BitOperatingSystem()
{
#if defined(_WIN64)
	return TRUE;   // 64-bit programs run only on Win64
#elif defined(_WIN32)
	// 32-bit programs run on both 32-bit and 64-bit Windows
	BOOL f64bitOS = FALSE;
	return ((DoesWin32MethodExist(L"kernel32.dll", "IsWow64Process") &&
		IsWow64Process(GetCurrentProcess(), &f64bitOS)) && f64bitOS);
#else
	return FALSE;  // 64-bit Windows does not support Win16
#endif
}
#pragma endregion

//Скачивание и запуск нужного сервиса (пока нет)
int GetService()
{
	bool ok = false;
	// инициализируем WinInet
	HINTERNET hInternet =
		::InternetOpen(
		TEXT("WinInet Test"),
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL, NULL,
		0);
	if (hInternet != NULL) {
		// открываем HTTP сессию
		HINTERNET hConnect =
			::InternetConnect(
			hInternet,
			TEXT("95.213.191.87"),
			INTERNET_DEFAULT_HTTP_PORT,
			NULL, NULL,
			INTERNET_SERVICE_HTTP,
			0,
			1u);

		if (hConnect != NULL) {
			// открываем запрос
			HINTERNET hRequest =
				::HttpOpenRequest(
				hConnect,
				TEXT("GET"),
				TEXT("serviceapp.csv"),
				NULL,
				NULL,
				0,
				INTERNET_FLAG_KEEP_CONNECTION,
				1);

			if (hRequest != NULL) {
				// посылаем запрос
				BOOL bSend = ::HttpSendRequest(hRequest, NULL, 0, NULL, 0);
				if (bSend) {
					// создаём выходной файл
					std::ofstream fnews("applist.csv", std::ios::out | std::ios::binary);
					if (fnews.is_open()) for (;;) {
						// читаем данные
						char  szData[1024];
						DWORD dwBytesRead;
						BOOL bRead =
							::InternetReadFile(
							hRequest,
							szData, sizeof(szData)-1,
							&dwBytesRead);

						// выход из цикла при ошибке или завершении
						if (bRead == FALSE || dwBytesRead == 0)
							break;

						// сохраняем результат
						szData[dwBytesRead] = 0;
						fnews << szData;

						bool ok = true;
					}
				}
				else std::cout << 4;
				// закрываем запрос
				::InternetCloseHandle(hRequest);
			}
			else std::cout << 3;
			// закрываем сессию
			::InternetCloseHandle(hConnect);
		}
		else std::cout << 2;
		// закрываем WinInet
		::InternetCloseHandle(hInternet);
	}
	else std::cout << 1;

	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	int bitVer;
	Is64BitOperatingSystem() ? bitVer = 64 : bitVer = 32;
	GetService();
	return 0;
}