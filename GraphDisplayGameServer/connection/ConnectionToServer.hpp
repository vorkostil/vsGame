#pragma once 

#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../../BackBoneServer/SimpleTcpConnection.hpp"


// this class is use to represent a client connection
class ConnectionToServer : public boost::enable_shared_from_this< ConnectionToServer >
{
public:
   // auto reference fir enable shared
   typedef boost::shared_ptr< ConnectionToServer > ConnectionToServerPtr;

   // classic dtor
	~ConnectionToServer();

   // creator for the shared ptr mechanism
	static ConnectionToServerPtr create( const std::string& name,
                                        connection_ptr tcp_connection )
	{
		ConnectionToServerPtr session( new ConnectionToServer( name,
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

private:
   // the real ctor in the private zone as we use the shared ptr mechanism
	ConnectionToServer( const std::string& name,
                       connection_ptr connection );

   // listen on the socket using the tcp connection
	void	waitForData	(); 

   // callback of write result
	void	handleWrite( const boost::system::error_code& error );

   // callback of read result
	void	handleRead( const boost::system::error_code& error );

   // callback of connect result
	void	handleConnect( connection_ptr new_connection, 
                        const boost::system::error_code& error );

   // the buffer used to receive message
   std::string message;

   // the connection use to read / write on the network
	connection_ptr connection;

   // the name of the connection (internal id)
   std::string name;
};