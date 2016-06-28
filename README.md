# xsocket

The following basic example shows how to create and bind a UDPSocket, TCPSocket works the same

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
