#include "xsocket.hpp"

#include <iostream>

using namespace net::ip;

int main()
{
	UDPSocket sock;
	int r = sock.bind( 4420 );
	if(r == -1)
		perror("failed to bind");

	std::cout << "socket bound to: " << sock.getlocaladdr().asString() << std::endl;

	std::string buf;
	endpoint ep;

	while(true)	{
		int i = sock.recvfrom(&buf, 512, &ep);
		if(i == 4 || i == -1)
			break;
		std::cout << "packet from: " << ep.asString() << std::endl
			  << "DATA START" << std::endl << buf << std::endl
			  << "DATA END" << std::endl;
	}

	return 0;
}
