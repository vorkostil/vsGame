#define _WIN32_WINNT 0x0501

#include <stdio.h>
#include "ConnectionManager.hpp"
#include "ClientConnection.hpp"
#include <boost/bind.hpp>

std::string createNewClientName()
{
   static int id = 0;
   char result[20];
   sprintf_s( result,
              20,
              "client_%d",
              id++ );
   return result;
}

ConnectionManager::ConnectionManager( boost::asio::io_service&              boostReactor, 
                                      const boost::asio::ip::tcp::endpoint& endpoint )
:
   boostReactor( boostReactor ),
   connectionAcceptor( boostReactor, 
                       endpoint ),
   connections()
{
   // waiting for the connection
	waitForConnection();
}

void ConnectionManager::waitForConnection()
{
	connection_ptr new_connection(new SimpleTcpConnection( boostReactor ) );

	// Attente d'une nouvelle connection
	connectionAcceptor.async_accept( new_connection->getSocket(),
		                              boost::bind( &ConnectionManager::handle_accept, 
                                                 this,
		                                           boost::asio::placeholders::error,
                                                 new_connection ) );
}

void ConnectionManager::handle_accept( const boost::system::error_code& error, 
                                       connection_ptr new_connection )
{
	if ( error == 0)
	{
      // create the unique id
      std::string newClientName = createNewClientName();
		std::cout << "ConnectionManager> " << "Connection accepted> " << newClientName << std::endl;

      // create the client
		ClientConnection::ClientConnectionPtr client = ClientConnection::create( newClientName, 
                                                                               this,
                                                                               new_connection );

      // and store the information
      connections.insert( client );

      // and back to client acceptance
		waitForConnection();
	}
	else 
   {
		std::cerr << "ConnectionManager> " << "Connection refused" << std::endl;
	}
}

void ConnectionManager::handleMessage( ClientConnection::ClientConnectionPtr connection,
                                       const std::string& message )
{
   // log the message
   std::cout << connection->getLogin() << "> " << message << std::endl;

   // close the connection to EC
   connection->sendMessage( connection->getLogin() + "> " + message );
}

void ConnectionManager::closeConnection( ClientConnection::ClientConnectionPtr connection )
{
   connections.erase( connection );
}
