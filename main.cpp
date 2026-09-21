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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdlib>
#include <iostream>
#include <string_view>

#pragma comment(lib, "Ws2_32.lib")

constexpr int DEFAULT_BUFLEN = 1 << 13;
constexpr std::string_view DEFAULT_PORT = "27015";

int __cdecl main(int argc, char **argv) {
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

	SOCKET ListenSocket = INVALID_SOCKET;

	struct addrinfo *result{ nullptr };
	struct addrinfo hints {};

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(nullptr, (argc > 1 ? argv[1] : DEFAULT_PORT.data()), &hints, &result);
	if (iResult != 0) {
		std::cerr << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	WSACleanup();

	return EXIT_SUCCESS;
}