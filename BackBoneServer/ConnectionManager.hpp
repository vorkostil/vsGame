#pragma once 

#include <boost/asio.hpp>
#include <set>
#include "ClientConnection.hpp"
#include "GameDefinition.hpp"

class Game;

// this class while listen on the given endpoint, accept the incoming connection
// and create a ClientConnection for each 
class ConnectionManager
{
   // member parts
   //-------------

	// the boost reactor
	boost::asio::io_service& boostReactor;

   // the boost acceptor used to listen on the socket for incoming connection
	boost::asio::ip::tcp::acceptor connectionAcceptor;

   // the list of current connection
   ClientList connections;

   // the list of clients indexed by game kind
   ClientAggregat consumerByGame;

   // the list of server indexed by game kind
   ClientAggregat providerByGame;

   // the list of defined game
   GameDefinitionMap gameDefinitions;

   // the list of current game indexed by gameId
   typedef std::map< std::string, Game* > GameMap;
   GameMap games;

public:
	// ctor with the used information
	ConnectionManager( boost::asio::io_service&              boostReactor, 
					       const boost::asio::ip::tcp::endpoint& endpoint );
		
	// used to wait for an incoming connection
	void waitForConnection();

   // used to handle message from a ClientConnection
   // those message can be 
   //     'SYSTEM_REGISTER <CONSUMER | PROVIDER> #Game [Game]' --> no answer
   //     'SYSTEM_REQUEST_GAME GameKind'
   //             'SYSTEM_REQUEST_GAME_REFUSED ErrorMessage'
   //             'SYSTEM_REQUEST_GAME_ACCEPTED GameId #Consumer [Consumer]'
   //     'SYSTEM_JOIN_GAME GameId'
   //     'SYSTEM_LEAVE_GAME GameId'
   //     '<gameId> MESSAGE'
   void handleMessage( ClientConnectionPtr connection,
                       const std::string& message );

   // close an dremove a ClientConnection
   void closeConnection( ClientConnectionPtr connection );

   // return true if the login:password is valid
   bool isValidLogin( const std::string& login,
                      const std::string& password );

private:

   // method parts
   //-------------

	// handle call when a connection is accepted on the socket
	void handle_accept( const boost::system::error_code&  error,
						     connection_ptr                    connection );

   // register a new connection on consumer or provider of game
   //     'SYSTEM_REGISTER <CONSUMER | PROVIDER> #Game [Game]' --> no answer
   void registerConnection( ClientConnectionPtr connection,
                            const std::string& message );

   // request a game to the server given its kind
   // compute the available player if needed
   // respond to the connection
   //     'SYSTEM_REQUEST_GAME GameKind'
   //             'SYSTEM_REQUEST_GAME_REFUSED ErrorMessage'
   //             'SYSTEM_REQUEST_GAME_ACCEPTED GameId #Consumer [Consumer]'
   void requestGame( ClientConnectionPtr connection,
                     const std::string& gameKind );

   // join the first non full game
   // or request a game to the server given its kind if no game exist or all is full
   // compute the available player if needed
   // respond to the connection
   //     'SYSTEM_JOIN_ORREQUEST_GAME GameKind'
   //             'SYSTEM_REQUEST_GAME_REFUSED ErrorMessage'
   //             'SYSTEM_REQUEST_GAME_ACCEPTED GameId #Consumer [Consumer]'
   void joinOrRequestGame( ClientConnectionPtr connection,
                           const std::string& gameKind );

   // join a known game given its gameId
   //     'SYSTEM_JOIN_GAME GameId'
   //          'SYSTEM_JOIN_GAME_REFUSED message'
   void joinGame( ClientConnectionPtr connection,
                  const std::string& gameId );

   // leave a current game given its gameId
   //     'SYSTEM_LEAVE_GAME GameId'
   void leaveGame( ClientConnectionPtr connection,
                   const std::string& gameId );

   // close a current game given its gameId
   //     'SYSTEM_GAME_CREATION_REFUSED GameId reason'
   void closeGame( ClientConnectionPtr connection,
                   const std::string& gameId,
                   const std::string& reason );

   // forward the message to the game given its ID
   //     '<gameId> MESSAGE'
   void handleGameMessage( ClientConnectionPtr connection,
                           const std::string& gameId,
                           const std::string& fullMessage );

   // find the less loaded provider in the list of provider
   ClientConnectionPtr findLessLoadedProvider( const ClientList& providers ) const;

   // display on std::cout the current state of the provider
   // connectiosn, games ...
   void dumpCurrentState() const;
};
