#include "xsocket.hpp"

#include <iostream>

using namespace net;


int main()	{
	ip::TCPSocket sock( 8080 );

	if( !sock.isValid() )	{
		std::cout << "error creating socket" << std::endl;
		return -1;
	}

	sock.listen( 5 );

	std::cout << "listening on port: " << sock.getlocaladdr().getPort() << std::endl;

	ip::TCPSocket client;
	ip::endpoint remoteAddr;

	while( true )	{
		client = sock.accept( &remoteAddr );
		if(client.isValid())	{
			std::string msg = remoteAddr.asString();
			client.send( &msg );
			client.close();
		}
	}

	return 0;
}
