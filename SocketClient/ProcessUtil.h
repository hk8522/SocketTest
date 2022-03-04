#pragma once

#include <stdint.h>
#include <string>

uint32_t getParentProcessId(uint32_t pid, std::wstring& exeFile);
