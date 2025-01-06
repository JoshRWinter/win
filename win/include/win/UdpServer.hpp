#pragma once

#include <win/Win.hpp>

#undef distance
#include <cstring>
#ifdef WINPLAT_WINDOWS
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <ws2tcpip.h>
#define ssize_t SSIZE_T
#else
#include <netinet/in.h>
#include <netdb.h>
#endif

namespace win {

struct UdpId
{
	WIN_NO_COPY_MOVE(UdpId);

	UdpId()
		: len(sizeof(sockaddr_storage))
	{
		reset();
	}

	void reset()
	{
		memset(&storage, 0, sizeof(storage));
		initialized = false;
	}

	bool initialized;
	sockaddr_storage storage;
	socklen_t len;
};

class UdpServer
{
	WIN_NO_COPY(UdpServer);

#ifdef WINPLAT_WINDOWS
	static constexpr int WOULDBLOCK = WSAEWOULDBLOCK;
	static constexpr int CONNRESET = WSAECONNRESET;
#else
	static constexpr int WOULDBLOCK = EWOULDBLOCK;
	static constexpr int CONNRESET = ECONNRESET;
#endif

public:
	UdpServer();
	explicit UdpServer(unsigned short port);
	UdpServer(UdpServer &&rhs) noexcept;
	~UdpServer();

	UdpServer &operator=(UdpServer &&rhs) noexcept;

	operator bool() const;
	void close();
	void send(const void *buffer, int len, const UdpId &id);
	int recv(void *buffer, int len, UdpId &id);
	unsigned peek();
	bool error() const;

private:
	bool bind(unsigned short port);

	int sock;
};

}
