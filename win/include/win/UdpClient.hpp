#pragma once

#include <win/Win.hpp>

#undef distance
#include <string>
#ifdef WINPLAT_WINDOWS
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#define ssize_t SSIZE_T
#else
#include <netinet/in.h>
#include <netdb.h>
#endif

namespace win {


class UdpClient
{
	WIN_NO_COPY(UdpClient);

#ifdef WINPLAT_WINDOWS
	static constexpr int WOULDBLOCK = WSAEWOULDBLOCK;
	static constexpr int CONNRESET = WSAECONNRESET;
#else
	static constexpr int WOULDBLOCK = EWOULDBLOCK;
	static constexpr int CONNRESET = ECONNRESET;
#endif

public:
	UdpClient();
	UdpClient(const char *address, unsigned short port);
	UdpClient(UdpClient &&rhs) noexcept;
	~UdpClient();

	UdpClient &operator=(UdpClient &&rhs) noexcept;

	operator bool() const;
	void close();
	void send(const void *buffer, unsigned len);
	int recv(void *buffer, unsigned len);
	unsigned peek();
	bool error() const;

private:
	int sock;
	addrinfo *ai;
};

}
