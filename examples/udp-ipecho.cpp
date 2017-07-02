/*
 *	the following example demonstrates a very basic tcp ip echo server using the xsocket library
 */

#include "../xsocket.hpp"
#include <iostream>

int main()	{

	//init only required for windows, no-op on *nix
	net::init();

	//create a udp inetV4 socket and bind it to port 8080
	net::socket sock( net::af::inet, net::sock::dgram, 8080 );

	//make sure socket creation and binding did not fail
	if( !sock.good() )	{
		std::cerr << "error creating socket" << std::endl;
		return -1;
	}

	std::cout << "listening on port: " << sock.getlocaladdr().get_port() << std::endl;

	net::endpoint remoteAddr;
	std::string buf;

	while( true )	{

		//accept() returns the fd which you can directly copy into a net::socket
		int r = sock.recvfrom( &buf, 1024, &remoteAddr );

		if( r > 0 && buf == "ip\n" )	{
			std::string msg = remoteAddr.get_ip();
			sock.sendto( &msg, remoteAddr );
		}
	}

	return 0;
}
