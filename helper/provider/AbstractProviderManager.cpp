#define _WIN32_WINNT 0x0501

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "AbstractProviderManager.hpp"
#include "AbstractGameProvider.hpp"
#include "network/NetworkMessage.hpp"
#include "string/StringUtils.hpp"

// default ctor
AbstractProviderManager::AbstractProviderManager( ConnectionToServerPtr connection )
:
   connection( connection ),
   login(),
   gamePool()
{
   connection->setNetworkClient( this );
}

// connect to the BBServer
void AbstractProviderManager::connect( const std::string& host,
                                 int port )
{
   connection->connect( boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string( host ),
                                                        port ) );
}

// callback when the connection is accepted
void AbstractProviderManager::onConnection()
{
   login = getName() + "_" + connection->getLocalEndPointAsString();
}


// get the login
const std::string& AbstractProviderManager::getLogin() const
{
   return login;
}

// get the passwd
const std::string& AbstractProviderManager::getPassword() const
{
   return login;
}

// call back when the login procotol succeed
void AbstractProviderManager::onLoginSucced()
{
   // register as provider
   // [GameName MinPlayer MaxPlayer IAAvailable]
   connection->sendMessage( SYSTEM_REGISTER + " " + PROVIDER_PART + " " + getGameDefinitionForRegistration() );
}

// call back when a game creation message is received
void AbstractProviderManager::onNewGameCreation( const std::string& gameId,
                                                 const std::string& gameKind )
{
   // check the pool size
   if ( gamePool.size() < getMaxGameInPool() )
   {
      // create a new game
      AbstractGameProvider* game = requireNewGame( gameKind );

      // set its network information
      game->setNetworkInformation( this,
                                   gameId );

      // get the lock
      gamePoolMutex.lock();
      /*|*/ 
      /*|*/ // store the game
      /*|*/ gamePool.insert( GamePool::value_type( gameId,
      /*|*/                                        game ) );
      /*|*/ 
      // and release the kraken
      gamePoolMutex.unlock();
   }
   else
   {
      // send the refused message
      connection->sendMessage( SYSTEM_GAME_CREATION_REFUSED + " " + gameId + " No more slot available" );
   }
   // get the lock
   gamePoolMutex.lock();
   /*|*/ 
   /*|*/ // dump the state
   /*|*/ dumpCurrentState();
   /*|*/ 
   // release the lock
   gamePoolMutex.unlock();
}

// callback used to handle the message of game closure
void AbstractProviderManager::onGameClose( const std::string& gameIds,
                                   const std::string& reason )
{
   // explode the game id list
   std::vector< std::string > gameIdList;
   StringUtils::explode( gameIds,
                         '|',
                         gameIdList );

   // get the lock
   gamePoolMutex.lock();
   /*|*/ 
   /*|*/ for ( std::vector< std::string >::const_iterator it = gameIdList.begin();
   /*|*/       it != gameIdList.end();
   /*|*/       it++ )
   /*|*/ {
   /*|*/    // check if the game is managed
   /*|*/    GamePool::iterator itGame = gamePool.find( *it );
   /*|*/    if ( itGame != gamePool.end() )
   /*|*/    {
   /*|*/       // close the game
   /*|*/       itGame->second->close( reason );
   /*|*/ 
   /*|*/       // and get back the memory
   /*|*/       delete itGame->second;
   /*|*/       gamePool.erase( itGame );
   /*|*/    }
   /*|*/ }
   /*|*/ 
   /*|*/ // display the new state
   /*|*/ dumpCurrentState();
   /*|*/ 
   // and release the lock
   gamePoolMutex.unlock();
}

// callback used to handle the message when logon
void AbstractProviderManager::onHandleMessage( const std::string& gameId,
                                       const std::string& message )
{
   // get the lock
   gamePoolMutex.lock();
   /*|*/ 
   /*|*/ // check if the game is managed
   /*|*/ GamePool::iterator itGame = gamePool.find( gameId );
   /*|*/ if ( itGame != gamePool.end() )
   /*|*/ {
   /*|*/    // spwan a thread to manage the message and avoid locking the gamePool for nothing
   /*|*/    boost::thread worker( &AbstractGameProvider::handleGameMessage,
   /*|*/                          itGame->second,
   /*|*/                          message );
   /*|*/ }
   /*|*/ 
   // and release the lock
   gamePoolMutex.unlock();
}

// forward the message on the network
void AbstractProviderManager::sendMessage( const std::string& message )
{
   connection->sendMessage( message );
}

// dump the current state of the server
// should be call inside the mutex
void AbstractProviderManager::dumpCurrentState()
{
   std::stringstream stream;
   stream << std::endl;
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   stream << "games: " << gamePool.size() << " / " << getMaxGameInPool() << std::endl;
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   for ( GamePool::const_iterator it = gamePool.begin();
         it != gamePool.end();
         it++ )
   {
      stream << "\t" << it->first << std::endl;
   }
   stream << "----------------------------------------------------------------------------------------" << std::endl;
   AsyncLogger::getInstance()->log( stream.str() );
}
