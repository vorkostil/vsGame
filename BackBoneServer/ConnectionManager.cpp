#define _WIN32_WINNT 0x0501

#include <stdio.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "string/StringUtils.hpp"
#include "network/NetworkMessage.hpp"
#include "logger/asyncLogger.hpp"

#include "ConnectionManager.hpp"
#include "Game.hpp"
#include "ClientConnection.hpp"

std::string createNewClientName()
{
   static int id = 0;
   char result[20];
   sprintf_s( result,
              20,
              "client_%d",
              id++ );
   return result;
}

ConnectionManager::ConnectionManager( boost::asio::io_service&              boostReactor, 
                                      const boost::asio::ip::tcp::endpoint& endpoint )
:
   boostReactor( boostReactor ),
   connectionAcceptor( boostReactor, 
                       endpoint ),
   connections(),
   consumerByGame(),
   providerByGame(),
   gameDefinitions(),
   games()
{
   // waiting for the connection
	waitForConnection();
}

void ConnectionManager::waitForConnection()
{
	connection_ptr new_connection(new SimpleTcpConnection( boostReactor ) );

	// Attente d'une nouvelle connection
	connectionAcceptor.async_accept( new_connection->getSocket(),
		                              boost::bind( &ConnectionManager::handle_accept, 
                                                 this,
		                                           boost::asio::placeholders::error,
                                                 new_connection ) );
}

void ConnectionManager::handle_accept( const boost::system::error_code& error, 
                                       connection_ptr new_connection )
{
	if ( error == 0)
	{
      // create the unique id
      std::string newClientName = createNewClientName();
		AsyncLogger::getInstance()->log( "ConnectionManager> Connection accepted> " + newClientName );

      // create the client
		ClientConnectionPtr client = ClientConnection::create( newClientName, 
                                                             this,
                                                             new_connection );

      // and back to client acceptance
		waitForConnection();
	}
	else 
   {
		std::cerr << "ConnectionManager> " << "Connection refused" << std::endl;
	}
}

// used to handle message from a ClientConnection
// those message can be 
//     'SYSTEM_REGISTER <CONSUMER | PROVIDER> #Game [Game]' --> no answer
//     'SYSTEM_REQUEST_GAME GameKind'
//             'SYSTEM_REQUEST_GAME_REFUSED ErrorMessage'
//             'SYSTEM_REQUEST_GAME_ACCEPTED GameId #Consumer [Consumer]'
//     'SYSTEM_JOIN_GAME GameId'
//     'SYSTEM_LEAVE_GAME GameId'
//     '<gameId> MESSAGE'
void ConnectionManager::handleMessage( ClientConnectionPtr connection,
                                       const std::string& message )
{
   // log the message
   AsyncLogger::getInstance()->log( "RECEIVE FROM (" + connection->getLogin() + ") : " + message );

   // explode the message to be able to check the kind 
   std::vector< std::string > messageParts;
   if ( StringUtils::explode( message,
                              ' ',
                              messageParts,
                              2 ) == 2 )
   {
      if ( messageParts[ 0 ] == SYSTEM_REGISTER )
      {
         registerConnection( connection,
                              messageParts[ 1 ] );
         dumpCurrentState();
      }
      else if ( messageParts[ 0 ] == SYSTEM_REQUEST_GAME )
      {
         requestGame( connection,
                        messageParts[ 1 ] );
         dumpCurrentState();
      }
      else if ( messageParts[ 0 ] == SYSTEM_REQUEST_GAME_LIST )
      {
         requestGameList( connection,
                           messageParts[ 1 ] );
      }
      else if ( messageParts[ 0 ] == SYSTEM_JOIN_OR_REQUEST_GAME )
      {
         joinOrRequestGame( connection,
                              messageParts[ 1 ] );
         dumpCurrentState();
      }
      else if ( messageParts[ 0 ] == SYSTEM_JOIN_GAME )
      {
         joinGame( connection,
                     messageParts[ 1 ] );
         dumpCurrentState();
      }
      else if ( messageParts[ 0 ] == SYSTEM_LEAVE_GAME )
      {
         leaveGame( connection,
                     messageParts[ 1 ] );
         dumpCurrentState();
      }
      else if ( messageParts[ 0 ] == SYSTEM_GAME_CREATION_REFUSED )
      {
         // get the relevant information
         std::vector< std::string > messageInformation;
         StringUtils::explode( messageParts[ 1 ],
                                 ' ',
                                 messageInformation,
                                 2 );

         // and close the game
         closeGame( connection,
                     messageInformation[ 0 ],
                     messageInformation[ 1 ] );
         dumpCurrentState();
      }
      else if ( messageParts[ 0 ] == GAME_MESSAGE )
      {
         // get the relevant information
         std::vector< std::string > messageInformation;
         StringUtils::explode( messageParts[ 1 ],
                                 ' ',
                                 messageInformation,
                                 2 );

         // and forward the message (gameId, message)
         handleGameMessage( connection,
                              messageInformation[ 0 ],
                              message );
      }
   }
}

