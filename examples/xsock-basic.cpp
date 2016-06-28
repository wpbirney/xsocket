#include "xsocket.hpp"
#include <iostream>

int main()
{
	net::socket v6s ( net::af::inet6, net::sock::dgram, 4444 );

	std::cout << "listening at: " << v6s.getlocaladdr().asString() << std::endl;
	/*
	addrinfo info;
	info.ai_family = AF_UNSPEC;
	info.ai_socktype = SOCK_DGRAM;
	info.ai_flags = 0;
	info.ai_protocol = 0;

	addrinfo *res, *rp;

	getaddrinfo("192.168.1.5", NULL, &info, &res);

	for(rp = res; rp != NULL; rp = rp->ai_next)	{
		std::vector<char> buffer ( INET6_ADDRSTRLEN );
		getnameinfo(rp->ai_addr, rp->ai_addrlen, buffer.data(), buffer.size(), NULL, 0, 0);
		std::cout << std::string( buffer.begin(), buffer.end() ) << std::endl;
	}
	return -1;
	*/
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
