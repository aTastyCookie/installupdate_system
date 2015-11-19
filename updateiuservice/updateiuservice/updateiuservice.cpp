#include "stdafx.h"
#include <iostream>
#include "windows.h"
#include <wininet.h>
#include <fstream>
#include <atlstr.h>
#include <urlmon.h>
#include <tlhelp32.h>
#include <Wincrypt.h>
#include <WbemCli.h>
#include <VersionHelpers.h>
#include <string>

#pragma comment(lib, "wbemuuid.lib")  
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet")

#define BUFSIZE 1024
#define MD5LEN  16

//md5 файла
std::string Filemd5(std::wstring nameOfFile)
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
	LPCWSTR filename = nameOfFile.c_str();
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

int updateService()
{
	std::wstring result = L"";
	std::wstring hash = L"";
	TCHAR szTempPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH, szTempPathBuffer);
	std::wstring filePath = szTempPathBuffer;
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
				if (bSend) {
					// создаём выходной файл
					TCHAR szTempPathBuffer[MAX_PATH];
					GetTempPath(MAX_PATH, szTempPathBuffer);
					std::wstring filePath = szTempPathBuffer;
					filePath += L"\serviceapp.txt";
					std::ofstream fnews(filePath, std::ios::out | std::ios::binary);
					BOOL urlread = FALSE;
					std::string urlReading = "";
					std::string hashReading = "";
					if (fnews.is_open()) for (;;) {
						// читаем данные
						char  szData[1024];
						DWORD dwBytesRead;
						BOOL bRead = ::InternetReadFile(hRequest, szData, sizeof(szData)-1, &dwBytesRead);
						// выход из цикла при ошибке или завершении
						if (bRead == FALSE || dwBytesRead == 0)
							break;
						if (urlread == FALSE)
						{
							int i = 0;
							while (szData[i] != ';')
							{
								hashReading.append(1, szData[i]);
								++i;
							}
							++i;
							while (szData[i] != '\n')
							{
								urlReading.append(1, szData[i]);
								++i;
							}
							urlread = TRUE;
						}
						// сохраняем результат
						szData[dwBytesRead] = 0;
						fnews << szData;
					}
					std::wstring resUrl(urlReading.begin(), urlReading.end());
					result = resUrl;
					std::wstring resHash(hashReading.begin(), hashReading.end());
					hash = resHash;
				}
				//else { std::cout << "req" << std::endl; std::cout << GetLastError(); }
				// закрываем запрос
				::InternetCloseHandle(hRequest);
			}
			//else std::cout << "ses" << std::endl;
			// закрываем сессию
			::InternetCloseHandle(hConnect);
		}
		//else std::cout << "WinInet" << std::endl;
		// закрываем WinInet
		::InternetCloseHandle(hInternet);
	}
	filePath = szTempPathBuffer;
	filePath += L"\iuservice.exe";
	std::string fileHash = Filemd5(filePath);
	std::wstring wfileHash(fileHash.begin(), fileHash.end());
	if (wfileHash != hash)
		HRESULT hr1 = URLDownloadToFile(NULL, result.c_str(), filePath.c_str(), 0, NULL);
	return 0;
}

std::wstring* readAppsUrls(const std::string version)
{
	std::wstring* result = new std::wstring[8]; //8 ссылок
	std::wstring wversion(version.begin(), version.end());
	wversion = L"/iuservice/applist/" + wversion + L".txt";
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
				wversion.c_str(),
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
					TCHAR szTempPathBuffer[MAX_PATH];
					GetTempPath(MAX_PATH, szTempPathBuffer);
					std::wstring filePath = szTempPathBuffer;
					filePath += L"\listapp.txt";
					std::ofstream applist(filePath, std::ios::out | std::ios::binary);
					std::string urlReading = "";
					if (applist.is_open()) for (;;) {
						// читаем данные
						char  szData[1024];
						DWORD dwBytesRead;
						BOOL bRead = ::InternetReadFile(hRequest, szData, sizeof(szData)-1, &dwBytesRead);
						// выход из цикла при ошибке или завершении
						if (bRead == FALSE || dwBytesRead == 0)
							break;
						int lines = 0;
						int i = 0;
						for (;;)
						{
							urlReading.clear();
							for (int dotCommaCount = 0; dotCommaCount < 1; ++dotCommaCount)
							{

								while (szData[i] != ';')
								{
									++i;
								}
								++i;
							}
							while (szData[i] != ';')
							{
								urlReading.append(1, szData[i]);
								++i;
							}
							urlReading.append(1, szData[i]);
							++i;
							while (szData[i] != ';')
							{
								urlReading.append(1, szData[i]);
								++i;
							}
							std::wstring resUrl(urlReading.begin(), urlReading.end());
							resUrl += L"\0";
							*(result + lines) = resUrl;
							++lines;
							while (szData[i] != '\n')
							{
								++i;
							}
							++i;
							if (szData[i] == '\n' || i > 1023) break;
						}

						// сохраняем результат
						szData[dwBytesRead] = 0;
						applist << szData;
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

	return result;
}

std::wstring* readAppsNames(std::wstring tempFolder)
{
	std::wstring* result = new std::wstring[8]; //8 имён
	std::wstring singleName = L"";
	std::string str;
	std::ifstream fin(tempFolder + L"\listapp.txt");
	int i = 0;
	while (!fin.eof())
	{

		std::getline(fin, str);
		if (str.length() == 0) break;
		int j = 0;
		while (str[j] != ';')
		{
			++j;
		}
		++j;
		while (str[j] != ';')
		{
			singleName += str[j];
			++j;
		}
		result[i] = singleName;
		singleName = L"";
		++i;
	}
	return result;
}

std::string* readAppsHashes(std::wstring tempFolder)
{
	std::string* result = new std::string[8]; //8 хэшей
	std::string singleHash = "";
	std::string str;
	std::ifstream fin(tempFolder + L"\listapp.txt");
	int i = 0;
	while (!fin.eof())
	{

		std::getline(fin, str);
		if (str.length() == 0) break;
		int j = 0;
		while (str[j] != ';')
		{
			++j;
		}
		++j;
		while (str[j] != ';')
		{
			++j;
		}
		++j;
		while (str[j] != ';')
		{
			++j;
		}
		++j;
		int count = 0;
		while (count != 32)
		{
			singleHash += str[j];
			++count;
			++j;
		}
		result[i] = singleHash;
		singleHash = "";
		++i;
	}
	return result;
}

int renewApplist()
{
	TCHAR szTempPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH, szTempPathBuffer);
	std::wstring filePath = szTempPathBuffer;
	filePath += L"\iuurl.txt";
	std::ifstream iuurl(filePath);
	std::string version = "";
	while (!iuurl.eof())
		iuurl >> version;
	std::wstring* allUrls = readAppsUrls(version);
	std::wstring* allNames = readAppsNames(szTempPathBuffer);
	std::string* allHashes = readAppsHashes(szTempPathBuffer);
	for (int i=0; i < 8; ++i)
	{
		filePath = szTempPathBuffer;
		filePath += allNames[i];
		if (filePath != szTempPathBuffer)
			if (Filemd5(filePath) != allHashes[i])
				HRESULT hr1 = URLDownloadToFile(NULL, allUrls[i].c_str(), filePath.c_str(), 0, NULL);
	}
	return 0;
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char*, int nShowCmd)
//int _tmain(int argc, _TCHAR* argv[])
{
	renewApplist();
	updateService();
	system("pause");
	return 0;
}

