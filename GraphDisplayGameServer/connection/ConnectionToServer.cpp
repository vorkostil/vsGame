#define _WIN32_WINNT 0x0501

#include "ConnectionToServer.hpp"
#include <boost/bind.hpp>

ConnectionToServer::ConnectionToServer( const std::string& name,
                                        connection_ptr connection )
:
   name( name ),
   connection( connection ),
   message()
{
	std::cout << "ConnectionToServer> New client conncection created> " << name << std::endl;
}

ConnectionToServer::~ConnectionToServer() 
{ 
	std::cout << "ConnectionToServer> Session close> " << name << std::endl;
}

const std::string& ConnectionToServer::getName() const
{
   return name;
}

void ConnectionToServer::close()
{
   connection->close();
}

void ConnectionToServer::connect( boost::asio::ip::tcp::endpoint& endpoint )
{
		connection->getSocket().async_connect( endpoint,
			                                    boost::bind( &ConnectionToServer::handleConnect, 
                                                          this,
                                                          connection,
                                                          boost::asio::placeholders::error ) );
}

void ConnectionToServer::waitForData()
{
	// Call the async listen using the connection
	connection->asyncRead( message, 
		                    boost::bind( &ConnectionToServer::handleRead, 
                                       shared_from_this(),
		                                 boost::asio::placeholders::error ) );
}

void ConnectionToServer::sendMessage(const std::string& message)
{
   // send the message on the network
   connection->asyncWrite( message,
		                     boost::bind( &ConnectionToServer::handleWrite, 
                                        shared_from_this(),
		                                  boost::asio::placeholders::error ) );
}

void ConnectionToServer::handleRead( const boost::system::error_code& error )
{
	if ( error == 0)
	{
		// manage the message
      std::cout << "ConnectionToServer> " << "Message received> " << message << std::endl;

		// and back to listen
		waitForData();
	}
	else
	{
      std::cout << "ConnectionToServer> " << name << "> handleRead call with error code: " << error.value() << " --> " << error.message() << std::endl;
	}
}

void ConnectionToServer::handleWrite( const boost::system::error_code& error )
{
   // if an error occurs, close the connection
	if ( error != 0 )
	{
      std::cout << "ConnectionToServer> " << name << "> handleWrite call with error code: " << error.value() << " --> " << error.message() << std::endl;
	}
}

void ConnectionToServer::handleConnect( connection_ptr new_connection, 
                                        const boost::system::error_code& error )
{
   std::cout << "ConnectionToServer> " << "Connection callback" << std::endl;

   // check the error status
	if ( error == 0)
	{
      // send the clientName on the socket
      sendMessage( name );

      // call the async reading
		waitForData();
	}
   else
   {
      std::cout << "ConnectionToServer> " << name << "> handleConnect call with error code: " << error.value() << " --> " << error.message() << std::endl;
   }
}
