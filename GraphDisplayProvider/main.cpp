#define _WIN32_WINNT 0x0501

#include <boost/asio.hpp>
#include <iostream>

#include "network/SimpleTcpConnection.hpp"
#include "connection/ConnectionToServer.hpp"
#include "ProviderManager.hpp"

int main( int argc, char* argv[] )
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
   ProviderManager server( ConnectionToServer::create( "GraphDisplayProvider",
                                                       new_connection ) );
   server.connect( argv[ 1 ],
                   atoi( argv[ 2 ] ) );

   // launc hthe async reactor
	io_service.run();
			
	return 0;
}
