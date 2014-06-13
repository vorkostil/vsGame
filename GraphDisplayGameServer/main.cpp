#define _WIN32_WINNT 0x0501

#include <boost/asio.hpp>
#include <iostream>

#include "../BackBoneServer/SimpleTcpConnection.hpp"
#include "connection/ConnectionToServer.hpp"

// create the endpoint of connection
boost::asio::ip::tcp::endpoint createEndpoint( int argc, char* argv[] )
{
   std::string host( "127.0.0.1" );
   int port = 8001;

   if ( argc > 2 )
   {
      host = argv[ 1 ];
      port = atoi( argv[ 2 ] );
   }

	return boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string( host ), 
                         port );
}

int main( int argc, char* argv[] )
{
	try
	{
      std::string clientName;
      std::cout << "login> ";
      std::cin >> clientName;

      // create the boost reactor
		boost::asio::io_service io_service;

      // create the connection to the server
      connection_ptr new_connection( new SimpleTcpConnection( io_service ) );
      ConnectionToServer::ConnectionToServerPtr session = ConnectionToServer::create( clientName,
                                                                                      new_connection );
      session->connect( createEndpoint( argc, 
                                        argv ) );

      // launc hthe async reactor
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "An Exception occurs: " << e.what() << std::endl;
	}
			
	return 0;
}
