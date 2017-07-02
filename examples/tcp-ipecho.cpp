/*
 *	the following example demonstrates a very basic tcp ip echo server using the xsocket library
 */

#include "../xsocket.hpp"
#include <iostream>

int main()	{

	//init only required for windows, no-op on *nix
	net::init();

	//create a tcp inetV4 socket and bind it to port 8080
	net::socket sock( net::af::inet, net::sock::stream, 8080 );

	//make sure socket creation and binding did not fail
	if( !sock.good() )	{
		std::cerr << "error creating socket" << std::endl;
		return -1;
	}

	//we must call listen before a call to accept
	sock.listen( 5 );

	std::cout << "listening on port: " << sock.getlocaladdr().get_port() << std::endl;

	net::socket client;
	net::endpoint remoteAddr;

	while( true )	{

		//accept() returns the fd which you can directly copy into a net::socket
		client = sock.accept( &remoteAddr );

		//verify that sock.accept did not fail
		if(client.good())	{
			std::string msg = remoteAddr.get_ip();
			client.send( &msg );
			client.close();
		}
	}

	return 0;
}
