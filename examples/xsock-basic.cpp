#include "xsocket.hpp"
#include <iostream>

int main()
{
	net::init();

	std::vector<net::endpoint> eps = net::endpoint::getEndpoints("www.google.com", "80", net::af::inet6);
	for( net::endpoint &i : eps )	{
		std::cout << i.asString() << std::endl;
	}

	net::socket v6s ( net::af::inet6, net::sock::dgram, 4444 );

	std::cout << "listening at: " << v6s.getlocaladdr().asString() << std::endl;

	std::string buff;
	net::endpoint pp;
	v6s.recvfrom( &buff, 512, &pp );
	std::cout << buff << std::endl;
	std::cout << pp.asString() << std::endl;

	net::socket sock(net::af::inet, net::sock::dgram);
	int r = sock.bind( 4420 );
	if(r == -1)
		perror("failed to bind");

	std::cout << "socket bound to: " << sock.getlocaladdr().asString() << std::endl;

	std::string buf;
	net::endpoint ep;

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
