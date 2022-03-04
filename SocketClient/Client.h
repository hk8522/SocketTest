#pragma once

#include <string>

int writeFileEvent(const std::wstring& path, const std::wstring& action);
int writeProcessEvent(DWORD pid, const std::wstring& processName, const std::wstring& action);

