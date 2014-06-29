#pragma once 

#include <boost/asio.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>
#include <boost/thread/mutex.hpp>
#include "../logger/asyncLogger.hpp"

#define SOCKET_READ_SIZE 1024

// this class is used to encapsulate asynchronous read / write on the network
// this class is fully inline to ease the sharing
class SimpleTcpConnection
{
private:

	// the socket used for communication
	boost::asio::ip::tcp::socket connectionSocket;

   // the buffer used to send message
	std::string       outMessage;

   // the buffer used to read incoming message
   std::vector< char > readBuffer;

   // the buffer sued to store message grater than SOCKET_READ_SIZE
   std::string readMessage;

   // the write mutex
   boost::mutex writeMutex;

public:
   // Create a cimple tcp connection to exchange async message
	SimpleTcpConnection( boost::asio::io_service& boostReactor )
   : 
      connectionSocket( boostReactor ),
      readBuffer(),
      readMessage()
   {
      AsyncLogger::getInstance()->log( "SimpleTcpConnection> SimpleTcpConnection created" );
   }

   // dtor
   ~SimpleTcpConnection()
   {
      // wait for the message to be sent
      // it's awful but it works
      writeMutex.lock();
      writeMutex.unlock();
   }

   // get the current boost socket
	boost::asio::ip::tcp::socket& getSocket()
   {
	   return connectionSocket;
   }

   // close the connection
   void close()
   {
      connectionSocket.close();
   }

   // write the message on the socket and use the templated Handler for callback
	template< typename Handler >
	void asyncWrite( const std::string& message, 
                    Handler handler )
   {
      writeMutex.lock();
      /*|*/ // write the message on the socket
	   /*|*/ outMessage = message;
	   /*|*/ boost::asio::async_write( connectionSocket, 
      /*|*/                           boost::asio::buffer( outMessage ),
      /*|*/                           handler );
      writeMutex.unlock();
   }

	// asynchronous read using the handler for callback
	template< typename Handler >
	void asyncRead( std::string& message, 
                   Handler handler )
   {
#ifdef __DEBUG__
      AsyncLogger::getInstance()->log( "SimpleTcpConnection> Reading on the socket ..." );
#endif

      // initialize the buffer 
      readBuffer.clear();
      readBuffer.resize( SOCKET_READ_SIZE );

      // call the async read
	   void (SimpleTcpConnection::*callback)( const boost::system::error_code&, 
                                             size_t,         
                                             std::string&, 
                                             boost::tuple< Handler > ) = &SimpleTcpConnection::handleRead< Handler >;

      boost::asio::async_read( connectionSocket, 
                               boost::asio::buffer( readBuffer ),
                               boost::asio::transfer_at_least(1), // at least 1 charactrers on the socket before reading the buffer
		                         boost::bind( callback, 
                                            this,
		                                      boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred,
                                            boost::ref( message ), 
                                            boost::make_tuple( handler ) ) );
   }

private:
   // handle message reception and signal it to the caller
   template< typename Handler >
   void handleRead( const boost::system::error_code& error,
                    size_t numberOfBytes,
	                 std::string& message, 
                    boost::tuple< Handler > handler )
   {
#ifdef __DEBUG
      std::stringstream stream;
      stream << "SimpleTcpConnection> Message received (" << readBuffer.size() << ") - (" << numberOfBytes << "): '@'" << readBuffer.data() << "'@'" << std::endl;
      AsyncLogger::getInstance()->log( stream.str() );
#endif
      // check if an error occurs
      if ( error )
      {
         // if so, alert the caller
         boost::get< 0 >( handler )( error );
      }
      else
      {
         for ( size_t i = 0; i < numberOfBytes; i++)
         {
            if ( readBuffer[ i ] == 0 )
               break;
            readMessage += readBuffer[ i ];
         }

         // check if there is still something to read
         if ( numberOfBytes == readBuffer.size() )
         {
            asyncRead( message,
                       boost::get< 0 >( handler ) );
         }
         else
         {
            // get the message from the socket
            message = readMessage;
            readMessage.clear();

            // alert the caller that something can be done with the readed data
            boost::get< 0 >( handler )( error );
         }
      }
   }
};

// typed used for the connection reset
typedef boost::shared_ptr< SimpleTcpConnection > connection_ptr;
