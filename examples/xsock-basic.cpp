#include "../xsocket.hpp"
#include <iostream>

/*
 *	this is a crude example of the basics, it really needs improved
 */

int main()
{

	//we must call net::init() on windows, if not on windows it is a no-op
	net::init();

	//get a vector of possible endpoints for google.com:80, af::unspec allows both ipv4 and ipv6
	auto eps = net::endpoint::getEndpoints("www.google.com", "80", net::af::unspec);

	std::cout << "the following endpoints were returned for www.google.com:80" << std::endl;
	for( net::endpoint &i : eps )	{
		std::cout << i.asString() << std::endl;
	}

	std::cout << std::endl << std::endl;

	//create an ipv6 udp socket, we can optionaly specify the port to bind to as the 3rd arg
	net::socket v6s ( net::af::inet6, net::sock::dgram, 4444 );
	if( !v6s.isValid() )	{
		std::cerr << "failed to create & bind ipv6 socket" << std::endl;
		return -1;
	}

	std::cout << "ipv6 socket created..." << std::endl;

	//we can access a sockets local endpoint by getlocaladdr() && getremoteaddr() for tcp peers
	std::cout << "listening at: " << v6s.getlocaladdr().asString() << std::endl
		  << "send a udp packet to :: " << v6s.getlocaladdr().getPort() << " to continue" << std::endl;

	//we can recv directly into a std::string or a char* buffer
	std::string buff;
	net::endpoint ep;

	//recv a packet up to 512 bytes and store the sender in endpoint ep
	v6s.recvfrom( &buff, 512, &ep );
	std::cout << buff << std::endl;
	std::cout << ep.asString() << std::endl;
	v6s.close();

	//create a socket without binding in the ctor
	net::socket sock(net::af::inet, net::sock::dgram);

	//most calls in xsocket return the same value as there c counterparts
	//like so if sock.bind returns -1 it failed
	int r = sock.bind( 4420 );
	if(r == -1)	{
		perror("failed to bind");
		return -1;
	}

	std::cout << "socket bound to: " << sock.getlocaladdr().asString() << std::endl;

	while(true)	{
		int i = sock.recvfrom(&buff, 512, &ep);
		if(i == 4 || i == -1)
			break;
		std::cout << "packet from: " << ep.asString() << std::endl
			  << "DATA START" << std::endl << buff << std::endl
			  << "DATA END" << std::endl;
	}

	return 0;
}
