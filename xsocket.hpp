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
#else
	#include <winsock2.h>
	typedef int socklen_t;
	static WSAData _wsaData;
	static bool _wsaInitDone = false;
	inline void _initWinsock()	{
		WSAStartup(MAKEWORD(2,2), &_wsaData);
		_wsaInitDone = true;
	}
#endif


namespace net
{

enum class af	{
	inet = AF_INET,
	inet6 = AF_INET6
};

enum class sock	{
	stream = SOCK_STREAM,
	dgram = SOCK_DGRAM
};

/*
 *	sa_* functions for sockaddr_* interactions
 */

void sa_set( sockaddr_in* addr, std::string ip, int port )	{
	addr->sin_family = AF_INET;
	if( ip == "0" || ip == "0.0.0.0" )
		addr->sin_addr.s_addr = INADDR_ANY;
	else
		inet_pton( AF_INET,  ip.c_str(), &addr->sin_addr );
	addr->sin_port = htons( port );
}

void sa_set( sockaddr_in6* addr, std::string ip, int port )	{
	addr->sin6_family = AF_INET6;
	if( ip == "0" || ip == "0.0.0.0" )
		addr->sin6_addr = in6addr_any;
	else
		inet_pton( AF_INET6,  ip.c_str(), &addr->sin6_addr );
	addr->sin6_port = htons( port );
}

int sa_getPort( sockaddr_in* addr )	{ return ntohs( addr->sin_port ); }
int sa_getPort( sockaddr_in6* addr )	{ return ntohs( addr->sin6_port ); }

std::string sa_getIP( sockaddr_in* addr )	{
	std::vector<char> buf( INET_ADDRSTRLEN );
	inet_ntop( AF_INET, &addr->sin_addr, buf.data(), INET_ADDRSTRLEN );
	return std::string( buf.begin(), buf.end() );
}

std::string sa_getIP( sockaddr_in6* addr )	{
	std::vector<char> buf( INET6_ADDRSTRLEN );
	inet_ntop( AF_INET6, &addr->sin6_addr, buf.data(), INET6_ADDRSTRLEN );
	return std::string( buf.begin(), buf.end() );
}

/*
 *	generic endpoint template class for uniform access to ipv4 and ipv6
 */

template<af f, typename T>
struct endpoint
{
	endpoint()				{ memset( &addr, 0, sizeof( T ) ); }
	endpoint( int port )			{ sa_set( &addr, "0", port ); }
	endpoint( std::string ip, int port )	{ sa_set( &addr, ip, port ); }

	bool operator== ( T& e )	{
		if( memcmp( &addr, &e, sizeof( T ) ) == 0 )
			return true;
		return false;
	}

	bool operator!= ( T& e )	{ return !operator==( e ); }

	void set( std::string ip, int port )	{ sa_set( &addr, ip, port ); }
	std::string getIP()	{ return sa_getIP( &addr ); }
	int getPort()		{ return sa_getPort( &addr ); }
	static af getAF()	{ return f; }

	const T* getData()	{ return &addr; }
	int getDataSize()	{ return sizeof( T ); }

	std::string asString()	{
		std::stringstream ss;
		ss << getIP() << ":" << getPort();
		return ss.str();
	}

	T addr;
};

// getname calls getsockname/getpeername and returns it as an endpoint type
template<typename T>
T getname(int fd, std::function<int(int,sockaddr*,socklen_t*)> target)
{
	T ep;
	socklen_t al = ep.getDataSize();
	int i = target(fd, (sockaddr*)ep.getData(), &al);
	return ep;
}

/*
 *	base socket template class
 */

template< sock s, typename eptype >
struct socket
{
	socket()	{
		#ifdef _WIN32
			if(!_wsaInitDone)
				_initWinsock();
		#endif
		fd = ::socket( (int)eptype::getAF(), (int)s, 0);

		#ifdef XS_NONBLOCKING
			setnonblocking( true );
		#endif
	}

	socket( int port ) : socket()	{
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

	int accept( eptype* ep )	{
		socklen_t al = ep->getDataSize();
		return ::accept( fd, (sockaddr*)ep->getData(), &al );
	}

	int listen( int n )	{
		return ::listen( fd, n );
	}

	int bind( eptype ep )	{
		return ::bind( fd, (sockaddr*)ep.getData(), ep.getDataSize() );
	}

	int bind( int port )	{
		return bind( eptype( "0", port ) );
	}

	int connect( eptype ep )	{
		return ::connect( fd, (sockaddr*)ep.getData(), ep.getDataSize() );
	}

	int sendto( char* data, int len, eptype ep )	{
		return ::sendto( fd, data, len, 0, (sockaddr*)ep.getData(), ep.getDataSize() );
	}

	int sendto( std::string* data, eptype ep )	{
		return sendto( (char*)data->c_str(), data->size(), ep );
	}

	int recvfrom( char* buf, int len, eptype* ep )	{
		socklen_t al = ep->getDataSize();
		return ::recvfrom( fd, buf, len, 0, (sockaddr*)ep->getData(), &al );
	}

	int recvfrom( std::string* buf, int len, eptype* ep )	{
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

	eptype getlocaladdr()	{
		return getname<eptype>(fd, getsockname);
	}

	eptype getremoteaddr()	{
		return getname<eptype>(fd, getpeername);
	}

	int getError()	{ return errno; }

	bool isValid()	{
		if( fd != -1 )
			return true;
		return false;
	}

private:
	int fd;
};

namespace ip
{
typedef net::endpoint<af::inet, sockaddr_in>	endpoint;
typedef socket<net::sock::dgram, endpoint>	UDPSocket;
typedef socket<net::sock::stream, endpoint>	TCPSocket;
}

namespace ipv6
{
typedef net::endpoint<af::inet6, sockaddr_in6>	endpoint;
typedef socket<net::sock::dgram, endpoint>	UDPSocket;
typedef socket<net::sock::stream, endpoint>	TCPSocket;
}

} //namespace net

#endif // XSOCKET_HPP
