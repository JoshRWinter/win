#include <win/Win.hpp>

#include <cstring>

#ifdef WINPLAT_LINUX
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#else
#include <Ws2tcpip.h>
#endif

#include <win/WsaSingleton.hpp>
#include <win/UdpClient.hpp>

namespace win
{

// errno related stuff
#define NET_WOULDBLOCK
static int get_errno(){
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif // _WIN32
}

UdpClient::UdpClient()
{
	initialize_wsa();

	ai = NULL;
	sock = -1;
}

UdpClient::UdpClient(const char *address, unsigned short port)
{
	ai = NULL;
	sock = -1;

	addrinfo hints;

	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	// convert port to string
	char port_string[20];
	sprintf(port_string,"%hu",port);

	// resolve hostname
	if (getaddrinfo(address, port_string, (const addrinfo*)&hints, &ai) != 0)
		return;

	// create the socket
	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if(sock == -1)
		return;

	// set to non blocking
#ifdef _WIN32
	u_long nonblock = 1;
	ioctlsocket(sock, FIONBIO, &nonblock);
#else
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK); // set to non blocking
#endif
}

UdpClient::UdpClient(UdpClient &&rhs) noexcept
{
	sock = rhs.sock;
	ai = rhs.ai;

	rhs.sock = -1;
	rhs.ai = NULL;
}

UdpClient::~UdpClient()
{
	this->close();
}

UdpClient &UdpClient::operator=(UdpClient &&rhs) noexcept
{
	close();

	sock = rhs.sock;
	ai = rhs.ai;

	rhs.sock = -1;
	rhs.ai = NULL;

	return *this;
}

UdpClient::operator bool() const
{
	return !error();
}

void UdpClient::send(const void *buffer, unsigned len)
{
	if (sock == -1)
		bug("UdpClient closed");

	// no such thing as a partial send for udp with sendto
	const ssize_t result = sendto(sock, (const char*)buffer, len, 0, ai->ai_addr, ai->ai_addrlen);
	if ((unsigned)result != len && get_errno() != WOULDBLOCK)
		this->close();
}

int UdpClient::recv(void *buffer, unsigned len)
{
	if (sock == -1)
		bug("UdpClient closed");

	// no such thing as a partial send for udp with sendto
	const ssize_t result = recvfrom(sock, (char*)buffer, len, 0, NULL, NULL);
	if (result == -1)
	{
		const auto eno = get_errno();
		if(eno == WOULDBLOCK || eno == CONNRESET)
			return 0;
		else
			this->close();

		return 0;
	}

	return result;
}

unsigned UdpClient::peek()
{
	if(sock == -1)
		bug("UdpClient closed");

#ifdef _WIN32
	u_long avail = 0;
	ioctlsocket(sock, FIONREAD, &avail);
#else
	int avail = 0;
	ioctl(sock, FIONREAD, &avail);
#endif // _WIN32

	if (avail < 0)
	{
		this->close();
		return 0;
	}

	return avail;
}

bool UdpClient::error() const
{
	return sock	== -1;
}

void UdpClient::close()
{
	if( sock != -1)
	{
#ifdef _WIN32
		::closesocket(sock);
#else
		::close(sock);
#endif // _WIN32
		sock = -1;
	}

	if (ai != NULL)
	{
		freeaddrinfo(ai);
		ai = NULL;
	}
}

}
