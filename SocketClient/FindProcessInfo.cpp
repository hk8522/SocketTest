#include "stdafx.h"
#include "FindProcessInfo.h"
#include <stdio.h>

#pragma comment(lib, "Rstrtmgr.lib")

int FindProcessInfo(PCWSTR pszFile)
{
    DWORD dwSession;
    WCHAR szSessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };
    DWORD dwError = RmStartSession(&dwSession, 0, szSessionKey);
    if (dwError == ERROR_SUCCESS) {
        dwError = RmRegisterResources(dwSession, 1, &pszFile,
            0, NULL, 0, NULL);
        if (dwError == ERROR_SUCCESS) {
            DWORD dwReason;
            UINT i;
            UINT nProcInfoNeeded;
            UINT nProcInfo = 10;
            RM_PROCESS_INFO rgpi[10];
            dwError = RmGetList(dwSession, &nProcInfoNeeded,
                &nProcInfo, rgpi, &dwReason);
            if (dwError == ERROR_SUCCESS) {
                for (i = 0; i < nProcInfo; i++) {
                    wprintf(L"%d.ApplicationType = %d\n", i,
                        rgpi[i].ApplicationType);
                    wprintf(L"%d.strAppName = %ls\n", i,
                        rgpi[i].strAppName);
                    wprintf(L"%d.Process.dwProcessId = %d\n", i,
                        rgpi[i].Process.dwProcessId);
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                        FALSE, rgpi[i].Process.dwProcessId);
                    if (hProcess) {
                        FILETIME ftCreate, ftExit, ftKernel, ftUser;
                        if (GetProcessTimes(hProcess, &ftCreate, &ftExit,
                            &ftKernel, &ftUser) &&
                            CompareFileTime(&rgpi[i].Process.ProcessStartTime,
                                &ftCreate) == 0) {
                            WCHAR sz[MAX_PATH];
                            DWORD cch = MAX_PATH;
                            if (QueryFullProcessImageNameW(hProcess, 0, sz, &cch) &&
                                cch <= MAX_PATH) {
                                wprintf(L"  = %ls\n", sz);
                            }
                        }
                        CloseHandle(hProcess);
                    }
                }
            }
        }
        RmEndSession(dwSession);
    }
    return 0;
}

