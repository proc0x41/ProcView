#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <stdio.h>
#include <string>

#define SEPARATOR(n, c) std::wstring(n, c).c_str()

#define W_PID			6
#define W_NAME			42
#define W_THREADS		8
#define W_MODULES_BASE	16
#define W_MODULES_SIZE	12
#define W_MODULES_PATH	60
#define W_MEMORY		16

#define FMT_TRUNCATE(str, max_size) \
	(wcslen(str) >= (max_size - 3) ? L"%-*p   %-*s   %-*.*s...\n" : L"%-*p   %-*s   %-*.*s\n")

VOID PrintProcessHeaders()
{
	wprintf(
		L"%-*s   %-*s   %-*s   %-*s \n",
		W_PID,		L"PID",
		W_PID,		L"PPID",
		W_NAME,		L"NOME",
		W_THREADS,	L"THREADS"
	);

	wprintf(
		L"%s   %s   %s   %s\n",
		SEPARATOR(W_PID,		'='),
		SEPARATOR(W_PID,		'='),
		SEPARATOR(W_NAME,		'='),
		SEPARATOR(W_THREADS,	'=')
	);
}

VOID PrintModulesHeaders(LPCWSTR name)
{
	wprintf(L"Modules (%s)\n", name);

	wprintf(
		L"%-*s   %-*s   %-*s\n",
		W_MODULES_BASE, L"BASE",
		W_MODULES_SIZE, L"SIZE",
		W_MODULES_PATH, L"NOME"
	);

	wprintf(
		L"%s   %s   %s\n",
		SEPARATOR(W_MODULES_BASE, '='),
		SEPARATOR(W_MODULES_SIZE, '='),
		SEPARATOR(W_MODULES_PATH, '=')
	);
}

VOID PrintAllProcess()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32W Entry = { 0 };
	Entry.dwSize = sizeof(PROCESSENTRY32W);

	PrintProcessHeaders();

	if (!Process32First(hSnapshot, &Entry))
	{
		wprintf(L"%d\n", GetLastError());
		return;
	}

	do
	{
		wprintf(
			L"%-*d   %-*d   %-*s   %-*d\n",
			W_PID, Entry.th32ProcessID,
			W_PID, Entry.th32ParentProcessID,
			W_NAME, Entry.szExeFile,
			W_THREADS, Entry.cntThreads
		);
	} while (Process32NextW(hSnapshot, &Entry));
}

VOID PrintProcessModules(DWORD dwPID)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return;

	MODULEENTRY32W Entry = { 0 };
	Entry.dwSize = sizeof(MODULEENTRY32W);

	if (!Module32FirstW(hSnapshot, &Entry))
	{
		CloseHandle(hSnapshot);
		return;
	}
	
	PrintModulesHeaders(Entry.szModule);

	do
	{
		WCHAR sizeStr[16];
		swprintf(sizeStr, 16, L"%.2fMB", (double)Entry.modBaseSize / (1024 * 1024));
		wprintf(
			FMT_TRUNCATE(Entry.szExePath, W_MODULES_PATH),
			W_MODULES_BASE,			(void*)Entry.modBaseAddr,
			W_MODULES_SIZE,			sizeStr,
			(W_MODULES_PATH - 3),	(W_MODULES_PATH - 3), Entry.szExePath
		);
	} while (Module32NextW(hSnapshot, &Entry));
	CloseHandle(hSnapshot);
	putwchar(L'\n');
}

VOID PrintProcessMemoryInfo(DWORD dwPID)
{
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
		dwPID
		);

	if (hProcess == NULL)
		return;

	PROCESS_MEMORY_COUNTERS_EX pmc = { 0 };
	pmc.cb = sizeof(pmc);

	if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(PROCESS_MEMORY_COUNTERS_EX)))
	{
		WCHAR wsetStr[16], privateStr[16];

		swprintf(wsetStr, W_MEMORY, L"%.2fMB", (double)pmc.WorkingSetSize / (1024.0 * 1024.0));
		swprintf(privateStr, W_MEMORY, L"%.2fMB", (double)pmc.PrivateUsage / (1024.0 * 1024.0));

		wprintf(
			L"Memory:\n"
			L"\tWorking Set: %-*s\n"
			L"\tPrivate Bytes: %-*s\n",
			W_MEMORY,	wsetStr,
			W_MEMORY,	privateStr
		);
	}
	
	CloseHandle(hProcess);
}

VOID SearchProcessInfoByPID(DWORD dwPID)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32W Entry = { 0 };
	Entry.dwSize = sizeof(PROCESSENTRY32W);

	Process32FirstW(hSnapshot, &Entry);

	while (Entry.th32ProcessID != dwPID)
	{
		if (!Process32NextW(hSnapshot, &Entry))
		{
			wprintf(L"Nenhum processo com o PID %d rodando.", dwPID);
			return;
		}	
	} 

	CloseHandle(hSnapshot);

	PrintProcessHeaders();

	wprintf(
		L"%-*d   %-*d   %-*s   %-*d\n",
		W_PID, Entry.th32ProcessID,
		W_PID, Entry.th32ParentProcessID,
		W_NAME, Entry.szExeFile,
		W_THREADS, Entry.cntThreads
	);
	putwchar(L'\n');

	PrintProcessModules(dwPID);
	PrintProcessMemoryInfo(dwPID);
}

DWORD GetProcessPIDByName(LPCWSTR wszName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32W Entry = { 0 };
	Entry.dwSize = sizeof(PROCESSENTRY32W);

	if (!Process32FirstW(hSnapshot, &Entry)) {
		CloseHandle(hSnapshot);
		return 0;
	}

	DWORD pid = 0;
	do
	{
		if (_wcsicmp(Entry.szExeFile, wszName) == 0)
		{
			pid = Entry.th32ProcessID;
			break;
		}
	} while (Process32NextW(hSnapshot, &Entry));

	CloseHandle(hSnapshot);
	return pid;
}

INT wmain(INT argc, WCHAR* argv[])
{
	if (argc == 1)
	{
		PrintAllProcess();
		return EXIT_SUCCESS;
	}

	for (INT i = 1; i < argc; i++)
	{
		if (i + 1 < argc)
		{
			if (wcscmp(argv[1], L"--pid") == 0)
			{
				DWORD pid = wcstoul(argv[++i], NULL, 10);
				SearchProcessInfoByPID(pid);
			}
			else if (wcscmp(argv[1], L"--name") == 0)
			{
				DWORD pid = GetProcessPIDByName(argv[++i]);
				SearchProcessInfoByPID(pid);
			}
			else
			{
				wprintf(L"Uso: procview [--pid <PID>] [--name <nome>]\n");
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}