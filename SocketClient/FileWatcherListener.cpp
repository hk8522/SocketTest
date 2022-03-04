#include "network_init.h"
#include "FileWatcherListener.h"
#include <iostream>
#include <conio.h>
#include <memory.h>
#include <tlhelp32.h>
#include <chrono>

using namespace std;

#define SERVER_ADDR "127.0.0.1"

#define SEND_GAP															\
	{																		\
		uint32_t gap = DELIMITER;   										\
		if (send(sock, (char*)&gap, DELIMITER_SIZE, 0) < DELIMITER_SIZE)	\
			return EXIT_FAILURE;   											\
	}


int logToServer(const std::wstring& path, const std::wstring& action);
int sendData(SOCKET sock, const std::wstring& path, const std::wstring& action);

static uint32_t getParentProcessId(uint32_t pid, std::wstring &exeFile)
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

__int64 FileSize(const wchar_t* name)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
		return -1; // error condition, could call GetLastError to find out more
	LARGE_INTEGER size;
	size.HighPart = fad.nFileSizeHigh;
	size.LowPart = fad.nFileSizeLow;
	return size.QuadPart;
}

// Inherited via IFileWatcherListener
void FileWatcherListener::OnFileChange(const std::wstring& path)
{
	wcout << L"OnFileChange " << path << endl;
	logToServer(path, L"Change");
}

// Inherited via IFileWatcherListener
void FileWatcherListener::OnFileAdded(const std::wstring& path)
{
	wcout << L"OnFileAdded " << path << endl;
	logToServer(path, L"Created");
}

// Inherited via IFileWatcherListener
void FileWatcherListener::OnFileRemoved(const std::wstring& path)
{
	wcout << L"OnFileRemoved " << path << endl;
	logToServer(path, L"Deleted");
}

// Inherited via IFileWatcherListener
void FileWatcherListener::OnFileRenamed(const std::wstring& path)
{
	wcout << L"OnFileRenamed " << path << endl;
	logToServer(path, L"Renamed");
}

int logToServer(const std::wstring& path, const std::wstring &action)
{
	SOCKET sock;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	int err;

	err = network_init();
	if (err != 0) {
		fprintf(stderr, "Network initialization failed.\n");
		return err;
	}

	// Creating socket file descriptor
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		perror("socket failed");
		network_cleanup();
		return EXIT_FAILURE;
	}

	if (!inet_pton(AF_INET, SERVER_ADDR, &address.sin_addr))
		return EXIT_FAILURE;
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT_NO);

	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		perror("connect failed");
		network_cleanup();
		return EXIT_FAILURE;
	}

	sendData(sock, path, action);

	closesocket(sock);

	network_cleanup();

	return 0;
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

int sendData(SOCKET sock, const std::wstring& path, const std::wstring& action)
{
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
	msg.process_id = GetCurrentProcessId();
	//std::wstring processName;
	msg.processName = szFileName;
	//uint32_t parent_process_id;
	//std::wstring parentProcessName;
	msg.parent_process_id = getParentProcessId(msg.process_id, msg.parentProcessName);
	//std::wstring fileSystemActivity; // (read/write/delete/etc.)
	msg.fileSystemActivity = action;
	//std::wstring filePath;
	msg.filePath = path;
	//uint32_t fileSize;
	msg.fileSize = (uint32_t)FileSize(path.c_str());
	//uint32_t readDataLength;
	msg.readDataLength = 0;
	//uint32_t writeDataLength;
	msg.writeDataLength = 0;

	//uint8_t* snapshot;
	//uint32_t snapshotLength;

	//uint32_t length;
	msg.length = (uint32_t)(
		sizeof(msg.length) + DELIMITER_SIZE
		+ sizeof(MAGIC) - 1 + DELIMITER_SIZE
		+ sizeof(uint32_t) + 6 + DELIMITER_SIZE									// time
		+ sizeof(uint32_t) + DELIMITER_SIZE										// process_id
		+ sizeof(wchar_t) * (1 + msg.processName.length()) + DELIMITER_SIZE			// processName
		+ sizeof(uint32_t) + DELIMITER_SIZE										// parent_process_id
		+ sizeof(wchar_t) * (1 + msg.parentProcessName.length()) + DELIMITER_SIZE		// parentProcessName
		+ sizeof(wchar_t) * (1 + msg.fileSystemActivity.length()) + DELIMITER_SIZE	// fileSystemActivity // (read/write/delete/etc.)
		+ sizeof(wchar_t) * (1 + msg.filePath.length()) + DELIMITER_SIZE				// filePath
		+ sizeof(uint32_t) + DELIMITER_SIZE										// fileSize
		+ sizeof(uint32_t) + DELIMITER_SIZE										// readDataLength
		+ sizeof(uint32_t) + DELIMITER_SIZE										// writeDataLength
		+ sizeof(msg)// snapshot
		);

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

	//std::string fileSystemActivity; // (read/write/delete/etc.)
	if (send_string(sock, msg.fileSystemActivity) != EXIT_SUCCESS)
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
	if (send(sock, (char*)&msg, sizeof(msg), 0) < sizeof(msg))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
