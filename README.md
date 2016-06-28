# xsocket

xsocket is a header only library, simply include it to use it `#include <xsocket>`

## Platforms

* Linux ( tested on Arch Linux with 4.6.3-1-ARCH g++(6.1.1)/clang++(3.8.0) )
* Windows ( only tested on Windows 10 with msys2 and VisualC++ )

xsocket should work with most other platforms as well

## Installing

`sudo scons install` will install xsocket.hpp as /usr/include/xsocket

## Example

The following basic example shows how to create and bind a inet6 udp socket

```c++
#include <xsocket>

int main( int argc, char** argv )	{
	net::socket sock ( net::af::inet6, net::sock::dgram, 61270 );
	if( !sock.isValid() )
		return r;

	string msg;
	net::endpoint remote;

	sock.recvfrom( &msg, &remote );

	std::cout << "udp packet from: " << ep.asString() << " == " << msg << std::endl;

	return 0;
}
```
