#define _WIN32_WINNT 0x0501

#include "GraphProviderManager.hpp"
#include "graph\GraphGridProvider.hpp"

static const std::string NAME( "GraphProviderManager" );
static const size_t MAX_GAME_IN_POOL = 4;

// ctor with the connection
GraphProviderManager::GraphProviderManager( ConnectionToServerPtr connection )
:
   AbstractProviderManager( connection )
{
}

// get the name of the providermanager instanciate for registration to the server
std::string GraphProviderManager::getName()
{
   return NAME;
}

// get the list of game providing format being NAME minPlayer MaxPlayer IA
std::string GraphProviderManager::getGameDefinitionForRegistration()
{
   return GraphGridProvider::DEFINITION;
}

// get the max number of game managed
size_t GraphProviderManager::getMaxGameInPool()
{
   return MAX_GAME_IN_POOL;
}

// return a game given its kind
AbstractGameProvider* GraphProviderManager::requireNewGame( const std::string& gameKind )
{
   return new GraphGridProvider( 32, 32 );
}
