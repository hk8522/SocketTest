#pragma once

#ifdef _WIN32
# include <WinSock2.h>
# include <ws2tcpip.h>
# include <io.h>
# define SO_REUSEPORT SO_REUSEADDR
# pragma comment(lib, "Ws2_32.lib")
#else
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_protocol.h"

int network_init();
void network_cleanup();
