#pragma once 

#include <boost/asio.hpp>
#include <set>
#include "ClientConnection.hpp"

// this class while listen on the given endpoint, accept the incoming connection
// and create a ClientConnection for each 
class ConnectionManager
{
public:
	// ctor with the used information
	ConnectionManager( boost::asio::io_service&              boostReactor, 
					       const boost::asio::ip::tcp::endpoint& endpoint );
		
	// used to wait for an incoming connection
	void waitForConnection();

   // used to handle message from a ClientConnection
   void handleMessage( ClientConnection::ClientConnectionPtr connection,
                       const std::string& message );

   // close an dremove a ClientConnection
   void closeConnection( ClientConnection::ClientConnectionPtr connection );

private:
	// handle call when a connection is accepted on the socket
	void handle_accept( const boost::system::error_code&  error,
						     connection_ptr                    connection );

	// the boost reactor
	boost::asio::io_service& boostReactor;

   // the boost acceptor used to listen on the socket for incoming connection
	boost::asio::ip::tcp::acceptor connectionAcceptor;

   // the list of current connection
   std::set< ClientConnection::ClientConnectionPtr > connections;
};
