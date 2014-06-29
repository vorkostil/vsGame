#define _WIN32_WINNT 0x0501

#include "AbstractProviderManager.hpp"
#include "AbstractGameProvider.hpp"
#include "logger/asyncLogger.hpp"

// default ctor
AbstractGameProvider::AbstractGameProvider()
:
   manager( NULL )
{
}

// dtor
AbstractGameProvider::~AbstractGameProvider()
{
}

// network communication management
//---------------------------------

// set the manager used to forward message on network
void AbstractGameProvider::setNetworkInformation( AbstractProviderManager* manager,
                                                  const std::string& gameId )
{
   this->manager = manager;
   this->gameId = gameId;
}

// callback used to handle the message of game closure
void AbstractGameProvider::close( const std::string& reason )
{
   AsyncLogger::getInstance()->log( "Game " + gameId + " is close due to> " + reason );
}
