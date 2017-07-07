#include "../xsocket.hpp"

void usage()	{
	std::cout << "usage: resolve [ip or hostname]" << std::endl;
}

int main( int argc, char** argv )	{
	net::init();

	if( argc != 2 )	{
		usage();
		return -1;
	}

	try {
		auto eplist = net::endpoint::resolve(argv[1], "420", net::af::unspec);

		for( net::endpoint &ep : eplist )	{
			std::cout << ep.to_string() << std::endl;
		}
	} catch ( std::runtime_error e )	{
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