void ConnectionManager::closeConnection( ClientConnectionPtr connection )
{
   std::set< std::string > gameToCloseList;

   // find all the game related to this connection
   for ( GameMap::iterator itGame = games.begin();
         itGame != games.end();
         )
   {
      // get the current game
      Game* game = itGame->second;

      // check the contains status
      if ( game->contains( connection ) == true )
      {
         // remove the connection from the game
         if ( game->remove( connection ) == true )
         {
            // if the connection was the provider, close the game
            gameToCloseList.insert( game->getId() );
            delete game;
            itGame = games.erase( itGame );
            continue;
         }
         else if ( game->getClients().size() == 0 )
         {
            // if there is no more players
            gameToCloseList.insert( game->getId() );
            delete game;
            itGame = games.erase( itGame );
            continue;
         }
      }

      // go to the next game
      itGame++;
   }

   // check if there is some game to close
   if ( gameToCloseList.size() > 0 )
   {
      // create the close message
      std::string closeMessage( GAME_MESSAGE + " " + CLOSE_MESSAGE + " " );
      size_t i = 0;
      for ( std::set< std::string >::const_iterator it = gameToCloseList.begin();
            it != gameToCloseList.end();
            it++ )
      {
         closeMessage += *it;
         if ( i++ < gameToCloseList.size() - 1 )
         {
            closeMessage += "|";
         }
      }
      closeMessage += " Client close its connection and end the game" ;

      // and send it to everyone
      for ( ClientList::const_iterator itClient = connections.begin();
            itClient != connections.end();
            itClient++ )
      {
         (*itClient)->sendMessage( closeMessage );
      }
   }

   // remove the connection from the client aggregat
   for ( ClientAggregat::iterator itAgg = consumerByGame.begin();
         itAgg != consumerByGame.end();
         )
   {
      for ( ClientList::iterator itClient = itAgg->second.begin();
            itClient != itAgg->second.end();
            itClient++ )
      {
         // check the client
         if ( *itClient == connection )
         {
            // remove it
            itAgg->second.erase( itClient );

            // and break as there is only one occurence of client per game
            break;
         }
      }

      // check if there is still some client for the game
      if ( itAgg->second.size() == 0 )
      {
         itAgg = consumerByGame.erase( itAgg );
         continue;
      }

      // and next game
      itAgg++;
   }

   // remove the connection from the provider aggregat
   for ( ClientAggregat::iterator itAgg = providerByGame.begin();
         itAgg != providerByGame.end();
         )
   {
      for ( ClientList::iterator itClient = itAgg->second.begin();
            itClient != itAgg->second.end();
            itClient++ )
      {
         // check the client
         if ( *itClient == connection )
         {
            // remove it
            itAgg->second.erase( itClient );

            // and break as there is only one occurence of client per game
            break;
         }
      }

      // check if there is still some client for the game
      if ( itAgg->second.size() == 0 )
      {
         itAgg = providerByGame.erase( itAgg );
         continue;
      }

      // and next game
      itAgg++;
   }

   // remove the connections from the list 
   connections.erase( connection );

   dumpCurrentState();
}

