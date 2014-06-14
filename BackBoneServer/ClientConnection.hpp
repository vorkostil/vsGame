#pragma once 

#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <set>
#include <map>

#include "network/SimpleTcpConnection.hpp"

class ConnectionManager;

// this class is use to represent a client connection
class ClientConnection : public boost::enable_shared_from_this< ClientConnection >
{
private:
   // the Connection manager
   ConnectionManager* connectionManager;

   // the buffer used to receive message
   std::string message;

   // the connection use to read / write on the network
	connection_ptr connection;

   // the name of the connection (internal id)
   std::string technicalId;

   // the login of the user
   std::string login;

   // the current status of the connection
   int currentState;

   // the load of the client (if it's a provider)
   size_t load;

   enum State
   {
      INIT = 0,
      WAITING_FOR_LOGIN,
      CONNECTED
   };

public:
   // auto reference fir enable shared
   typedef boost::shared_ptr< ClientConnection > InternalClientConnectionPtr;

   // classic dtor
	~ClientConnection();

   // creator for the shared ptr mechanism
	static InternalClientConnectionPtr create( const std::string& name,
                                      ConnectionManager* connectionManager,
                                      connection_ptr tcp_connection )
	{
		InternalClientConnectionPtr session( new ClientConnection( name,
                                                                 connectionManager,
                                                                 tcp_connection ) );
		session->waitForData();
		return session;
	}

   // send a message on the network
	void sendMessage(const std::string& message);

   // return the client name
   const std::string& getTechnicalId() const;

   // return the client name
   const std::string& getLogin() const;

   // close the connection
   void close();

   // increase the load of the provider
   void incLoad();

   // decrease the load of the provider
   void decLoad();

   // get the load of the provider
   size_t getLoad() const;

private:
   // the real ctor in the private zone as we use the shared ptr mechanism
	ClientConnection( const std::string& name,
                     ConnectionManager* connectionManager,
                     connection_ptr connection );

   // listen on the socket using the tcp connection
	void waitForData(); 

   // callback of write result
	void handleWrite( const boost::system::error_code& error );

   // callback of read result
	void handleRead( const boost::system::error_code& error );

   // ask the login of the client
   void askForLogin();
};

// the related typedef to ease the manipulation
typedef ClientConnection::InternalClientConnectionPtr ClientConnectionPtr;
typedef std::set< ClientConnectionPtr > ClientList;
typedef std::map< std::string, ClientList > ClientAggregat;
