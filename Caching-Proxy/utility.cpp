#include "utility.h"

#include <cstdlib>
#include <iostream>

void parse_cli(int argc, char *argv[],
							 std::string &port, std::string &origin) {
	for (int i = 1; i < argc && argv[i] != nullptr; ++i) {
		if (strcmp(argv[i], "--port") == 0) {
			if (argv[i + 1] != nullptr) {
				port = argv[i + 1];
			}
			else {
				std::cerr << "error: missing port number" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		if (strcmp(argv[i], "--origin") == 0) {
			if (argv[i + 1] != nullptr) {
				origin = argv[i + 1];
			}
			else {
				std::cerr << "error: missing origin's url" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
	}
}

PCTSTR address_to_string(ADDRINFO *info, PTSTR buf) {
	switch (info->ai_family) {
	case AF_UNSPEC:
	case AF_NETBIOS:
		return NULL;
	case AF_INET:
		return InetNtop(
			info->ai_family,
			&reinterpret_cast<struct sockaddr_in *>(info->ai_addr)->sin_addr,
			buf,
			INET_ADDRSTRLEN);
	case AF_INET6:
		return InetNtop(
			info->ai_family,
			&reinterpret_cast<struct sockaddr_in6 *>(info->ai_addr)->sin6_addr,
			buf,
			INET6_ADDRSTRLEN);
	default:
		break;
	}
	return NULL;
}