// register a new connection on consumer or provider of game
//     'SYSTEM_REGISTER CONSUMER [Game]' --> no answer
//     'SYSTEM_REGISTER PROVIDER [GameName MinPlayer MaxPlayer IAAvailable]' --> no answer
void ConnectionManager::registerConnection( ClientConnectionPtr connection,
                                            const std::string& message )
{
   // check if the connection already exist
   if ( connections.find( connection ) == connections.end() )
   {
      // register only if there is at least one game to consume/provide
      std::vector< std::string > messageParts;
      size_t size;
      if ( ( size = StringUtils::explode( message,
                                          ' ',
                                          messageParts ) ) > 1 )
      {
         // get the kind of the client
         if ( messageParts[ 0 ] == CONSUMER_PART )
         {
            for ( size_t i = 1;
                  i < size;
                  ++i )
            {
               // check if there is an already existing game by inserting it
               // either none exist and there is one created
               // or one already exist and we get the iterator
               ClientAggregat::iterator it = consumerByGame.insert( ClientAggregat::value_type( messageParts[ i ],
                                                                                                ClientList() ) ).first;
               it->second.insert( connection );
            }
         }
         else if ( messageParts[ 0 ] == PROVIDER_PART )
         {
            for ( size_t i = 1;
                  i < size;
                  i += 4 )
            {
               // check if there is an already existing game by checking the game description
               if ( gameDefinitions.find( messageParts[ i ] ) == gameDefinitions.end() )
               {
                  gameDefinitions.insert( GameDefinitionMap::value_type( messageParts[ i ],
                                                                         GameDefinition( messageParts[ i ],
                                                                                         atoi( messageParts[ i + 1 ].c_str() ),
                                                                                         atoi( messageParts[ i + 2 ].c_str() ),
                                                                                         atoi( messageParts[ i + 3 ].c_str() ) ) ) );
               }

               // inserting the connection whatever happens
               // either none exist and there is one created
               // or one already exist and we get the iterator
               ClientAggregat::iterator it = providerByGame.insert( ClientAggregat::value_type( messageParts[ i ],
                                                                                                ClientList() ) ).first;
               it->second.insert( connection );
            }
         }
         else
         {
            throw std::exception( std::string( "Not able to register correctly " + connection->getTechnicalId() + " name " + connection->getLogin() ).c_str() );
         }

         // register to the connections
         connections.insert( connection );
      }
   }
}

// request a game to the server given its kind
// compute the available player if needed
// respond to the connection
//     'SYSTEM_REQUEST_GAME GameKind'
//             'SYSTEM_REQUEST_GAME_REFUSED ErrorMessage'
//             'SYSTEM_REQUEST_GAME_ACCEPTED GameId #Consumer [Consumer]'
void ConnectionManager::requestGame( ClientConnectionPtr connection,
                                     const std::string& gameKind )
{
   // first check if there is at least one provider for it
   ClientAggregat::iterator itProviders = providerByGame.find( gameKind );
   if ( itProviders != providerByGame.end() )
   {
      // get the game definition
      GameDefinition& gameDef = gameDefinitions.find( gameKind )->second;

      // get the list of consumers available (always define as there is at least the requester present)
      ClientList consumers = consumerByGame.find( gameKind )->second;

      // create the game
      Game* game = new Game( gameDef, 
                             findLessLoadedProvider( itProviders->second ) );

      // store it
      games.insert( GameMap::value_type( game->getId(),
                                         game ) );

      // alert the provider about a new game creation
      // it can send back a GAME_CREATION_REFUSED which will leads to close the game
      game->getProvider()->sendMessage( GAME_MESSAGE + " " + GAME_CREATED + " " + game->getId()  + " " + gameDef.kind );

      // and send the accept message to the client
      connection->sendMessage( GAME_MESSAGE + " " + GAME_ACCEPTED + " " + game->getId() + " " + gameDef.kind );
   }
   else
   {
      connection->sendMessage( GAME_MESSAGE + " " + GAME_REFUSED + " No server found to handle this game" );
   }
}

