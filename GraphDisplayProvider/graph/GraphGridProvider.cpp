#define _WIN32_WINNT 0x0501

#include <boost/asio.hpp>
#include "GraphGridProvider.hpp"
#include "GraphGrid.hpp"
#include "network/NetworkMessage.hpp"

const std::string GraphGridProvider::NAME( "GraphGridProvider" );

// default ctor
GraphGridProvider::GraphGridProvider( ConnectionToServerPtr connection )
:
   graphGrid( NULL ),
   connection( connection )
{
   connection->setNetworkClient( this );
}

// connect to the BBServer
void GraphGridProvider::connect( const std::string& host,
                                 int port )
{
   connection->connect( boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string( host ),
                                                        port ) );
}

// create a new graph withe the size
void GraphGridProvider::createGraph( size_t width,
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
void GraphGridProvider::modifyCellValue( size_t x,
                                         size_t y,
                                         int value )
{
   if ( graphGrid == NULL )
   {
      connection->sendMessage( "ERROR No graph define" );
   }
   else
   {
      graphGrid->setValueAt( x, 
                             y,
                             value );

      char message[ 1024 ];
      sprintf_s( message,
                 1024,
                 "CELL_UPDATED %d %d %d",
                 x,
                 y,
                 value );

      connection->sendMessage( message );
   }
}

// call a DFS computation
void GraphGridProvider::callDFS()
{
   if ( graphGrid == NULL )
   {
      connection->sendMessage( "ERROR No graph define" );
   }
   else
   {
      int size = 256 + ( graphGrid->getNumberOfCells() * 2 );
      char* message = new char( size );

      if ( graphGrid->computeDFS() == true )
      {
         sprintf_s( message,
                    size,
                    "PATH_COMPUTED OK %d ",
                    graphGrid->getNumberOfCells() );
      }
      else
      {
         sprintf_s( message,
                    size,
                    "PATH_COMPUTED KO %d ",
                    graphGrid->getNumberOfCells() );
      }

      std::stringstream graphStr;

      graphGrid->display( graphStr );
      sprintf_s( message,
                 size,
                 "%s %s",
                 message,
                 graphStr.str() );

      connection->sendMessage( message );
   }
}

// get the login
const std::string& GraphGridProvider::getLogin() const
{
   return NAME;
}

// get the passwd
const std::string& GraphGridProvider::getPassword() const
{
   return NAME;
}

// call back when the login procotol succeed
void GraphGridProvider::onLoginSucced()
{
   // register as provider
   // [GameName MinPlayer MaxPlayer IAAvailable]
   connection->sendMessage( SYSTEM_REGISTER + " " + PROVIDER_PART + " " + NAME + " 1 1 0" );
}

// callback used to handle the message when logon
void GraphGridProvider::onHandleMessage( const std::string& message )
{
   // TODO
}
