#define _WIN32_WINNT 0x0501

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "ClientConnection.hpp"
#include "ConnectionManager.hpp"
#include "network/NetworkMessage.hpp"
#include "logger/asyncLogger.hpp"

ClientConnection::ClientConnection( const std::string& technicalId,
                                    ConnectionManager* connectionManager,
                                    connection_ptr connection )
:
   technicalId( technicalId ),
   connectionManager( connectionManager ),
   connection( connection ),
   message(),
   login(),
   currentState( INIT ),
   load( 0 )
{
   AsyncLogger::getInstance()->log( "ClientConnection> New client connection created> " + technicalId );
}

ClientConnection::~ClientConnection() 
{ 
	AsyncLogger::getInstance()->log( "ClientConnection (" + technicalId + ") > Session destroyed" );
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
   sendMessage( MESSAGE_LOGIN_ASKED );
}

void ClientConnection::sendMessage(const std::string& message)
{
   AsyncLogger::getInstance()->log( "WRITING TO (" + technicalId + "): " + message );

   // send the message on the network
   connection->asyncWrite( message + '\0',
		                     boost::bind( &ClientConnection::handleWrite, 
                                        shared_from_this(),
		                                  boost::asio::placeholders::error ) );
}

void ClientConnection::handleRead( const boost::system::error_code& error )
{
	if ( error == 0)
	{
      // invoke a thread to handle the message
      boost::thread worker( &ClientConnection::handleReadInThread,
                            this,
                            message );

		// back to listen
		waitForData();
	}
	else
	{
      // if an error occurs, close the connection
      std::stringstream stream;
      stream << "ClientConnection (" << technicalId << ") > handleRead call with error code: " << error.value() << " --> " << error.message();
      AsyncLogger::getInstance()->log( stream.str() );

      connectionManager->closeConnection( shared_from_this() );
	}
}

// callback of handle result in a separate thread
void ClientConnection::handleReadInThread( const std::string& messageToTreat )
{
   // check if it's the init message
   if (  ( currentState == INIT )
       &&( messageToTreat == MESSAGE_INIT )  )
   {
      askForLogin();
   }
   // check if it's a login message
   else if ( currentState == WAITING_FOR_LOGIN )
   {
      // login|passwd are in the message
      login = messageToTreat.substr( 0, messageToTreat.find( ':' ) );
      std::string passwd = messageToTreat.substr( messageToTreat.find( ':' ) + 1 );

      // WAITIG_FOR_LOGIN and login is set, check the passwd
      if ( connectionManager->isValidLogin( login,
                                            passwd ) == true )
      {
         // connection accepted
         currentState = CONNECTED;

         // send the acceptance message
         sendMessage( MESSAGE_LOGIN_ACCEPTED );
      }
      else
      {
         // refused the connection
         sendMessage( MESSAGE_LOGIN_REFUSED );

         // and close the socket
         AsyncLogger::getInstance()->log( "ClientConnection (" + technicalId + ") > login refused: " + login + " | " + passwd );
         connectionManager->closeConnection( shared_from_this() );
      }
   }
   else if ( currentState == CONNECTED )
   {
      // check the close connection message
      if ( message == MESSAGE_CLOSE )
      {
         // close the communication
         AsyncLogger::getInstance()->log( "ClientConnection (" + technicalId + ") > close connection" );
         connectionManager->closeConnection( shared_from_this() );
      }
      else
      {
		   // forward the message to the connection manager
		   connectionManager->handleMessage( shared_from_this(),
                                           messageToTreat );
      }
   }
}

void ClientConnection::handleWrite( const boost::system::error_code& error )
{
   // if an error occurs, close the connection
	if ( error != 0 )
	{
      std::stringstream stream;
      stream << "ClientConnection (" << technicalId << ") > handleWrite call with error code: " << error.value() << " --> " << error.message();
      AsyncLogger::getInstance()->log( stream.str() );

      connectionManager->closeConnection( shared_from_this() );
	}
}

// increase the load of the provider
void ClientConnection::incLoad()
{
   load++;
}

// decrease the load of the provider
void ClientConnection::decLoad()
{
   if ( load > 0 )
   {
      load--;
   }
}

// get the load of the provider
size_t ClientConnection::getLoad() const
{
   return load;
}