// request a list of game to the server given its kind
// respond to the connection
//     'SYSTEM_REQUEST_GAME_LIST GameKind'
//             'SYSTEM_REQUEST_GAME_LIST_RESULT [game]'
void ConnectionManager::requestGameList( ClientConnectionPtr connection,
                                         const std::string& gameKind ) const
{
   std::string responseMessage( SYSTEM_REQUEST_GAME_LIST_RESULT );

   // check if there is avaialble game 
   for ( GameMap::const_iterator itGame = games.begin();
         itGame != games.end();
         itGame++ )
   {
      Game* game = itGame->second;

      // check if the game is of good kind and if there is enough places
      if (  ( game->getKind() == gameKind )
          &&( game->placeAvailable() == true )  )
      {
         responseMessage += " " + game->getId();
      }
   }

   connection->sendMessage( responseMessage );
}

// join the first non full game
// or request a game to the server given its kind if no game exist or all is full
// compute the available player if needed
// respond to the connection
//     'SYSTEM_JOIN_ORREQUEST_GAME GameKind'
//             'SYSTEM_REQUEST_GAME_REFUSED ErrorMessage'
//             'SYSTEM_REQUEST_GAME_ACCEPTED GameId #Consumer [Consumer]'
void ConnectionManager::joinOrRequestGame( ClientConnectionPtr connection,
                                           const std::string& gameKind )
{
   bool gameFound = false;

   // check if there is avaialble game 
   for ( GameMap::iterator itGame = games.begin();
         itGame != games.end();
         itGame++ )
   {
      Game* game = itGame->second;

      // check if the game is of good kind and if there is enough places
      if (  ( game->getKind() == gameKind )
          &&( game->placeAvailable() == true )  )
      {
         // send the accept message to the client
         connection->sendMessage( GAME_MESSAGE + " " + GAME_ACCEPTED + " " + game->getId() + " " + gameKind );

         // and quit the function
         gameFound = true;
         break;
      }
   }

   // check if there is a need of a new game
   if ( gameFound == false )
   {
      requestGame( connection,
                   gameKind );
   }
}

// join a known game given its gameId
//     'SYSTEM_JOIN_GAME GameId'
//          'SYSTEM_JOIN_GAME_REFUSED message'
void ConnectionManager::joinGame( ClientConnectionPtr connection,
                                  const std::string& gameId )
{
   // find the game
   GameMap::iterator itGame = games.find( gameId );
   if ( itGame != games.end() )
   {
      Game* game = itGame->second;

      // check if there is enough places
      if ( game->placeAvailable() == true )
      {
         // add the player to the game
         game->addConsumer( connection );
      }
      else
      {
         connection->sendMessage( GAME_MESSAGE + " " + GAME_JOIN_REFUSED + " " + gameId + " The game is full" );
      }
   }
   else
   {
      connection->sendMessage( GAME_MESSAGE + " " + GAME_JOIN_REFUSED + " " + gameId + " The game is unknown" );
   }
}

// leave a current game given its gameId
//     'SYSTEM_LEAVE_GAME GameId'
void ConnectionManager::leaveGame( ClientConnectionPtr connection,
                                   const std::string& gameId )
{
   // find the game
   GameMap::iterator itGame = games.find( gameId );
   if ( itGame != games.end() )
   {
      Game* game = itGame->second;
      if ( game->remove( connection ) == true )
      {
         // if the connection was the provider, close the game
         game->close( "Provider leave the network" );
         delete game;
         games.erase( itGame );
      }
      else if ( game->getClients().size() == 0 )
      {
         // if there is no more players
         game->close( "No more players" );
         delete game;
         games.erase( itGame );
      }
   }
}

// close a current game given its gameId
//     'SYSTEM_GAME_CREATION_REFUSED GameId reason'
void ConnectionManager::closeGame( ClientConnectionPtr connection,
                                   const std::string& gameId,
                                   const std::string& reason )
{
   // find the game
   GameMap::iterator itGame = games.find( gameId );
   if ( itGame != games.end() )
   {
      Game* game = itGame->second;

      // and close it
      game->close( reason );
      delete game;
      games.erase( itGame );
   }
}

