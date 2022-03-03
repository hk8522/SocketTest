#pragma once

#include <stdint.h>
#include <string>

#define PORT_NO 5055

#define MAGIC "QZW-0001-0009"
#define DELIMITER 0x00FF00
#define DELIMITER_SIZE 3

#pragma pack(push, 1)
typedef struct __PROTOCOL_MESSAGE
{
	uint32_t length;
	char szMagic[sizeof(MAGIC)];
	uint32_t datetime;
	uint32_t process_id;
	std::string processName;
	uint32_t parent_process_id;
	std::string parentProcessName;
	std::string fileSystemActivity; // (read/write/delete/etc.)
	std::string filePath;
	uint32_t fileSize;
	uint32_t readDataLength;
	uint32_t writeDataLength;

	uint8_t* snapshot;
	uint32_t snapshotLength;
} PROTOCOL_MESSAGE, *PPROTOCOL_MESSAGE;

#pragma pack(pop)
