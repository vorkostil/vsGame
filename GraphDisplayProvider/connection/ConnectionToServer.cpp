#define _WIN32_WINNT 0x0501

#include "ConnectionToServer.hpp"
#include <boost/bind.hpp>
#include "network/NetworkMessage.hpp"
#include "network/NetworkClient.hpp"

ConnectionToServer::ConnectionToServer( const std::string& name,
                                        connection_ptr connection )
:
   name( name ),
   connection( connection ),
   message(),
   client( NULL ),
   status( INIT )
{
	std::cout << "ConnectionToServer> New client conncection created> " << name << std::endl;
}

ConnectionToServer::~ConnectionToServer() 
{ 
	std::cout << "ConnectionToServer> Session close> " << name << std::endl;
}

// set the client user of this connection
void ConnectionToServer::setNetworkClient( NetworkClient* client )
{
   this->client = client;
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
#ifdef __DEBUG__
      // log the message if needed
      std::cout << "ConnectionToServer> " << "Message received> " << message << std::endl;
#endif

      // check if its the init process
      if (  ( status == INIT )
          &&( message == MESSAGE_LOGIN_ASKED )  )
      {
         status = LOGIN;
         sendMessage( client->getLogin() + ":" + client->getPassword() );
      }
      else if ( status == LOGIN )
      {
         if ( message == MESSAGE_LOGIN_ACCEPTED )
         {
            status = CONNECTED;
            client->onLoginSucced();
         }
         else if ( message == MESSAGE_LOGIN_REFUSED )
         {
            status = INIT;
         }
      }
      else if ( status == CONNECTED )
      {
         // forward the message to the client
         client->onHandleMessage( message );
      }

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
      // send the init message
      sendMessage( MESSAGE_INIT );

      // call the async reading
		waitForData();
	}
   else
   {
      std::cout << "ConnectionToServer> " << name << "> handleConnect call with error code: " << error.value() << " --> " << error.message() << std::endl;
   }
}
