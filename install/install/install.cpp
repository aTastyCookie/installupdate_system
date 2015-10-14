#include "stdafx.h"
#include <iostream>
#include "windows.h"
#include <wininet.h>
#include <fstream>
#include <atlstr.h>
#include <urlmon.h>
#include <Wincrypt.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet")

#define BUFSIZE 1024
#define MD5LEN  16

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

//Скачивание нужного сервиса
int GetService()
{
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
			TEXT("rananyev.ru"),
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
				TEXT("/iuservice/serviceapp/"),
				NULL,
				NULL,
				0,
				INTERNET_FLAG_KEEP_CONNECTION,
				1);

			if (hRequest != NULL) {
				// посылаем запрос
				BOOL bSend = ::HttpSendRequest(hRequest, NULL, 0, NULL, 0);
				std::cout << bSend << std::endl;
				if (bSend) {
					// создаём выходной файл
					std::ofstream fnews("serviceapp.txt", std::ios::out | std::ios::binary);
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

						/*
						* Тут узнаём чё откуда качать
						*/
						HRESULT hr = URLDownloadToFile(NULL, _T("https://rananyev.ru/iuservice/files/mainservice.exe"), _T("iuservice.exe"), 0, NULL); // ссылку из файла выдрать
					}
				}
				else { std::cout << "req" << std::endl; std::cout << GetLastError(); }
				// закрываем запрос
				::InternetCloseHandle(hRequest);
			}
			else std::cout << "ses" << std::endl;
			// закрываем сессию
			::InternetCloseHandle(hConnect);
		}
		else std::cout << "WinInet" << std::endl;
		// закрываем WinInet
		::InternetCloseHandle(hInternet);
	}

	return 0;
}

//Установка и запуск сервиса
BOOL InstallAndStartService(const CString& strServiceName, const CString& strDisplayName, const CString& strBinaryPathName)
{
	BOOL bResult = FALSE;

	SC_HANDLE hServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL != hServiceControlManager)
	{
		SC_HANDLE hService = CreateService(hServiceControlManager,
			(LPCTSTR)strServiceName,
			(LPCTSTR)strDisplayName,
			SC_MANAGER_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
			(LPCTSTR)strBinaryPathName,
			0,
			0,
			0,
			0,
			0);
		if (hService != NULL)
		{
			StartService(hService, NULL, NULL);
			bResult = TRUE;
			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hServiceControlManager);
	}

	return bResult;
}

//md5 файла
std::string Filemd5(const CString& nameOfFile)
{
	std::string resmd5 = "00112233445566778899aabbccddeeff"; // костыли&велосипеды вперде
	DWORD dwStatus = 0;
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = NULL;
	BYTE rgbFile[BUFSIZE];
	DWORD cbRead = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = 0;
	CHAR rgbDigits[] = "0123456789abcdef";
	LPCWSTR filename = (LPCTSTR)nameOfFile;
	// Logic to check usage goes here.

	hFile = CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		dwStatus = GetLastError();
		printf("Error opening file %s\nError: %d\n", filename,
			dwStatus);
		return (char*)dwStatus;
	}

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		return (char*)dwStatus;
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		CryptReleaseContext(hProv, 0);
		return (char*)dwStatus;
	}

	while (bResult = ReadFile(hFile, rgbFile, BUFSIZE,
		&cbRead, NULL))
	{
		if (0 == cbRead)
		{
			break;
		}

		if (!CryptHashData(hHash, rgbFile, cbRead, 0))
		{
			dwStatus = GetLastError();
			printf("CryptHashData failed: %d\n", dwStatus);
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return (char*)dwStatus;
		}
	}

	if (!bResult)
	{
		dwStatus = GetLastError();
		printf("ReadFile failed: %d\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		CloseHandle(hFile);
		return (char*)dwStatus;
	}

	cbHash = MD5LEN;
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		//printf("MD5 hash of file %s is: ", filename);
		for (unsigned int i = 0; i < cbHash; i++)
		{
			//printf("%c%c", rgbDigits[rgbHash[i] >> 4],
				//rgbDigits[rgbHash[i] & 0xf]);
			resmd5[2 * i] = rgbDigits[rgbHash[i] >> 4];
			//std::cout << resmd5[2*i];
			resmd5[2 * i + 1] = rgbDigits[rgbHash[i] & 0xf];
		}
		printf("\n");
	}
	else
	{
		dwStatus = GetLastError();
		printf("CryptGetHashParam failed: %d\n", dwStatus);
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);
	//std::cout << resmd5;
	return resmd5;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int bitVer;
	//Is64BitOperatingSystem() ? bitVer = 64 : bitVer = 32;
	GetService();
	int timeout = 0;
	while (Filemd5("iuservice.exe") != "c1c68651b5f5270361c84562be67af2e") //не тот хэш, надо бы из файла прочесть
	{
		++timeout;
		if (timeout == 100000)
		{
			std::cout << "Your connection is bad";
			break;
		}
	}
	InstallAndStartService("IUservice", "IUservice", "iuservice.exe");
	//system("pause");
	return 0;
}