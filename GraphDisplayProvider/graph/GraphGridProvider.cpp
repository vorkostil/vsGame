#define _WIN32_WINNT 0x0501

#include "GraphGridProvider.hpp"
#include "GraphGrid.hpp"
#include "network/NetworkMessage.hpp"
#include "provider/AbstractProviderManager.hpp"
#include "string/StringUtils.hpp"
#include "logger/asyncLogger.hpp"

const std::string GraphGridProvider::NAME( "GraphGame" );
const std::string GraphGridProvider::DEFINITION( NAME + " 1 1 0" );

static const std::string CHANGE_CELL_STATE( "CHANGE_CELL_STATE" );
static const std::string COMPUTE_DFS( "COMPUTE_DFS" );
static const std::string COMPUTE_BFS( "COMPUTE_BFS" );
static const std::string COMPUTE_DIJ( "COMPUTE_DIJ" );
static const std::string COMPUTE_ASTAR( "COMPUTE_ASTAR" );
static const std::string EUCLIDE( "EUCLIDE" );
static const std::string MANHATTAN( "MANHATTAN" );
static const std::string EPSILON( "EPSILON" );
static const std::string RESET_PATH( "RESET_PATH" );
static const std::string CLEAR_GRAPH( "CLEAR_GRAPH" );

// default ctor
GraphGridProvider::GraphGridProvider( size_t width,
                                      size_t height )
:
   AbstractGameProvider()
{
   graphGrid.initializeGraph( this,
                              width,
                              height );
}

// dtor
GraphGridProvider::~GraphGridProvider()
{
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
void GraphGridProvider::callComputation( ComputeFunctionPtr algorithm )
{
   // compute the DFS
   if ( algorithm( &graphGrid ) == true )
   {
      // and send the result
      sendComputeResult( "OK" );
   }
   else
   {
      // and send the result
      sendComputeResult( "KO" );
   }
}

// call the reset on the graph and send the result as a compute result
void GraphGridProvider::callReset()
{
   // call the reset
   graphGrid.reset();

   // and send the result
   sendComputeResult( "RESET" );
}

// call the clear all on the graph and send the result as a compute result
void GraphGridProvider::callClear()
{
   // call the reset
   graphGrid.clear();

   // and send the result
   sendComputeResult( "CLEAR" );
}

// create he compute result message given the result expected
// and send the message
void GraphGridProvider::sendComputeResult( const std::string& result )
{
   // get the graph description
   std::stringstream graphStr;
   graphGrid.display( graphStr );

   // send the message if the manager is present
   if ( manager != NULL )
   {
      //int size = 256 + ( graphGrid.getNumberOfCells() * 2 );
      //char* message = new char( size );
      char message[ 5000 ];
      sprintf_s( message,
                 5000,
                 "%s %s COMPUTE_RESULT %s %s",
                 GAME_MESSAGE.c_str(),
                 gameId.c_str(),
                 result.c_str(),
                 graphStr.str().c_str() );

      std::string messageToSend( message );
      manager->sendMessage( messageToSend );
   }
}

// call the DFS on the grid
bool computeDFS( GraphGrid* gg )
{
   return gg->computeDFS();
}

// call the BFS on the grid
bool computeBFS( GraphGrid* gg )
{
   return gg->computeBFS();
}

// call the DIJ on the grid
bool computeDIJ( GraphGrid* gg )
{
   return gg->computeDIJ();
}

// call the A* on the grid
bool computeAstar( GraphGrid* gg )
{
   return gg->computeAstar();
}

// call the A* manhattan on the grid
bool computeAstarM( GraphGrid* gg )
{
   return gg->computeAstarM();
}

// call the A* manhattan epsilon on the grid
bool computeAstarME( GraphGrid* gg )
{
   return gg->computeAstarME();
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
      graphGrid.setValueAt( atoi( messageParts[ 1 ].c_str() ),
                            atoi( messageParts[ 2 ].c_str() ),
                            messageParts[ 3 ] );
   }
   else if ( messageParts[ 0 ] == COMPUTE_DFS )
   {
      callComputation( &computeDFS );
   }
   else if ( messageParts[ 0 ] == COMPUTE_BFS )
   {
      callComputation( &computeBFS );
   }
   else if ( messageParts[ 0 ] == COMPUTE_DIJ )
   {
      callComputation( &computeDIJ );
   }
   else if ( messageParts[ 0 ] == COMPUTE_ASTAR )
   {
      if ( messageParts[ 1 ] == EUCLIDE )
      {
         callComputation( &computeAstar );
      }
      else if ( messageParts[ 1 ] == MANHATTAN )
      {
         callComputation( &computeAstarM );
      }
      else if ( messageParts[ 1 ] == EPSILON )
      {
         callComputation( &computeAstarME );
      }
   }
   else if ( messageParts[ 0 ] == RESET_PATH )
   {
      callReset();
   }
   else if ( messageParts[ 0 ] == CLEAR_GRAPH )
   {
      callClear();
   }
}

// get the name of the provider
const std::string& GraphGridProvider::getName()
{
   return NAME;
}