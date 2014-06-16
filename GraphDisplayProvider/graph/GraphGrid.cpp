#include "GraphGrid.hpp"
#include "GraphGridProvider.hpp"
#include <iostream>
#include <string>

static const std::string EMPTY_STR( "EMPTY" );
static const std::string BLOCK_STR( "BLOCK" );
static const std::string START_STR( "START" );
static const std::string EXIT_STR( "EXIT" );
static const std::string VISITED_STR( "VISITED" );
static const std::string PATH_STR( "PATH" );

static const std::string& cellToString( int value )
{
   if ( value == GraphGrid::PATH )
   {
      return PATH_STR;
   }
   else if ( value == GraphGrid::VISITED )
   {
      return VISITED_STR;
   }
   else if ( value == GraphGrid::ENTRY )
   {
      return START_STR;
   }
   else if ( value == GraphGrid::EXIT )
   {
      return EXIT_STR;
   }
   else if ( value == GraphGrid::BLOCK )
   {
      return BLOCK_STR;
   }
   return EMPTY_STR;
}

// ctor
GraphGrid::GraphGrid()
:
   Grid(),
   provider( NULL ),
   entryPointX( 0 ),
   entryPointY( 0 ),
   blockMessage( false )
{
}

// initialize the graph with the size of the grid
// and the provider to send message
void GraphGrid::initializeGraph( GraphGridProvider* provider,
                                 size_t width,
                                 size_t height )
{
   // create the grid
   Grid::initialize( width,
                     height );

   // set the entry point @0,0
   Grid::setValueAt( entryPointX,
                     entryPointY,
                     ENTRY );

   // and initialize the provider
   this->provider = provider;
}

// change the value at the given point
// overrided to store the entry point
bool GraphGrid::setValueAt( size_t x,
                            size_t y,
                            int v )
{
   if ( v == ENTRY )
   {
      Grid::setValueAt( entryPointX,
                        entryPointY,
                        EMPTY );

      if ( blockMessage == false )
      {
         provider->sendMessageChangeCell( entryPointX,
                                          entryPointY,
                                          EMPTY_STR );
      }

      entryPointX = x;
      entryPointY = y;
   }

   if ( Grid::setValueAt( x, y, v ) == true )
   {
      if ( blockMessage == false )
      {
         provider->sendMessageChangeCell( x,
                                          y,
                                          cellToString( v ) );
      }
      return true;
   }
   return false;
}

// change the value at the given point
// overrided to store the entry point
// return true if the cell is really modified
bool GraphGrid::setValueAt( size_t x,
                            size_t y,
                            const std::string& value )
{
   int v = EMPTY;
   if (  ( value == BLOCK_STR )
       &&( getValueAt( x,
                       y ) != BLOCK )  )
   { 
      v = BLOCK;
   }
   else if ( value == START_STR )
   {
      v = ENTRY;
   }
   else if (  ( value == EXIT_STR )
            &&( getValueAt( x,
                            y ) != EXIT )  )
   {
      v = EXIT;
   }

   return setValueAt( x, y, v );
}

// do a DFS search on the graph
bool GraphGrid::computeDFS()
{
   // block message sending
   blockMessage = true;

   // prepare the result
   bool result = false;

   // check the graph computation visibility
   if ( isValid() == true )
   {
      // compute the DFS
      result = internalComputeDFS( entryPointX, 
                                   entryPointY );

      // put back the entryPoint to ENTRY (instead of PATH)
      // TODO manage to exclude the Entry from the Path
      setValueAt( entryPointX,
                  entryPointY,
                  ENTRY );
   }

   // allow message sending bakc to normal
   blockMessage = false;

   // rturn the computation result
   return result;
}

// do a recursive DFS search on the graph given the currentPoint
bool GraphGrid::internalComputeDFS( size_t x,
                                    size_t y )
{
   // check the end of the recursion
   int currentValue;
   if ( ( currentValue = getValueAt( x, y ) ) == EXIT )
   {
      // we find the exit
      return true;
   }
   else if (  ( ( currentValue & BLOCK ) == BLOCK )
            ||( ( currentValue & VISITED ) == VISITED )
            ||( currentValue == -1 )  )
   {
      // we find an obstacle or an invalid cell
      return false;
   }

   // set the visited flag
   setValueAt( x, y, currentValue + VISITED );

   // compute the E child
   if ( internalComputeDFS( x + 1, y ) == true )
   {
      // set the path flag
      setValueAt( x, y, currentValue + PATH );

      // and return as a path
      return true;
   }

   // compute the S child
   if ( internalComputeDFS( x, y + 1 ) == true )
   {
      // set the path flag
      setValueAt( x, y, currentValue + PATH );

      // and return as a path
      return true;
   }

   // compute the W child
   if ( internalComputeDFS( x - 1, y ) == true )
   {
      // set the path flag
      setValueAt( x, y, currentValue + PATH );

      // and return as a path
      return true;
   }

   // compute the N child
   if ( internalComputeDFS( x, y - 1 ) == true )
   {
      // set the path flag
      setValueAt( x, y, currentValue + PATH );

      // and return as a path
      return true;
   }

   // no path found on this exploration choice
   return false;
}

// check the grpah validity before computation
// at least one EXIT
bool GraphGrid::isValid()
{
   for ( std::vector< int >::const_iterator it = getCells().begin();
         it != getCells().end();
         it++ )
   {
      if ( *it == EXIT )
      {
         return true;
      }
   }
   return false;
}

// display the cells on the stream
void GraphGrid::display( std::ostream& out,
                         bool flat ) const
{
   for( size_t y = 0;
        y < getHeight();
        ++y )
   {
      for( size_t x = 0;
           x < getWidth();
           ++x )
      {
         int currentValue = getValueAt( x, y );
         if ( ( currentValue & PATH ) == PATH )
         {
            out << 'P';
         }
         else if ( ( currentValue & VISITED ) == VISITED )
         {
            out << 'V';
         }
         else if ( ( currentValue & ENTRY ) == ENTRY )
         {
            out << 'S';
         }
         else if ( ( currentValue & EXIT ) == EXIT )
         {
            out << 'E';
         }
         else if ( ( currentValue & BLOCK ) == BLOCK )
         {
            out << 'X';
         }
         else 
         {
            out << '0';
         }
      }

      if ( flat == false )
      {
         out << std::endl;
      }
   }
}

// reset the graph information to remove the VISITED and PATH informatio
void GraphGrid::reset()
{
   for ( std::vector< int >::iterator it = getCells().begin();
         it != getCells().end();
         it++ )
   {
      if ( ( *it & VISITED ) == VISITED )
      {
         *it -= VISITED;
      }
      else if ( ( *it & PATH ) == PATH )
      {
         *it -= PATH;
      }
   }
}

// get the number of cell
size_t GraphGrid::getNumberOfCells() const
{
   return getWidth() * getHeight();
}