// forward the message to the game given its ID
//     '<gameId> MESSAGE'
void ConnectionManager::handleGameMessage( ClientConnectionPtr connection,
                                           const std::string& gameId,
                                           const std::string& fullMessage )
{
   // find the game
   GameMap::iterator itGame = games.find( gameId );
   if ( itGame != games.end() )
   {
      Game* game = itGame->second;
      game->handleMessage( connection,
                           fullMessage );
   }
}

// find the less loaded provider in the list of provider
ClientConnectionPtr ConnectionManager::findLessLoadedProvider( const ClientList& providers ) const
{
   // get the first provider
   ClientList::iterator itProvider = providers.begin();
   ClientConnectionPtr lessLoadedProvider = *itProvider;

   // and found the less loaded
   while( itProvider != providers.end() )
   {
      if ( (*itProvider)->getLoad() < lessLoadedProvider->getLoad() )
      {
         lessLoadedProvider = *itProvider;
      }
      itProvider++;
   }

   // return it
   return lessLoadedProvider;
}

// return true if the login:password is valid
bool ConnectionManager::isValidLogin( const std::string& login,
                                      const std::string& password )
{
   return ( login == password );
}

// display on std::cout the current state of the provider
// connectiosn, games ...
void ConnectionManager::dumpCurrentState() const
{
   std::stringstream stream;
   stream << std::endl;
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   stream << "connections:     " << connections.size() << std::endl;
   stream << "gameDefinitions: " << gameDefinitions.size() << std::endl;
   stream << "consumerByGame:  " << consumerByGame.size() << std::endl;
   stream << "providerByGame:  " << providerByGame.size() << std::endl;
   stream << "games:           " << games.size() << std::endl;
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   stream << "CONNECTIONS: " << std::endl;
   for ( ClientList::const_iterator it = connections.begin();
         it != connections.end();
         it++ )
   {
      stream << "\t" << (*it)->getTechnicalId() << "\t" << (*it)->getLogin() << std::endl;
   }
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   stream << "GAME DEFINITION: " << std::endl;
   for ( GameDefinitionMap::const_iterator it = gameDefinitions.begin();
         it != gameDefinitions.end();
         it++ )
   {
      stream << "\t" << it->first << std::endl;
      stream << "\t\t" << it->second.kind << "\t" << it->second.minPlayer << "\t" << it->second.maxPlayer << "\t" << it->second.iaAvailable << std::endl;
   }
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   stream << "CONSUMER BY GAME: " << std::endl;
   for ( ClientAggregat::const_iterator itAgg = consumerByGame.begin();
         itAgg != consumerByGame.end();
         itAgg++ )
   {
      stream << "\t" << itAgg->first << std::endl;
      for ( ClientList::const_iterator it = itAgg->second.begin();
            it != itAgg->second.end();
            it++ )
      {
         stream << "\t\t" << (*it)->getTechnicalId() << "\t" << (*it)->getLogin() << std::endl;
      }
   }
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   stream << "PROVIDER BY GAME: " << std::endl;
   for ( ClientAggregat::const_iterator itAgg = providerByGame.begin();
         itAgg != providerByGame.end();
         itAgg++ )
   {
      stream << "\t" << itAgg->first << std::endl;
      for ( ClientList::const_iterator it = itAgg->second.begin();
            it != itAgg->second.end();
            it++ )
      {
         stream << "\t\t" << (*it)->getTechnicalId() << "\t" << (*it)->getLogin() << "\t" << (*it)->getLoad() << std::endl;
      }
   }
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   stream << "CURRENT GAME: " << std::endl;
   for ( GameMap::const_iterator it = games.begin();
         it != games.end();
         it++ )
   {
      Game* game = it->second;
      const ClientList& clients = game->getClients();

      stream << "\t" << it->first << std::endl;
      stream << "\t\t" << game->getProvider()->getTechnicalId() << "\t" << game->getProvider()->getLogin() << "\t--> " << clients.size() << " clients." << std::endl;
      for ( ClientList::const_iterator itClient = clients.begin();
            itClient != clients.end();
            itClient++ )
      {
         stream << "\t\t\t" << (*itClient)->getTechnicalId() << "\t" << (*itClient)->getLogin() << std::endl;
      }
   }
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   AsyncLogger::getInstance()->log( stream.str() );
}
