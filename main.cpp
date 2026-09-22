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

#ifndef UNICODE
#define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

#pragma comment(lib, "Ws2_32.lib")

constexpr int DEFAULT_BUFLEN = 1 << 13;
constexpr std::string_view DEFAULT_PORT = "48012";

int __cdecl main(int argc, char **argv) {
	std::string port{};

	// Read command line arguments
	for (int i = 1; i < argc && argv[i] != nullptr; ++i) {
		if (strcmp(argv[i], "--port") == 0) {
			if (argv[i + 1] != nullptr) {
				port = argv[i + 1];
			}
			else {
				std::cerr << "error: missing port number" << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	WSADATA wsaData{};
	int iResult{};

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cerr << "WSAStartup failed: " << iResult << std::endl;
		return EXIT_FAILURE;
	}

	// Ensure that WinSock DLL supports 2.2.
	if (LOBYTE(wsaData.wVersion) != 2
		|| HIBYTE(wsaData.wVersion) != 2)
	{
		std::cerr << "Unable to find a usable WinSock DLL" << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	struct addrinfo *result{ nullptr };
	struct addrinfo hints {};

	ZeroMemory(&hints, sizeof(hints));
	// TODO: Handle IPv6 properly
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	//hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	// TODO: Handle --port <number>
	iResult = getaddrinfo("www.example.com",
		port.empty() ? DEFAULT_PORT.data() : port.c_str(), &hints, &result);
	if (iResult != 0) {
		std::cerr << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	// TODO: Maybe add logging instead of standart I/O
	std::cout << "Set caching proxy server port to " << DEFAULT_PORT << std::endl;

	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *ptr{ nullptr };

	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}

		break; // connection established
	}

	freeaddrinfo(result);
	closesocket(ConnectSocket);
	WSACleanup();

	return EXIT_SUCCESS;
}