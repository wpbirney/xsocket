#include "xsocket.hpp"

#include <iostream>


int main()	{
	net::socket sock( net::af::inet, net::sock::stream, 8080 );

	if( !sock.isValid() )	{
		std::cout << "error creating socket" << std::endl;
		return -1;
	}

	sock.listen( 5 );

	std::cout << "listening on port: " << sock.getlocaladdr().getPort() << std::endl;

	net::socket client;
	net::endpoint remoteAddr;

	while( true )	{
		client = sock.accept( &remoteAddr );
		if(client.isValid())	{
			std::stringstream ss;
			ss << "HTTP/1.1 200 OK" << std::endl
			   << "Content-Type: text/html"
			   << std::endl
			   << std::endl
			   << remoteAddr.asString()
			   << std::endl;
			std::string msg = ss.str();
			client.send( &msg );
			client.close();
		}
	}

	return 0;
}
