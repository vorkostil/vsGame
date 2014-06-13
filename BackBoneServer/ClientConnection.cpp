#define _WIN32_WINNT 0x0501

#include "ClientConnection.hpp"
#include "ConnectionManager.hpp"
#include <boost/bind.hpp>

#define MESSAGE_INIT "INIT"
#define MESSAGE_SYSTEM "SYSTEM"
#define MESSAGE_LOGIN "LOGIN"
#define MESSAGE_ASKED "ASKED"
#define MESSAGE_ACCEPTED "ACCEPTED"
#define MESSAGE_REFUSED "REFUSED"

ClientConnection::ClientConnection( const std::string& technicalId,
                                    ConnectionManager* connectionManager,
                                    connection_ptr connection )
:
   technicalId( technicalId ),
   connectionManager( connectionManager ),
   connection( connection ),
   message(),
   login(),
   currentState( INIT )
{
	std::cout << "ClientConnection> " << "New client conncection created> " << technicalId << std::endl;
}

ClientConnection::~ClientConnection() 
{ 
	std::cout << "ClientConnection (" << technicalId << ") > Session destroyed" << std::endl;
}

const std::string& ClientConnection::getTechnicalId() const
{
   return technicalId;
}

const std::string& ClientConnection::getLogin() const
{
   return login;
}

void ClientConnection::close()
{
   connection->close();
}

void ClientConnection::waitForData()
{
	// Call the async listen using the connection
	connection->asyncRead( message, 
		                    boost::bind( &ClientConnection::handleRead, 
                                       shared_from_this(),
		                                 boost::asio::placeholders::error ) );
}

void ClientConnection::askForLogin()
{
   // send a login demand
   currentState = WAITING_FOR_LOGIN;
   sendMessage( MESSAGE_SYSTEM" "MESSAGE_LOGIN"_"MESSAGE_ASKED );
}

void ClientConnection::sendMessage(const std::string& message)
{
   // send the message on the network and finish with a \n 
   connection->asyncWrite( message,
		                     boost::bind( &ClientConnection::handleWrite, 
                                        shared_from_this(),
		                                  boost::asio::placeholders::error ) );
}

void ClientConnection::handleRead( const boost::system::error_code& error )
{
	if ( error == 0)
	{
      // check if it's the init message
      if (  ( currentState == INIT )
          &&( message == MESSAGE_INIT )  )
      {
         askForLogin();
      }
      // check if it's a login message
      else if ( currentState == WAITING_FOR_LOGIN )
      {
         // login|passwd are in the message
         login = message.substr( 0, message.find( ':' ) );
         std::string passwd = message.substr( message.find( ':' ) + 1 );

         // WAITIG_FOR_LOGIN and login is set, check the passwd
         if ( login == passwd )
         {
            // connection accepted
            currentState = CONNECTED;

            // send the acceptance message
            sendMessage( MESSAGE_SYSTEM" "MESSAGE_LOGIN"_"MESSAGE_ACCEPTED );
         }
         else
         {
            // refused the connection
            sendMessage( MESSAGE_SYSTEM" "MESSAGE_LOGIN"_"MESSAGE_REFUSED );

            // and close the socket
            std::cout << "ClientConnection (" << technicalId << ") > login refused: " << login << " | " << passwd << std::endl;
            connectionManager->closeConnection( shared_from_this() );
         }
      }
      else if ( currentState == CONNECTED )
      {
		   // manage the message
		   connectionManager->handleMessage( shared_from_this(),
                                           message );
      }

		// back to listen
		waitForData();
	}
	else
	{
      // if an error occurs, close the connection
      std::cout << "ClientConnection (" << technicalId << ") > handleRead call with error code: " << error.value() << " --> " << error.message() << std::endl;
      connectionManager->closeConnection( shared_from_this() );
	}
}

void ClientConnection::handleWrite( const boost::system::error_code& error )
{
   // if an error occurs, close the connection
	if ( error != 0 )
	{
      std::cout << "ClientConnection (" << technicalId << ") > handleWrite call with error code: " << error.value() << " --> " << error.message() << std::endl;
      connectionManager->closeConnection( shared_from_this() );
	}
}
