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
#include <iostream>
#include <string>
#include <string_view>

#pragma comment(lib, "Ws2_32.lib")

constexpr int DEFAULT_BUFLEN{ 1 << 13 };
constexpr std::string_view DEFAULT_PORT{ "48012" };
constexpr std::string_view HTTP_PORT{ "80" };

PCTSTR address_to_string(ADDRINFO*, PTSTR, size_t);

int __cdecl main(int argc, char **argv) {
	std::string port{};
	std::string domainName{};

	// TODO: obtain domain/port from command line arguments
	domainName = "www.example.com";
	port = DEFAULT_PORT;

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

	// TODO: Maybe add logging instead of standart I/O
	std::cout << "Set caching proxy server port to " << port << std::endl;

	WSADATA wsaData{};
	int iResult{};

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cerr << "WSAStartup failed: " << iResult << std::endl;
		return EXIT_FAILURE;
	}

	// Ensure that WinSock DLL supports 2.2.
	if (LOBYTE(wsaData.wVersion) != 2 ||
			HIBYTE(wsaData.wVersion) != 2) {
		std::cerr << "Unable to find a usable WinSock DLL" << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	ADDRINFO *result{ NULL };
	ADDRINFO hints{};

	ZeroMemory(&hints, sizeof(hints));
	// TODO: Handle IPv6 properly
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_CANONNAME;

	// Resolve the server address and port
	// TODO: Handle --port <number>
	iResult = GetAddrInfo("www.example.com", // give it a test
												"http",
												&hints,
												&result);
	if (iResult != 0) {
		std::cerr << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	SOCKET ConnectSocket{ INVALID_SOCKET };
	ADDRINFO *ptr{ NULL };

	// Process the linked list of addrinfo structures
	// to find ...
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET){
			std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return EXIT_FAILURE;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			iResult = closesocket(ConnectSocket);
			if (iResult == SOCKET_ERROR) {
				std::cerr << "closesocket failed with error: " << WSAGetLastError() << std::endl;
				WSACleanup();
				return EXIT_FAILURE;
			}
			ConnectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		std::cerr << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	CHAR ipstrbuf[NI_MAXHOST]{};
	if (address_to_string(ptr, ipstrbuf, sizeof ipstrbuf)) {
		std::cout << "origin ip: " << ipstrbuf << std::endl;
	}

	int recvbuflen{ DEFAULT_BUFLEN };

	const char *sendbuf{ "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n" };
	char recvbuf[DEFAULT_BUFLEN]{};

	iResult = send(ConnectSocket,
								 sendbuf,
								 static_cast<int>(strlen(sendbuf)),
								 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	std::cout << "bytes sent: " << iResult << '\n';

	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			std::cout << "bytes received: " << iResult << '\n';
		else if (iResult == 0)
			std::cout << "connection closed\n";
		else
			std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
	} while (iResult > 0);
	
	std::cout << recvbuf << '\n';

	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "shutdown failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	iResult = closesocket(ConnectSocket);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "closesocket failed with error: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return EXIT_FAILURE;
	}

	WSACleanup();
	return EXIT_SUCCESS;
}

PCTSTR address_to_string(ADDRINFO * info, PTSTR buf, size_t bufSize) {
	switch (info->ai_family) {
	case AF_UNSPEC:
		return NULL;
	case AF_INET:
		return InetNtop(
			info->ai_family,
			&reinterpret_cast<struct sockaddr_in *>(info->ai_addr)->sin_addr,
			buf,
			bufSize);
	case AF_INET6:
		// TODO
		return NULL;
	case AF_NETBIOS:
		return NULL;
	default:
		break;
	}
	return NULL;
}