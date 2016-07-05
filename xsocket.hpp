/*
Copyright (c) 2016 Wilson Birney <wpbirney@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef XSOCKET_HPP
#define XSOCKET_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <cstring> //memset
#include <functional>
#include <vector>

#define XSOCK_VERSION	0x00

//Defaults all sockets to nonblocking
//#define XS_NONBLOCKING

//cross platform includes
#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#else
//#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
static WSAData _wsaData;
static bool _wsaInitDone = false;
inline void _initWinsock()	{
	WSAStartup(MAKEWORD(2,2), &_wsaData);
	_wsaInitDone = true;
}
#define SHUT_RD SD_RECEIVE
#define SHUT_WR	SD_SEND
#define SHUT_RDWR SD_BOTH
#endif


namespace net {

inline void init()	{
//no-op on *nix
#ifdef _WIN32
	if( !_wsaInitDone )
		_initWinsock();
#endif
}

enum class af	{
	inet = AF_INET,
	inet6 = AF_INET6,
	unspec = AF_UNSPEC
};

enum class sock	{
	stream = SOCK_STREAM,
	dgram = SOCK_DGRAM
};

enum class shut	{
	rd = SHUT_RD,
	wr = SHUT_WR,
	rdwr = SHUT_RDWR
};

/*
 *	generic endpoint class for uniform access to ipv4 and ipv6
 */

struct endpoint {
	endpoint()	{
		memset( &addr, 0, sizeof( sockaddr_storage ) );
		addrlen = sizeof( sockaddr_storage );
	}

	endpoint( int port )				{
		set( "0", port );
	}
	endpoint( std::string ip, int port )		{
		set( ip, port );
	}
	endpoint( std::string ip, int port, af fam )	{
		set( ip, port, fam );
	}

	bool operator== ( endpoint& e )	{
		if( getIP() == e.getIP() && getPort() == e.getPort() )
			return true;
		return false;
	}

	bool operator!= ( endpoint& e )	{
		return !operator==( e );
	}

	//returns a vector of all possible endpoints for host:port for the specified sock_type and address family
	static std::vector<endpoint> getEndpoints( const char* host, const char* service, af f=af::unspec )	{
		addrinfo hints;
		memset( &hints, 0, sizeof( addrinfo ) );
		hints.ai_family = (int)f;
		hints.ai_socktype = 0;

		addrinfo *res, *rp;

		if( host == nullptr )
			hints.ai_flags = AI_PASSIVE;

		int i = getaddrinfo( host, service, &hints, &res );

		std::vector<endpoint> buffer;

		if( i != 0 )	{
			std::cerr << "[xsocket]: " << gai_strerror( i ) << std::endl;
			return buffer;
		}

		for( rp = res; rp != nullptr; rp = rp->ai_next )	{
			endpoint ep;
			memcpy( &ep.addr, rp->ai_addr, rp->ai_addrlen );
			ep.addrlen = rp->ai_addrlen;
			ep.addrfam = (af)rp->ai_family;

			bool found = false;
			for( endpoint &e : buffer )	{
				if( e == ep )	{
					found = true;
					break;
				}
			}

			if( !found )
				buffer.push_back( ep );
		}

		freeaddrinfo( res );

		return buffer;
	}

	void set( std::string ip, int port, af f=af::unspec )	{
		const char *host = ip.c_str();
		if( ip == "0" )
			host = nullptr;
		std::vector<endpoint> epList = endpoint::getEndpoints( host, std::to_string(port).c_str(), f );
		*this = epList[0];
	}

	std::string getIP()	{
		std::vector<char> buf( INET6_ADDRSTRLEN );
		getnameinfo( (sockaddr*)&addr, getDataSize(), buf.data(), buf.size(), nullptr, 0, NI_NUMERICHOST );
		return std::string( buf.begin(), buf.end() );
	}
	int getPort()		{
		std::vector<char> buf( INET6_ADDRSTRLEN );
		getnameinfo( (sockaddr*)&addr, getDataSize(), nullptr, 0, buf.data(), buf.size(), NI_NUMERICSERV );
		return std::atoi( std::string(buf.begin(), buf.end()).c_str() );
	}

	af getAF()	{
		return addrfam;
	}

	sockaddr* getData()	{
		return (sockaddr*)&addr;
	}
	int getDataSize()	{
		return addrlen;
	}

