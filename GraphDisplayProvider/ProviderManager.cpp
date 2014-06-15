#define _WIN32_WINNT 0x0501

#include <boost/asio.hpp>
#include "ProviderManager.hpp"
#include "network/NetworkMessage.hpp"
#include "graph/GraphGridProvider.hpp"

// the maximum of available game at a given time
const int ProviderManager::MAX_GAME_POOL_SIZE = 1;

// default ctor
ProviderManager::ProviderManager( ConnectionToServerPtr connection )
:
   connection( connection ),
   login(),
   gamePool()
{
   connection->setNetworkClient( this );
}

// connect to the BBServer
void ProviderManager::connect( const std::string& host,
                                 int port )
{
   connection->connect( boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string( host ),
                                                        port ) );
}

// callback when the connection is accepted
void ProviderManager::onConnection()
{
   login = GraphGridProvider::NAME + "_" + connection->getLocalEndPointAsString();
}


// get the login
const std::string& ProviderManager::getLogin() const
{
   return login;
}

// get the passwd
const std::string& ProviderManager::getPassword() const
{
   return login;
}

// call back when the login procotol succeed
void ProviderManager::onLoginSucced()
{
   // register as provider
   // [GameName MinPlayer MaxPlayer IAAvailable]
   connection->sendMessage( SYSTEM_REGISTER + " " + PROVIDER_PART + " " + GraphGridProvider::NAME + " 1 1 0" );
}

// call back when a game creation message is received
void ProviderManager::onNewGameCreation( const std::string& gameId )
{
   // check the pool size
   if ( gamePool.size() < MAX_GAME_POOL_SIZE )
   {
      // create a new game
      GraphGridProvider* game = new GraphGridProvider( 32, 32 );

      // set its network information
      game->setNetworkInformation( this,
                                   gameId );

      // and store it
      gamePool.insert( GamePool::value_type( gameId,
                                             game ) );
   }
   else
   {
      // send the refused message
      connection->sendMessage( SYSTEM_GAME_CREATION_REFUSED + " " + gameId + " No more slot available" );
   }
   dumpCurrentState();
}

// callback used to handle the message of game closure
void ProviderManager::onGameClose( const std::string& gameId,
                                   const std::string& reason )
{
   // check if the game is managed
   GamePool::iterator itGame = gamePool.find( gameId );
   if ( itGame != gamePool.end() )
   {
      // close the game
      itGame->second->close( reason );

      // and get back the memory
      delete itGame->second;
      gamePool.erase( itGame );
   }
   dumpCurrentState();
}

// callback used to handle the message when logon
void ProviderManager::onHandleMessage( const std::string& gameId,
                                       const std::string& message )
{
   // check if the game is managed
   GamePool::iterator itGame = gamePool.find( gameId );
   if ( itGame != gamePool.end() )
   {
      itGame->second->handleGameMessage( message );
   }
}

// forward the message on the network
void ProviderManager::sendMessage( const std::string& message )
{
   connection->sendMessage( message );
}

// dump the current state of the server
void ProviderManager::dumpCurrentState() const
{
   std::cout << "----------------------------------------------------------------------------------------" << std::endl;
   std::cout << "games: " << gamePool.size() << std::endl;
   std::cout << "----------------------------------------------------------------------------------------" << std::endl;
   for ( GamePool::const_iterator it = gamePool.begin();
         it != gamePool.end();
         it++ )
   {
      std::cout << "\t" << it->first << std::endl;
   }
   std::cout << "----------------------------------------------------------------------------------------" << std::endl;
}
