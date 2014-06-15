#define _WIN32_WINNT 0x0501

#include "../ProviderManager.hpp"
#include "GraphGridProvider.hpp"
#include "GraphGrid.hpp"
#include "network/NetworkMessage.hpp"
#include "string/StringUtils.hpp"

const std::string GraphGridProvider::NAME( "GraphGame" );

static const std::string CHANGE_CELL_STATE( "CHANGE_CELL_STATE" );
static const std::string COMPUTE_DFS( "COMPUTE_DFS" );

// default ctor
GraphGridProvider::GraphGridProvider( size_t width,
                                      size_t height )
:
   manager( NULL )
{
   graphGrid = new GraphGrid( this,
                              width,
                              height );
}

// dtor
GraphGridProvider::~GraphGridProvider()
{
   delete graphGrid;
}


// modify a cell value
void GraphGridProvider::sendMessageChangeCell( size_t x,
                                               size_t y,
                                               const std::string& value )
{
   if ( manager != NULL )
   {
      char message[ 1024 ];
      sprintf_s( message,
                 1024,
                 "%s %s CELL_UPDATED %d %d %s",
                 GAME_MESSAGE.c_str(),
                 gameId.c_str(),
                 x,
                 y,
                 value.c_str() );

      manager->sendMessage( message );
   }
}

// call a DFS computation
void GraphGridProvider::callDFS()
{
   // initialize the buffer
   int size = 256 + ( graphGrid->getNumberOfCells() * 2 );
   char* message = new char( size );

   // begin the message creation
   sprintf_s( message,
              size,
              "%s %s PATH_RESULT",
              GAME_MESSAGE,
              gameId );

   // compute the result
   if ( graphGrid->computeDFS() == true )
   {
      sprintf_s( message,
                 size,
                 "%s OK",
                 message );
   }
   else
   {
      sprintf_s( message,
                 size,
                 "%s KO",
                 message );
   }

   // get the graph description
   std::stringstream graphStr;
   graphGrid->display( graphStr );

   // create the end of the message
   sprintf_s( message,
              size,
              "%s %d %d %s",
              message,
              graphGrid->getWidth(),
              graphGrid->getHeight(),
              graphStr.str() );

   // send the message if the manager is present
   if ( manager != NULL )
   {
      manager->sendMessage( message );
   }
}


// network communication management
//---------------------------------

// set the manager used to forward message on network
void GraphGridProvider::setNetworkInformation( ProviderManager* manager,
                                               const std::string& gameId )
{
   this->manager = manager;
   this->gameId = gameId;
}

// callback used to handle the message of game closure
void GraphGridProvider::close( const std::string& reason )
{
   std::cout << "Game " << gameId << " is close due to> " << reason << std::endl;
}

// call back for message managmeent
void GraphGridProvider::handleGameMessage( const std::string& message )
{
   // get the action and message information
   std::vector< std::string > messageParts;
   StringUtils::explode( message,
                         ' ',
                         messageParts );

   if ( messageParts[ 0 ] == CHANGE_CELL_STATE )
   {
      graphGrid->setValueAt( atoi( messageParts[ 1 ].c_str() ),
                             atoi( messageParts[ 2 ].c_str() ),
                             messageParts[ 3 ] );
   }
   else if ( messageParts[ 0 ] == COMPUTE_DFS )
   {
      callDFS();
   }
}
