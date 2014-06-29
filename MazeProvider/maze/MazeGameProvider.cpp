#define _WIN32_WINNT 0x0501

#include "MazeGameProvider.hpp"
#include "provider/AbstractProviderManager.hpp"
//#include "string/StringUtils.hpp"
//#include "logger/asyncLogger.hpp"

const std::string MazeGameProvider::NAME( "MazeGame" );
const std::string MazeGameProvider::DEFINITION( NAME + " 1 1 0" );

// default ctor
MazeGameProvider::MazeGameProvider()
:
   AbstractGameProvider()
{
}

// dtor
MazeGameProvider::~MazeGameProvider()
{
}

// call back for message managmeent
void MazeGameProvider::handleGameMessage( const std::string& message )
{
   // get the action and message information
   //std::vector< std::string > messageParts;
   //StringUtils::explode( message,
   //                      ' ',
   //                      messageParts );
}

// get the name of the provider
const std::string& MazeGameProvider::getName()
{
   return NAME;
}