//	Module:
//		main.cpp
//
//	Abstract:
//		TODO
//
//	Usage:
//		Start the caching proxy server and wait for connections from origin <url>
//		on port <number>.
//			caching-proxy --port <number> --origin <url>
// 
//	Build:
//		Link with Ws2_32.lib

// For now I will disable any unicode to
// use Windows code page versions of functions
#undef UNICODE
#undef _UNICODE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

#include "utility.h"

#pragma comment(lib, "Ws2_32.lib")

constexpr int DEFAULT_BUFLEN{ 1 << 13 };
constexpr std::string_view DEFAULT_PORT{ "48012" };
constexpr std::string_view HTTP_PORT{ "80" };

int __cdecl main(int argc, char **argv) {
	std::string port{ DEFAULT_PORT };
	std::string origin{};

	// Parse command line arguments
	parse_cli(argc, argv, port, origin);

	// TODO: Maybe add logging instead of standart I/O
	std::cout << std::format("Set caching proxy server port to {}\n", port);

	WSADATA wsaData{};
	int iResult{};

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cerr << std::format("WSAStartup failed: {}\n", iResult) << std::flush;
		return EXIT_FAILURE;
	}

	// Ensure that WinSock DLL supports 2.2.
	if (LOBYTE(wsaData.wVersion) != 2 ||
			HIBYTE(wsaData.wVersion) != 2) {
		std::cerr << "Unable to find a usable WinSock DLL" << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	ADDRINFO *result{ NULL };	 // linked-list of all possible address information
	ADDRINFO hints{};

	ZeroMemory(&hints, sizeof(hints));
	// TODO: Handle IPv6 properly
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = GetAddrInfo(origin.c_str(),
												"http",
												&hints,
												&result);
	if (iResult != 0) {
		std::cerr << std::format("getaddrinfo failed: {}\n", iResult) << std::flush;
		WSACleanup();
		return EXIT_FAILURE;
	}

	SOCKET ConnectSocket{ INVALID_SOCKET };
	ADDRINFO *ptr{ NULL }; // current element in the linked-list

	// Process the linked list of addrinfo structures
	// to find ...
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			std::cerr << std::format("socket failed with error: {}\n",  WSAGetLastError()) << std::flush;
			WSACleanup();
			return EXIT_FAILURE;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
		if (iResult == SOCKET_ERROR) {
			iResult = closesocket(ConnectSocket);
			if (iResult == SOCKET_ERROR) {
				std::cerr << std::format("closesocket failed with error: {}\n", WSAGetLastError()) << std::flush;
				WSACleanup();
				return EXIT_FAILURE;
			}
			ConnectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result); // free the linked-list

	if (ConnectSocket == INVALID_SOCKET) {
		std::cerr << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	CHAR ipstrbuf[NI_MAXHOST]{};
	if (address_to_string(ptr, ipstrbuf)) {
		std::cout << std::format("origin ip: {}\n", ipstrbuf);
	}

	int recvbuflen{ DEFAULT_BUFLEN };

	std::string sendbuf{ std::format(
		"GET {} HTTP/1.1\r\nHost: {}\r\nConnection: close\r\n\r\n",
		"/",
		origin)};
	//const char *sendbuf{ "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n" };
	char recvbuf[DEFAULT_BUFLEN]{};

	iResult = send(ConnectSocket,
								 sendbuf.c_str(),
								 static_cast<int>(sendbuf.size()),
								 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << std::format("send failed with error: {}\n", WSAGetLastError()) << std::flush;
		closesocket(ConnectSocket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	std::cout << std::format("bytes sent: {}\n", iResult);

	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			std::cout << std::format("bytes received: {}\n", iResult);
		else if (iResult == 0)
			std::cout << "connection closed\n";
		else
			std::cerr << std::format("recv failed with error: {}\n", WSAGetLastError()) << std::flush;
	} while (iResult > 0);

	std::cout << std::format("response: \n{}", recvbuf);

	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cerr << std::format("shutdown failed with error: {}\n", WSAGetLastError()) << std::flush;
		closesocket(ConnectSocket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	iResult = closesocket(ConnectSocket);
	if (iResult == SOCKET_ERROR) {
		std::cerr << std::format("closesocket failed with error: {}\n", WSAGetLastError()) << std::flush;
		WSACleanup();
		return EXIT_FAILURE;
	}

	WSACleanup();
	return EXIT_SUCCESS;
}