#include "stdafx.h"
#include "Client.h"
#include "network_init.h"
#include "ProcessUtil.h"
#include <chrono>
#include <psapi.h>

#define SERVER_ADDR "127.0.0.1"
#define SNAPSHOT_SIZE 4096

#define SEND_GAP															\
	{																		\
		uint32_t gap = DELIMITER;   										\
		if (send(sock, (char*)&gap, DELIMITER_SIZE, 0) < DELIMITER_SIZE)	\
			return EXIT_FAILURE;   											\
	}

static __int64 FileSize(const wchar_t* name)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
		return -1; // error condition, could call GetLastError to find out more
	LARGE_INTEGER size;
	size.HighPart = fad.nFileSizeHigh;
	size.LowPart = fad.nFileSizeLow;
	return size.QuadPart;
}

static SOCKET connectToServer()
{
	SOCKET sock;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	// Creating socket file descriptor
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		perror("socket failed");
		network_cleanup();
		return INVALID_SOCKET;
	}

	if (!inet_pton(AF_INET, SERVER_ADDR, &address.sin_addr))
		return EXIT_FAILURE;
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT_NO);

	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		perror("connect failed");
		network_cleanup();
		return INVALID_SOCKET;
	}

	return sock;
}

static int send_u32(SOCKET sock, uint32_t value)
{
	value = htonl(value);
	if (send(sock, (char*)&value, sizeof(uint32_t), 0) < sizeof(uint32_t))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

static int send_u8(SOCKET sock, uint8_t value)
{
	if (send(sock, (char*)&value, sizeof(uint8_t), 0) < sizeof(uint8_t))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

static int send_string(SOCKET sock, const std::wstring& text)
{
	size_t sz = sizeof(wchar_t) * (text.length() + 1);
	if (send(sock, (const char*)text.c_str(), (int)sz, 0) < (int)sz)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

static int sendData(SOCKET sock, PROTOCOL_MESSAGE &msg)
{
	//uint32_t length;
	if (send_u32(sock, msg.length) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//char szMagic[sizeof(MAGIC)];
	if (send(sock, MAGIC, sizeof(MAGIC) - 1, 0) < sizeof(MAGIC) - 1)
		return EXIT_FAILURE;
	SEND_GAP;

	//time;
	if (send_u32(sock, msg.time.year) != EXIT_SUCCESS
		|| send_u8(sock, msg.time.month) != EXIT_SUCCESS
		|| send_u8(sock, msg.time.date) != EXIT_SUCCESS
		|| send_u8(sock, msg.time.hour) != EXIT_SUCCESS
		|| send_u8(sock, msg.time.minite) != EXIT_SUCCESS
		|| send_u8(sock, msg.time.second) != EXIT_SUCCESS
		|| send_u8(sock, msg.time.miliseconds) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//uint32_t process_id;
	if (send_u32(sock, msg.process_id) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//std::string processName;
	if (send_string(sock, msg.processName) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//uint32_t parent_process_id;
	if (send_u32(sock, msg.parent_process_id) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//std::string parentProcessName;
	if (send_string(sock, msg.parentProcessName) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//std::string activity; // (read/write/delete/etc.)
	if (send_string(sock, msg.activity) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//std::string filePath;
	if (send_string(sock, msg.filePath) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//uint32_t fileSize;
	if (send_u32(sock, msg.fileSize) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//uint32_t readDataLength;
	if (send_u32(sock, msg.readDataLength) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//uint32_t writeDataLength;
	if (send_u32(sock, msg.writeDataLength) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	SEND_GAP;

	//uint8_t* snapshot;
	if (msg.snapshot != nullptr)
	{
		if (send(sock, (char*)msg.snapshot, msg.snapshotLength, 0) < (int)msg.snapshotLength)
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int writeFileEvent(const std::wstring& path, const std::wstring& action)
{
	int err;

	SOCKET sock = connectToServer();

	WCHAR szFileName[MAX_PATH];
	GetModuleFileNameW(NULL, szFileName, MAX_PATH);

	std::tm parts{};
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	localtime_s(&parts, &now_c);

	PROTOCOL_MESSAGE msg;

	//char szMagic[sizeof(MAGIC)];
	strcpy_s(msg.szMagic, sizeof(msg.szMagic), MAGIC);
	//time;
	msg.time.year = parts.tm_year;
	msg.time.month = parts.tm_mon;
	msg.time.date = parts.tm_mday;
	msg.time.hour = parts.tm_hour;
	msg.time.minite = parts.tm_min;
	msg.time.second = parts.tm_sec;
	msg.time.miliseconds = 0;
	//uint32_t process_id;
	msg.process_id = 0;// GetCurrentProcessId();
	//std::wstring processName;
	msg.processName = L"";// szFileName;
	//uint32_t parent_process_id;
	//std::wstring parentProcessName;
	msg.parent_process_id = 0;// getParentProcessId(msg.process_id, msg.parentProcessName);
	msg.parentProcessName = L"";
	//std::wstring activity; // (read/write/delete/etc.)
	msg.activity = action;
	//std::wstring filePath;
	msg.filePath = path;
	//uint32_t fileSize;
	msg.fileSize = (uint32_t)FileSize(path.c_str());
	//uint32_t readDataLength;
	msg.readDataLength = 0;
	//uint32_t writeDataLength;
	msg.writeDataLength = 0;

	//uint8_t* snapshot;
	msg.snapshot = nullptr;// (uint8_t*)&msg;
	//uint32_t snapshotLength;
	msg.snapshotLength = 0;// sizeof(msg);

	//uint32_t length;
	msg.length = (uint32_t)(
		sizeof(msg.length) + DELIMITER_SIZE
		+ sizeof(MAGIC) - 1 + DELIMITER_SIZE
		+ sizeof(uint32_t) + 6 + DELIMITER_SIZE										// time
		+ sizeof(uint32_t) + DELIMITER_SIZE											// process_id
		+ sizeof(wchar_t) * (1 + msg.processName.length()) + DELIMITER_SIZE			// processName
		+ sizeof(uint32_t) + DELIMITER_SIZE											// parent_process_id
		+ sizeof(wchar_t) * (1 + msg.parentProcessName.length()) + DELIMITER_SIZE	// parentProcessName
		+ sizeof(wchar_t) * (1 + msg.activity.length()) + DELIMITER_SIZE			// fileSystemActivity // (read/write/delete/etc.)
		+ sizeof(wchar_t) * (1 + msg.filePath.length()) + DELIMITER_SIZE			// filePath
		+ sizeof(uint32_t) + DELIMITER_SIZE											// fileSize
		+ sizeof(uint32_t) + DELIMITER_SIZE											// readDataLength
		+ sizeof(uint32_t) + DELIMITER_SIZE											// writeDataLength
		+ msg.snapshotLength// snapshot
		);

	err = sendData(sock, msg);

	closesocket(sock);

	return err;
}

int writeProcessEvent(DWORD pid, const std::wstring& processName,
	const std::wstring& action)
{
	int err;

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, TRUE, pid);
	uint32_t ppid = 0;
	std::wstring parentProcessName;
	ppid = getParentProcessId(pid, parentProcessName);

	SOCKET sock = connectToServer();

	std::tm parts{};
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	localtime_s(&parts, &now_c);

	PROTOCOL_MESSAGE msg = { 0 };

	//char szMagic[sizeof(MAGIC)];
	strcpy_s(msg.szMagic, sizeof(msg.szMagic), MAGIC);
	//time;
	msg.time.year = parts.tm_year;
	msg.time.month = parts.tm_mon;
	msg.time.date = parts.tm_mday;
	msg.time.hour = parts.tm_hour;
	msg.time.minite = parts.tm_min;
	msg.time.second = parts.tm_sec;
	msg.time.miliseconds = 0;
	//uint32_t process_id;
	msg.process_id = pid;
	//std::wstring processName;
	msg.processName = processName;
	//uint32_t parent_process_id;
	msg.parent_process_id = ppid;
	//std::wstring parentProcessName;
	msg.parentProcessName = parentProcessName;
	//std::wstring activity; // (read/write/delete/etc.)
	msg.activity = action;
	//std::wstring filePath;
	msg.filePath = L"";
	//uint32_t fileSize;
	msg.fileSize = 0;
	//uint32_t readDataLength;
	msg.readDataLength = 0;
	//uint32_t writeDataLength;
	msg.writeDataLength = 0;

	////uint8_t* snapshot;
	//msg.snapshot = nullptr;// (uint8_t*)&msg;
	////uint32_t snapshotLength;
	//msg.snapshotLength = 0;// sizeof(msg);

	HMODULE hModules[16];
	DWORD cbModules = ARRAYSIZE(hModules);
	if (EnumProcessModules(hProcess, hModules, cbModules, &cbModules) && cbModules > 0) {
		MODULEINFO modinfo;
		if (GetModuleInformation(hProcess, hModules[0], &modinfo, sizeof(modinfo))) {
			size_t szMem = modinfo.SizeOfImage > SNAPSHOT_SIZE ? SNAPSHOT_SIZE : modinfo.SizeOfImage;
			msg.snapshot = (uint8_t*)malloc(szMem);
			msg.snapshotLength = (uint32_t)szMem;
			SIZE_T szRead = 0;
			if (!ReadProcessMemory(hProcess, modinfo.EntryPoint, msg.snapshot, szMem, &szRead)) {
				free(msg.snapshot);
				msg.snapshot = nullptr;
				msg.snapshotLength = 0;
			}
		}
	}

	//uint32_t length;
	msg.length = (uint32_t)(
		sizeof(msg.length) + DELIMITER_SIZE
		+ sizeof(MAGIC) - 1 + DELIMITER_SIZE
		+ sizeof(uint32_t) + 6 + DELIMITER_SIZE										// time
		+ sizeof(uint32_t) + DELIMITER_SIZE											// process_id
		+ sizeof(wchar_t) * (1 + msg.processName.length()) + DELIMITER_SIZE			// processName
		+ sizeof(uint32_t) + DELIMITER_SIZE											// parent_process_id
		+ sizeof(wchar_t) * (1 + msg.parentProcessName.length()) + DELIMITER_SIZE	// parentProcessName
		+ sizeof(wchar_t) * (1 + msg.activity.length()) + DELIMITER_SIZE			// activity // (read/write/delete/etc.)
		+ sizeof(wchar_t) * (1 + msg.filePath.length()) + DELIMITER_SIZE			// filePath
		+ sizeof(uint32_t) + DELIMITER_SIZE											// fileSize
		+ sizeof(uint32_t) + DELIMITER_SIZE											// readDataLength
		+ sizeof(uint32_t) + DELIMITER_SIZE											// writeDataLength
		+ msg.snapshotLength// snapshot
		);

	err = sendData(sock, msg);

	closesocket(sock);

	if (msg.snapshot != nullptr) {
		free(msg.snapshot);
		msg.snapshot = nullptr;
		msg.snapshotLength = 0;
	}

	return err;
}