	std::string asString()	{
		std::stringstream ss;
		ss << getIP() << ":" << getPort();
		return ss.str();
	}

	sockaddr_storage addr;
	socklen_t addrlen;
	af addrfam;
};

typedef std::vector<endpoint> endpointList;

// getname calls getsockname/getpeername and returns it as an endpoint type
inline endpoint getname(int fd, std::function<int(int,sockaddr*,socklen_t*)> target) {
	endpoint ep;
	socklen_t al = ep.getDataSize();
	target(fd, ep.getData(), &al);
	ep.addrlen = al;
	return ep;
}

/*
 *	base socket class
 */

struct socket {
	socket()	{
		fd = -1;
	}

	socket( af fam, sock socktype )	{
		init( fam, socktype );
	}

	socket( af fam, sock socktype, int port ) : socket( fam, socktype )	{
		int r = bind( port );
		if( r == -1 )	{
			close();
			fd = -1;
		}
	}

	socket& operator =( int newfd )	{
		fd = newfd;
		return *this;
	}

	int init( af fam, sock socktype )	{
		fd = ::socket( (int)fam, (int)socktype, 0);
		addrfam = fam;
#ifdef XS_NONBLOCKING
		setnonblocking( true );
#endif
		return fd;
	}

	int accept( endpoint* ep )	{
		socklen_t al = ep->getDataSize();
		return ::accept( fd, ep->getData(), &al );
	}

	int listen( int n )	{
		return ::listen( fd, n );
	}

	int bind( endpoint ep )	{
		return ::bind( fd, ep.getData(), ep.getDataSize() );
	}

	int bind( std::string addr, int port )	{
		endpoint ep( addr, port, addrfam );
		return bind( ep );
	}

	int bind( int port )	{
		return bind( "0", port );
	}

	int connect( endpoint ep )	{
		return ::connect( fd, ep.getData(), ep.getDataSize() );
	}

	int sendto( char* data, int len, endpoint ep )	{
		return ::sendto( fd, data, len, 0, ep.getData(), ep.getDataSize() );
	}

	int sendto( std::string* data, endpoint ep )	{
		return sendto( (char*)data->c_str(), data->size(), ep );
	}

	int recvfrom( char* buf, int len, endpoint* ep )	{
		socklen_t al = ep->getDataSize();
		return ::recvfrom( fd, buf, len, 0, ep->getData(), &al );
	}

	int recvfrom( std::string* buf, int len, endpoint* ep )	{
		std::vector<char> buffer( len );
		int r = recvfrom( buffer.data(), buffer.size(), ep );
		if( r > 0 )
			*buf = std::string( buffer.data(), r );
		return r;
	}

	int send( char* data, int len )	{
		return ::send( fd, data, len, 0 );
	}

	int send( std::string* data )	{
		return send( (char*)data->c_str(), data->size() );
	}

	int recv( char* buf, int len )	{
		return ::recv( fd, buf, len, 0 );
	}

	int recv( std::string* buf, int len )	{
		std::vector<char> buffer( len );
		int r = recv( buffer.data(), buffer.size() );
		if( r > 0 )
			*buf = std::string( buffer.data(), r );
		return r;
	}

	int close()	{
#ifndef _WIN32
		return ::close( fd );
#else
		return ::closesocket( fd );
#endif
	}

	int shutdown( shut how )	{
		return ::shutdown( fd, (int)how );
	}

	int setnonblocking( bool block )	{
#ifndef _WIN32
		return fcntl( fd, F_SETFL, O_NONBLOCK, block );
#else
		DWORD nb = (int)block;
		return ioctlsocket( fd,  FIONBIO,  &nb );
#endif
	}

	// TODO: implement the timeout for windows
	int settimeout( int sec, int us )	{
#ifndef _WIN32
		timeval tv;
		tv.tv_sec = sec;
		tv.tv_usec = us;
		int i = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		return i;
#else
		return -1;
#endif
	}

	endpoint getlocaladdr()	{
		return getname(fd, getsockname);
	}

	endpoint getremoteaddr()	{
		return getname(fd, getpeername);
	}

	int getError()	{
		return errno;
	}

	bool isValid()	{
		if( fd != -1 )
			return true;
		return false;
	}

  private:
	int fd;
	af addrfam;
};

} //namespace net

#endif // XSOCKET_HPP
