#define _WIN32_WINNT 0x0501

#include <boost/asio.hpp>
#include <iostream>

#include "network/SimpleTcpConnection.hpp"
#include "connection/ConnectionToServer.hpp"
#include "graph/GraphGridProvider.hpp"

int main( int argc, char* argv[] )
{
	try
	{
      if ( argc != 3 )
      {
         std::cout << "USAGE: GraphDisplayProvider <host> <port>" << std::endl;
         return 1;
      }

      // create the boost reactor
		boost::asio::io_service io_service;

      // create the connection to the server
      connection_ptr new_connection( new SimpleTcpConnection( io_service ) );
      GraphGridProvider server( ConnectionToServer::create( "GraphGridProvider",
                                                            new_connection ) );
      server.createGraph( 32, 32 );
      server.connect( argv[ 1 ],
                      atoi( argv[ 2 ] ) );

      // launc hthe async reactor
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "An Exception occurs: " << e.what() << std::endl;
	}
			
	return 0;
}
