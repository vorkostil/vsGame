#define _WIN32_WINNT 0x0501

#include "MazeProviderManager.hpp"
#include "maze\MazeGameProvider.hpp"

static const std::string NAME( "MazeProviderManager" );
static const size_t MAX_GAME_IN_POOL = 1;

// ctor with the connection
MazeProviderManager::MazeProviderManager( ConnectionToServerPtr connection )
:
   AbstractProviderManager( connection )
{
}

// get the name of the providermanager instanciate for registration to the server
std::string MazeProviderManager::getName()
{
   return NAME;
}

// get the list of game providing format being NAME minPlayer MaxPlayer IA
std::string MazeProviderManager::getGameDefinitionForRegistration()
{
   return MazeGameProvider::DEFINITION;
}

// get the max number of game managed
size_t MazeProviderManager::getMaxGameInPool()
{
   return MAX_GAME_IN_POOL;
}

// return a game given its kind
AbstractGameProvider* MazeProviderManager::requireNewGame( const std::string& gameKind )
{
   return new MazeGameProvider();
}
