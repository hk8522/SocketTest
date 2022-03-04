#include "stdafx.h"
#include "ProcessUtil.h"
#include <tlhelp32.h>

uint32_t getParentProcessId(uint32_t pid, std::wstring& exeFile)
{
	uint32_t process_id = 0;

	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	//assume first arg is the PID to get the PPID for, or use own PID
	if (pid == 0)
		pid = GetCurrentProcessId();

	if (Process32FirstW(h, &pe)) {
		do {
			if (pe.th32ProcessID == pid) {
				exeFile = pe.szExeFile;
				process_id = pe.th32ParentProcessID;
				break;
			}
		} while (Process32Next(h, &pe));
	}

	CloseHandle(h);
	return process_id;
}

