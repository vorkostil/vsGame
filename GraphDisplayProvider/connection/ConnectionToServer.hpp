#pragma once 

#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "network/SimpleTcpConnection.hpp"

class NetworkClient;

// this class is use to represent a client connection
class ConnectionToServer : public boost::enable_shared_from_this< ConnectionToServer >
{
private:

   // the status of the connection
   enum State
   {
      INIT = 0,
      LOGIN,
      CONNECTED
   };
   int status;

   // the buffer used to receive message
   std::string message;

   // the connection use to read / write on the network
	connection_ptr connection;

   // the name of the connection (internal id)
   std::string name;

   // the client using the connection
   NetworkClient* client;

public:
   // auto reference fir enable shared
   typedef boost::shared_ptr< ConnectionToServer > InternalConnectionToServerPtr;

   // classic dtor
	~ConnectionToServer();

   // creator for the shared ptr mechanism
	static InternalConnectionToServerPtr create( const std::string& name,
                                                connection_ptr tcp_connection)
	{
		InternalConnectionToServerPtr session( new ConnectionToServer( name,
                                                                     tcp_connection ) );
		return session;
	}

   // send a message on the network
	void sendMessage(const std::string& message);

   // return the client name
   const std::string& getName() const;

   // close the connection
   void close();

   // connect to the server known by its endpoint
   void connect( boost::asio::ip::tcp::endpoint& endpoint );

   // set the client user of this connection
   void setNetworkClient( NetworkClient* client );

   // return the localendpoint as string host:port
   std::string getLocalEndPointAsString() const;

private:
   // the real ctor in the private zone as we use the shared ptr mechanism
	ConnectionToServer( const std::string& name,
                       connection_ptr connection );

   // listen on the socket using the tcp connection
	void	waitForData	(); 

   // callback of write result
	void handleWrite( const boost::system::error_code& error );

   // callback of read result
	void handleRead( const boost::system::error_code& error );

   // thread used to decipher and manage the message
   void handleMessageInThread( const std::string& messageToTreat );

   // callback of connect result
	void handleConnect( connection_ptr new_connection, 
                       const boost::system::error_code& error );
};

// typedef to ease the coding
typedef ConnectionToServer::InternalConnectionToServerPtr ConnectionToServerPtr;
