// Server side C/C++ program to demonstrate Socket programming

#include "SocketServer.h"

int main(int argc, char const* argv[])
{
	SOCKET server_fd, new_socket;
	struct sockaddr_in address;
	struct sockaddr_in peer;
	int opt = 1;
	int addrlen = sizeof(address);
	int err;

	err = network_init();
	if (err != 0) {
		fprintf(stderr, "Network initialization failed.\n");
		return err;
	}

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		perror("socket failed");
		network_cleanup();
		return EXIT_FAILURE;
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		(const char *)&opt, sizeof(opt)))
	{
		perror("setsockopt");
		network_cleanup();
		return EXIT_FAILURE;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT_NO);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address,
		sizeof(address)) < 0)
	{
		perror("bind failed");
		network_cleanup();
		return EXIT_FAILURE;
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		network_cleanup();
		return EXIT_FAILURE;
	}
	if ((new_socket = accept(server_fd, (struct sockaddr*)&peer,
		(socklen_t*)&addrlen)) < 0)
	{
		perror("accept");
		network_cleanup();
		return EXIT_FAILURE;
	}

	process_data(new_socket, &peer);

	closesocket(new_socket);
	closesocket(server_fd);

	network_cleanup();

	return 0;
}
