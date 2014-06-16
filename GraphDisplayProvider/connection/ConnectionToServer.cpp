#define _WIN32_WINNT 0x0501

#include "ConnectionToServer.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "network/NetworkMessage.hpp"
#include "network/NetworkClient.hpp"
#include "string/StringUtils.hpp"

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
   std::cout << "ConnectionToServer (" << name << ") writing: " << message << std::endl;

   // send the message on the network
   connection->asyncWrite( message + '\0',
		                     boost::bind( &ConnectionToServer::handleWrite, 
                                        shared_from_this(),
		                                  boost::asio::placeholders::error ) );
}

void ConnectionToServer::handleRead( const boost::system::error_code& error )
{
	if ( error == 0)
	{
      // invoke a thread to handle the message
      boost::thread worker( &ConnectionToServer::handleMessageInThread,
                            this,
                            message );

		// and back to listen
		waitForData();
	}
	else
	{
      std::cout << "ConnectionToServer> " << name << "> handleRead call with error code: " << error.value() << " --> " << error.message() << std::endl;
	}
}

// thread used to decipher and manage the message
void ConnectionToServer::handleMessageInThread( const std::string& messageToTreat )
{
   // log the message if needed
   std::cout << "ConnectionToServer> " << "Message received> " << message << std::endl;

   // check if its the init process
   if (  ( status == INIT )
       &&( messageToTreat == MESSAGE_LOGIN_ASKED )  )
   {
      // manage the login message
      status = LOGIN;
      sendMessage( client->getLogin() + ":" + client->getPassword() );
   }
   // waiting for the login response
   else if ( status == LOGIN )
   {
      // check the status
      if ( messageToTreat == MESSAGE_LOGIN_ACCEPTED )
      {
         // forward the success to the client
         status = CONNECTED;
         client->onLoginSucced();
      }
      else if ( messageToTreat == MESSAGE_LOGIN_REFUSED )
      {
         // back to the INIT state
         status = INIT;
      }
   }
   // forward the day to day message
   else if ( status == CONNECTED )
   {
      // explode the message to get the < kind, [ action | gameId ], remaining message >
      std::vector< std::string > messagePart;
      StringUtils::explode( messageToTreat,
                            ' ',
                            messagePart,
                            3 );
      // check for game message
      if ( messagePart[ 0 ] == GAME_MESSAGE )
      {
         // game creation
         if ( messagePart[ 1 ] == GAME_CREATED )
         {
            client->onNewGameCreation( messagePart[ 2 ] );
         }
         // game destruction
         else if ( messagePart[ 1 ] == CLOSE_MESSAGE )
         {
            // retrieve the message informatio
            std::vector< std::string > messageInformation;
            StringUtils::explode( messagePart[ 2 ],
                                    ' ',
                                    messageInformation,
                                    2 );

            // and close the game
            client->onGameClose( messageInformation[ 0 ],
                                 messageInformation[ 1 ] );
         }
         // forward the message to the client
         else 
         {
            client->onHandleMessage( messagePart[ 1 ], 
                                     messagePart[ 2 ] );
         }
      }
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
      // alert the client
      client->onConnection();

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

// return the localendpoint as string host:port
std::string ConnectionToServer::getLocalEndPointAsString() const
{
   char result[ 1024 ];
   sprintf_s( result,
              1024,
              "%s@%d",
              connection->getSocket().local_endpoint().address().to_string().c_str(),
              connection->getSocket().local_endpoint().port() );

   return std::string( result );
}

