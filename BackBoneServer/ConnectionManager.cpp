#define _WIN32_WINNT 0x0501

#include <stdio.h>
#include <boost/bind.hpp>

#include "string/StringUtils.hpp"
#include "network/NetworkMessage.hpp"

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
		std::cout << "ConnectionManager> " << "Connection accepted> " << newClientName << std::endl;

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
#ifdef __DEBUG__
   // log the message
   std::cout << connection->getLogin() << "> " << message << std::endl;
#endif

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
      }
      else if ( messageParts[ 0 ] == SYSTEM_REQUEST_GAME )
      {
         requestGame( connection,
                      messageParts[ 1 ] );
      }
      else if ( messageParts[ 0 ] == SYSTEM_JOIN_GAME )
      {
         joinGame( connection,
                   messageParts[ 1 ] );
      }
      else if ( messageParts[ 0 ] == SYSTEM_LEAVE_GAME )
      {
         leaveGame( connection,
                    messageParts[ 1 ] );
      }
      else if ( messageParts[ 0 ] == GAME_MESSAGE )
      {
         handleGameMessage( connection,
                            messageParts[ 0 ],
                            messageParts[ 1 ] );
      }
#ifdef __DEBUG__
      dumpCurrentState();
#endif
   }
}

void ConnectionManager::closeConnection( ClientConnectionPtr connection )
{
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
            game->close();
            delete game;
            itGame = games.erase( itGame );
            continue;
         }
      }

      // go to the next game
      itGame++;
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

#ifdef __DEBUG__
      dumpCurrentState();
#endif
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

      // check if there is enough consumer to play the game
      // if IA can play, the game definition should reflect it by decreasing the minPlayer status
      if ( consumers.size() >= gameDef.minPlayer )
      {
         // create the game
         Game* game = new Game( gameDef, 
                                findLessLoadedProvider( itProviders->second ) );

         // create the list of available consumer (except the connection's one)
         std::vector< std::string > availableConsumers;

         // fill the list if needed
         if ( gameDef.maxPlayer != 1 )
         {
            // manage the IA parts
            if ( gameDef.iaAvailable == true )
            {
               availableConsumers.push_back( "IA" );
            }

            for ( ClientList::iterator itConsumer = consumers.begin();
                  itConsumer != consumers.end();
                  itConsumer++ )
            {
               if ( *itConsumer != connection )
               {
                  ClientConnectionPtr cPtr = *itConsumer;
                  availableConsumers.push_back( cPtr->getLogin() );
               }
            }
         }

         // create the acception message
         std::string message = CREATE_MESSAGE( SYSTEM_GAME_ACCEPTED, game->getId() );
         message += " " + availableConsumers.size();

         for ( std::vector< std::string >::const_iterator itStr = availableConsumers.begin();
               itStr != availableConsumers.end();
               itStr++ )
         {
            message += " " + *itStr;
         }

         // and send it
         connection->sendMessage( message );
      }
      else
      {
         connection->sendMessage( CREATE_MESSAGE( SYSTEM_GAME_REFUSED, "Not enough players available" ) );
      }
   }
   else
   {
      connection->sendMessage( CREATE_MESSAGE( SYSTEM_GAME_REFUSED, "Not server found for this game" ) );
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
         connection->sendMessage( CREATE_MESSAGE( SYSTEM_JOIN_GAME_REFUSED, "The game is full" ) );
      }
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
         game->close();
         delete game;
         games.erase( itGame );
      }
   }
}

// forward the message to the game given its ID
//     '<gameId> MESSAGE'
void ConnectionManager::handleGameMessage( ClientConnectionPtr connection,
                                           const std::string& gameId,
                                           const std::string& remainingMessage )
{
   // find the game
   GameMap::iterator itGame = games.find( gameId );
   if ( itGame != games.end() )
   {
      Game* game = itGame->second;
      game->handleMessage( connection,
                           remainingMessage );
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

#ifdef __DEBUG__
// display on std::cout the current state of the provider
// connectiosn, games ...
void ConnectionManager::dumpCurrentState() const
{
   std::cout << "------------------------------------------------------------------------------" << std::endl;
   std::cout << "connections:     " << connections.size() << std::endl;
   std::cout << "gameDefinitions: " << gameDefinitions.size() << std::endl;
   std::cout << "consumerByGame:  " << consumerByGame.size() << std::endl;
   std::cout << "providerByGame:  " << providerByGame.size() << std::endl;
   std::cout << "games:           " << games.size() << std::endl;
   std::cout << "------------------------------------------------------------------------------" << std::endl;
   std::cout << "CONNECTIONS: " << std::endl;
   for ( ClientList::const_iterator it = connections.begin();
         it != connections.end();
         it++ )
   {
      std::cout << "\t" << (*it)->getLogin() << std::endl;
   }
   std::cout << "------------------------------------------------------------------------------" << std::endl;
   std::cout << "GAME DEFINITION: " << std::endl;
   for ( GameDefinitionMap::const_iterator it = gameDefinitions.begin();
         it != gameDefinitions.end();
         it++ )
   {
      std::cout << "\t" << it->first << std::endl;
      std::cout << "\t\t" << it->second.kind << "\t" << it->second.minPlayer << "\t" << it->second.maxPlayer << "\t" << it->second.iaAvailable << std::endl;
   }
   std::cout << "------------------------------------------------------------------------------" << std::endl;
   std::cout << "CONSUMER BY GAME: " << std::endl;
   for ( ClientAggregat::const_iterator itAgg = consumerByGame.begin();
         itAgg != consumerByGame.end();
         itAgg++ )
   {
      std::cout << "\t" << itAgg->first << std::endl;
      for ( ClientList::const_iterator it = itAgg->second.begin();
            it != itAgg->second.end();
            it++ )
      {
         std::cout << "\t\t" << (*it)->getLogin() << std::endl;
      }
   }
   std::cout << "------------------------------------------------------------------------------" << std::endl;
   std::cout << "PROVIDER BY GAME: " << std::endl;
   for ( ClientAggregat::const_iterator itAgg = providerByGame.begin();
         itAgg != providerByGame.end();
         itAgg++ )
   {
      std::cout << "\t" << itAgg->first << std::endl;
      for ( ClientList::const_iterator it = itAgg->second.begin();
            it != itAgg->second.end();
            it++ )
      {
         std::cout << "\t\t" << (*it)->getLogin() << "\t" << (*it)->getLoad() << std::endl;
      }
   }
   std::cout << "------------------------------------------------------------------------------" << std::endl;
   std::cout << "CURRENT GAME: " << std::endl;
   for ( GameMap::const_iterator it = games.begin();
         it != games.end();
         it++ )
   {
      std::cout << "\t" << it->first << std::endl;
      std::cout << "\t\t" << it->second->getProvider()->getLogin() << std::endl;
      for ( ClientList::const_iterator itClient = it->second->getClients().begin();
            itClient != it->second->getClients().end();
            itClient++ )
      {
         std::cout << "\t\t\t" << (*itClient)->getLogin() << std::endl;
      }
   }
   std::cout << "------------------------------------------------------------------------------" << std::endl;

}
#endif