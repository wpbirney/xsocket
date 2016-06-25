# xsocket

The following basic example shows how to create and bind a UDPSocket, TCPSocket works the same

```c++
#include <xsocket>

//for ipv4
using namespace net::ip;

//for ipv6
//using namespace net::ipv6;

int main( int argc, char** argv )	{
	UDPSocket sock ( 61270 );
	if( !sock.isValid() )
		return r;

	string msg;
	endpoint remote;

	sock.recvfrom( &msg, &remote );

	std::cout << "udp packet from: " << ep.asString() << " == " << msg << std::endl;

	return 0;
}
```
