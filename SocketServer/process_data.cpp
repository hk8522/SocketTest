#include "SocketServer.h"

#include <format>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;

#define CHECK_GAP \
	if ((*(uint32_t*)(buffer + offset) & 0x00FFFFFF) != DELIMITER) \
		return EXIT_FAILURE

int write_msg(const char* filename, PROTOCOL_MESSAGE * msg);

int process_data(SOCKET sock, sockaddr_in *peer)
{
	uint8_t* buffer;
	int valread;
	int received;
	int offset;
	int strlength;
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
	CHECK_GAP;
	offset += sizeof(MAGIC) - 1 + DELIMITER_SIZE;

	//uint32_t datetime;
	msg.datetime = ntohl(*(uint32_t*)(buffer + offset));
	CHECK_GAP;
	offset += sizeof(uint32_t) + DELIMITER_SIZE;

	//uint32_t process_id;
	msg.process_id = ntohl(*(uint32_t*)(buffer + offset));
	CHECK_GAP;
	offset += sizeof(uint32_t) + DELIMITER_SIZE;

	//std::string processName;
	for (strlength = 0; offset + strlength < (int)msg.length; strlength++) {
		if (buffer[offset + strlength] == '\0')
			break;
	}
	msg.processName.reserve(strlength + 1);
	msg.processName = (char*)(buffer + offset);
	CHECK_GAP;
	offset += strlength + DELIMITER_SIZE;

	//uint32_t parent_process_id;
	msg.parent_process_id = ntohl(*(uint32_t*)(buffer + offset));
	CHECK_GAP;
	offset += sizeof(uint32_t) + DELIMITER_SIZE;

	//std::string parentProcessName;
	for (strlength = 0; offset + strlength < (int)msg.length; strlength++) {
		if (buffer[offset + strlength] == '\0')
			break;
	}
	msg.parentProcessName.reserve(strlength + 1);
	msg.parentProcessName = (char*)(buffer + offset);
	CHECK_GAP;
	offset += strlength + DELIMITER_SIZE;

	//std::string fileSystemActivity; // (read/write/delete/etc.)
	for (strlength = 0; offset + strlength < (int)msg.length; strlength++) {
		if (buffer[offset + strlength] == '\0')
			break;
	}
	msg.fileSystemActivity.reserve(strlength + 1);
	msg.fileSystemActivity = (char*)(buffer + offset);
	CHECK_GAP;
	offset += strlength + DELIMITER_SIZE;

	//std::string filePath;
	for (strlength = 0; offset + strlength < (int)msg.length; strlength++) {
		if (buffer[offset + strlength] == '\0')
			break;
	}
	msg.filePath.reserve(strlength + 1);
	msg.filePath = (char*)(buffer + offset);
	CHECK_GAP;
	offset += strlength + DELIMITER_SIZE;

	//uint32_t fileSize;
	msg.fileSize = ntohl(*(uint32_t*)(buffer + offset));
	CHECK_GAP;
	offset += sizeof(uint32_t) + DELIMITER_SIZE;

	//uint32_t readDataLength;
	msg.readDataLength = ntohl(*(uint32_t*)(buffer + offset));
	CHECK_GAP;
	offset += sizeof(uint32_t) + DELIMITER_SIZE;

	//uint32_t writeDataLength;
	msg.writeDataLength = ntohl(*(uint32_t*)(buffer + offset));
	CHECK_GAP;
	offset += sizeof(uint32_t) + DELIMITER_SIZE;

	msg.snapshot = (uint8_t*)(buffer + offset);
	msg.snapshotLength = msg.length - offset;

	// filename format: Fixed Value (field 1) _ IP Address
	string filename;
	filename = MAGIC;
	filename = "_";
	filename += inet_ntop(peer->sin_family, &peer->sin_addr, addrbuff, sizeof(addrbuff));
	return write_msg(filename.c_str(), &msg);

	return 0;
}

int write_msg(const char* filename, PROTOCOL_MESSAGE* msg)
{
	int offset;
	char timebuff[64];
	errno_t err;

	ofstream file(filename);
	if (!file.is_open())
		return EXIT_FAILURE;

	err = ctime_s(timebuff, sizeof(timebuff), (time_t*) & msg->datetime);
	if (err != 0)
		return err;

	file << "Total byte-length of this record: " << msg->length << endl
		<< "Fixed value: " << MAGIC << endl
		<< "Date and time of event: " << timebuff << endl
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
		file << std::hex << msg->snapshot[offset];
		if (offset % 16 == 0)
			file << endl;
		else if (offset % 8 == 0)
			file << "   ";
		else
			file << " ";
	}
	file << endl;

	return 0;
}
