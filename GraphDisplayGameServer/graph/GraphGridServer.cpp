#include "GraphGridServer.hpp"
#include "GraphGrid.hpp"

// default ctor
GraphGridServer::GraphGridServer( ClientConnection::ClientConnectionPtr client )
:
   graphGrid( NULL ),
   client( client )
{
}

// create a new graph withe the size
void GraphGridServer::createGraph( size_t width,
                                   size_t height )
{
   if ( graphGrid != NULL )
   {
      delete graphGrid;
   }

   graphGrid = new GraphGrid( width,
                              height );
}

// modify a cell value
void GraphGridServer::modifyCellValue( size_t x,
                                       size_t y,
                                       int value )
{
   if ( graphGrid == NULL )
   {
      client->sendMessage( "ERROR No graph define" );
   }
   else
   {
      graphGrid->setValueAt( x, 
                             y,
                             value );

      char message[ 1024 ];
      sprintf( message,
               "CELL_UPDATED %d %d %d",
               x,
               y,
               value );

      client->sendMessage( message );
   }
}

// call a DFS computation
void GraphGridServer::callDFS()
{
   if ( graphGrid == NULL )
   {
      client->sendMessage( "ERROR No graph define" );
   }
   else
   {
      int size = graphGrid->getNumberOfCells();
      char* message = new char( 256 + size * 2 );

      if ( graphGrid->computeDFS() == true )
      {
         sprintf( message,
                  "PATH_COMPUTED OK %d ",
                  size );
      }
      else
      {
         sprintf( message,
                  "PATH_COMPUTED KO %d ",
                  size );
      }

      std::stringstream graphStr;

      graphGrid->display( graphStr );
      sprintf( message,
               "%s %s",
               message,
               graphStr.str() );

      client->sendMessage( message );
   }
}
