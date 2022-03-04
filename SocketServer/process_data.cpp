#include "SocketServer.h"

#include <format>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

using namespace std;

#define CHECK_GAP \
	if ((*(uint32_t*)(buffer + offset) & 0x00FFFFFF) != DELIMITER) \
		return EXIT_FAILURE; \
	offset += DELIMITER_SIZE

int write_msg(const char* filename, PROTOCOL_MESSAGE * msg);
int read_string(char* buffer, size_t length, int& offset, std::wstring & str);

int process_data(SOCKET sock, sockaddr_in *peer)
{
	uint8_t* buffer;
	int valread;
	int received;
	int offset;
	char addrbuff[64];

	PROTOCOL_MESSAGE msg;

	//uint32_t length;
	if ((valread = recv(sock, (char *)&msg.length, sizeof(msg.length), 0)) < sizeof(msg.length))
		return EXIT_FAILURE;
	msg.length = ntohl(msg.length);

	buffer = (uint8_t*)malloc(msg.length);
	if (buffer == NULL)
		return EXIT_FAILURE;

	received = sizeof(msg.length);
	while (received < (int)msg.length) {
		valread = recv(sock, (char*)buffer + received, msg.length - received, 0);
		if (valread <= 0)
			return EXIT_FAILURE;
		received += valread;
	}

	offset = sizeof(msg.length);
	CHECK_GAP;

	//char szMagic[sizeof(MAGIC)];
	if (strncmp((char*)(buffer + offset), MAGIC, sizeof(MAGIC) - 1) != 0)
		return EXIT_FAILURE;
	offset += sizeof(MAGIC) - 1;
	CHECK_GAP;

	//uint32_t datetime;
	msg.time.year = ntohl(*(uint32_t*)(buffer + offset)); offset += sizeof(uint32_t);
	msg.time.month = *(uint8_t*)(buffer + offset++);
	msg.time.date = *(uint8_t*)(buffer + offset++);
	msg.time.hour = *(uint8_t*)(buffer + offset++);
	msg.time.minite = *(uint8_t*)(buffer + offset++);
	msg.time.second = *(uint8_t*)(buffer + offset++);
	msg.time.miliseconds = *(uint8_t*)(buffer + offset++);
	CHECK_GAP;

	//uint32_t process_id;
	msg.process_id = ntohl(*(uint32_t*)(buffer + offset));
	offset += sizeof(uint32_t);
	CHECK_GAP;

	//std::string processName;
	if (read_string((char *)buffer, msg.length, offset, msg.processName) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	CHECK_GAP;

	//uint32_t parent_process_id;
	msg.parent_process_id = ntohl(*(uint32_t*)(buffer + offset));
	offset += sizeof(uint32_t);
	CHECK_GAP;

	//std::string parentProcessName;
	if (read_string((char*)buffer, msg.length, offset, msg.parentProcessName) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	CHECK_GAP;

	//std::string fileSystemActivity; // (read/write/delete/etc.)
	if (read_string((char*)buffer, msg.length, offset, msg.fileSystemActivity) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	CHECK_GAP;

	//std::string filePath;
	if (read_string((char*)buffer, msg.length, offset, msg.filePath) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	CHECK_GAP;

	//uint32_t fileSize;
	msg.fileSize = ntohl(*(uint32_t*)(buffer + offset));
	offset += sizeof(uint32_t);
	CHECK_GAP;

	//uint32_t readDataLength;
	msg.readDataLength = ntohl(*(uint32_t*)(buffer + offset));
	offset += sizeof(uint32_t);
	CHECK_GAP;

	//uint32_t writeDataLength;
	msg.writeDataLength = ntohl(*(uint32_t*)(buffer + offset));
	offset += sizeof(uint32_t);
	CHECK_GAP;

	msg.snapshot = (uint8_t*)(buffer + offset);
	msg.snapshotLength = msg.length - offset;

	// filename format: Fixed Value (field 1) _ IP Address _ YYYYMMDD_HH.txt
	std::tm parts{};
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	localtime_s(&parts, &now_c);

	stringstream ss;
	ss << MAGIC
		<< "_"
		<< inet_ntop(peer->sin_family, &peer->sin_addr, addrbuff, sizeof(addrbuff))
		<< "_"
		<< setfill('0') << setw(4) << (parts.tm_year + 1900)
		<< setfill('0') << setw(2) << (parts.tm_mon + 1)
		<< setfill('0') << setw(2) << parts.tm_mday
		<< setw(1) << "_"
		<< setfill('0') << setw(2) << parts.tm_hour
		<< ".txt";
	return write_msg(ss.str().c_str(), &msg);
}

int write_msg(const char* filename, PROTOCOL_MESSAGE* msg)
{
	int offset;

	wofstream file(filename, std::ios_base::app);
	if (!file.is_open())
		return EXIT_FAILURE;

	file << "Total byte-length of this record: " << msg->length << endl
		<< "Fixed value: " << MAGIC << endl
		<< "Date and time of event: "
		<< (msg->time.year + 1900) << "/" << (msg->time.month + 1) << "/" << msg->time.date << " " << msg->time.hour << ":" << msg->time.minite << ":" << msg->time.second /*<< "." << msg->time.miliseconds*/ << endl
		<< "Process ID initiating file access: " << msg->process_id << endl
		<< "Process name: " << msg->processName << endl
		<< "Parent Process ID: " << msg->parent_process_id << endl
		<< "Parent Process name: " << msg->parentProcessName << endl
		<< "File system activity: " << msg->fileSystemActivity << endl
		<< "File path and name: " << msg->filePath << endl
		<< "File size: " << msg->fileSize << endl
		<< "Read data length: " << msg->readDataLength << endl
		<< "Write data length: " << msg->writeDataLength << endl
		<< "Snapshot of the process memory block(s):" << endl;

	for (offset = 0; offset < (int)msg->snapshotLength; offset++) {
		file << std::hex << setw(2) << setfill(L'0') << msg->snapshot[offset];
		if (offset % 16 == 0)
			file << endl;
		else if (offset % 8 == 0)
			file << "   ";
		else
			file << " ";
	}
	file << endl;
	file.close();

	return 0;
}

int read_string(char* buffer, size_t length, int& offset, std::wstring& str)
{
	size_t strlength;
	if (offset >= length - 1)
		return EXIT_FAILURE;

	for (strlength = 0; offset + strlength < (length - 1); strlength += sizeof(wchar_t)) {
		if (*(wchar_t*)&buffer[offset + strlength] == L'\0')
			break;
	}
	*(wchar_t*)(buffer + offset + strlength) = L'\0';
	strlength += sizeof(wchar_t);
	str.reserve(strlength / sizeof(wchar_t));
	str = (const wchar_t*)(buffer + offset);
	offset += (int)strlength;

	return EXIT_SUCCESS;
}
