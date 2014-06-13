#define _WIN32_WINNT 0x0501

#include <iostream>
#include <boost/asio/ip/tcp.hpp>
#include "ConnectionManager.hpp"

// create the endpoint of connection
boost::asio::ip::tcp::endpoint createEndpoint( int argc, char* argv[] )
{
   std::string host( "127.0.0.1" );
   int port = 7171;

   if ( argc > 2 )
   {
      host = argv[ 1 ];
      port = atoi( argv[ 2 ] );
   }

   std::cout << "address: " << host << ":" << port << std::endl;
	return boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string( host ), 
                                          port );
}

int main( int argc, 
          char* argv[] )
{
   if ( argc != 3 )
   {
      std::cout << "USAGE: BoostServer <host> <port>" << std::endl;
      return 1;
   }

   // create the boost reactor
   boost::asio::io_service io_service;

   // create a connection manager on the host and port given in the argument
   ConnectionManager connectionManager( io_service,
                                        createEndpoint ( argc,
                                                         argv ) );

   // launch the boost reactor
   io_service.run();

   return 0;
}