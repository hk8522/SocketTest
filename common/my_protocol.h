#pragma once

#include <stdint.h>
#include <string>
#include <ctime>

#define PORT_NO 5055

#define MAGIC "QZW-0001-0009"
#define DELIMITER 0x00FF00
#define DELIMITER_SIZE 3

#pragma pack(push, 1)
typedef struct __PROTOCOL_MESSAGE
{
	uint32_t length;
	char szMagic[sizeof(MAGIC)];
	struct {
		uint32_t year;
		uint8_t month;
		uint8_t date;
		uint8_t hour;
		uint8_t minite;
		uint8_t second;
		uint8_t miliseconds;
	} time;
	uint32_t process_id;
	std::wstring processName;
	uint32_t parent_process_id;
	std::wstring parentProcessName;
	std::wstring fileSystemActivity; // (read/write/delete/etc.)
	std::wstring filePath;
	uint32_t fileSize;
	uint32_t readDataLength;
	uint32_t writeDataLength;

	uint8_t* snapshot;
	uint32_t snapshotLength;
} PROTOCOL_MESSAGE, *PPROTOCOL_MESSAGE;

#pragma pack(pop)
