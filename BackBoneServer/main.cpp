#define _WIN32_WINNT 0x0501

#include <iostream>
#include <boost/asio/ip/tcp.hpp>
#include "ConnectionManager.hpp"

int main( int argc, 
          char* argv[] )
{
   if ( argc != 3 )
   {
      std::cout << "USAGE: BackBoneServer <host> <port>" << std::endl;
      return 1;
   }

   // create the boost reactor
   boost::asio::io_service io_service;

   // create a connection manager on the host and port given in the argument
   ConnectionManager connectionManager( io_service,
                                        boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string( argv[ 1 ] ),
                                                                        atoi( argv[ 2 ] ) ) );

   // launch the boost reactor
   io_service.run();

   return 0;
}