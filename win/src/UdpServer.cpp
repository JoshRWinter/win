#include <win/Win.hpp>

#ifdef WINPLAT_LINUX
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#else
#include <Ws2tcpip.h>
#endif

#include <win/WsaSingleton.hpp>
#include <win/UdpServer.hpp>

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

UdpServer::UdpServer()
{
	initialize_wsa();

	sock = -1;
}

UdpServer::UdpServer(unsigned short port)
{
	sock = -1;
	bind(port);
}

UdpServer::UdpServer(UdpServer &&rhs) noexcept
{
	sock = rhs.sock;
	rhs.sock = -1;
}

UdpServer::~UdpServer()
{
	this->close();
}

UdpServer &UdpServer::operator=(UdpServer &&rhs) noexcept
{
	close();

	sock = rhs.sock;
	rhs.sock = -1;

	return *this;
}

UdpServer::operator bool() const
{
	return !error();
}

// cleanup
void UdpServer::close()
{
	if (sock != -1)
	{
#ifdef _WIN32
		::closesocket(sock);
#else
		::close(sock);
#endif // _WIN32
		sock = -1;
	}
}

// non blocking send
void UdpServer::send(const void *buffer, int len, const UdpId &id)
{
	if (sock == -1)
		bug("UdpServer closed");

	if (!id.initialized)
		bug("UdpId not initialized");

	// no such thing as partial sends for sendto with udp
	const int result = sendto(sock, (const char*)buffer, len, 0, (sockaddr*)&id.storage, id.len);
	if (result != len && get_errno() != WOULDBLOCK)
	{
		this->close();
		return;
	}
}

// non blocking recv
int UdpServer::recv(void *buffer, int len, UdpId &id)
{
	if(sock == -1)
		bug("UdpServer closed");

	// no partial receives
	const int result = recvfrom(sock, (char*)buffer, len, 0, (sockaddr*)&id.storage, &id.len);
	if(result == -1)
	{
		const auto eno = get_errno();
		if(eno == WOULDBLOCK || eno == CONNRESET)
			return 0;
		else
			this->close();

		return 0;
	}

	id.initialized = true;

	return result;
}

// how many bytes are available on the socket
unsigned UdpServer::peek()
{
	if (sock == -1)
		bug("UdpServer closed");

#ifdef _WIN32
	u_long available;
	ioctlsocket(sock,FIONREAD,&available);
#else
	int available=0;
	ioctl(sock, FIONREAD, &available);
#endif // _WIN32

	if (available < 0)
	{
		this->close();
		return 0;
	}

	return available;
}

// error check
bool UdpServer::error() const
{
	return sock == -1;
}

bool UdpServer::bind(unsigned short port)
{
	addrinfo hints, *ai;

	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	// convert port to string
	char port_string[20];
	sprintf(port_string, "%hu", port);

	// resolve hostname
	if (getaddrinfo("::", port_string, &hints, &ai) != 0)
	{
		this->close();
		return false;
	}

	// create the socket
	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if(sock == -1)
	{
		this->close();
		return false;
	}

#ifdef _WIN32
	// make this a dual stack socket
	u_long optmode = 0;
	setsockopt(sock, IPPROTO_IPV6, 27, (char*)&optmode, 40);
#else
	// make this socket reusable
	int reuse = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
#endif // _WIN32

	// set to non blocking
#ifdef _WIN32
	u_long nonblock = 1;
	ioctlsocket(sock, FIONBIO, &nonblock);
#else
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK); // set to non blocking
#endif // _WIN32

	if (::bind(sock, ai->ai_addr, ai->ai_addrlen))
	{
		this->close();
		return false;
	}

	freeaddrinfo(ai);

	return true;
}

}
