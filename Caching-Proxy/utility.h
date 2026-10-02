#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WS2tcpip.h>

#include <string>

void parse_cli(int argc, char *argv[],
							 std::string &port, std::string &origin);

PCTSTR address_to_string(ADDRINFO *, PTSTR);