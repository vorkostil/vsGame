#include "GraphGrid.hpp"
#include <iostream>

// ctor with the size of the grid
GraphGrid::GraphGrid( size_t width,
                      size_t height )
:
   Grid( width, 
         height ),
   entryPointX( 0 ),
   entryPointY( 0 )
{
}

// change the value at the given point
// overrided to store the entry point
void GraphGrid::setValueAt( size_t x,
                            size_t y,
                            int v )
{
   Grid::setValueAt( x, y, v );
   if ( v == ENTRY )
   {
      entryPointX = x;
      entryPointY = y;
   }
}

// do a DFS search on the graph
bool GraphGrid::computeDFS()
{
   if ( isValid() == true )
   {
      return internalComputeDFS( entryPointX, 
                                 entryPointY );
   }
   return false;
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
void GraphGrid::display( std::ostream& out ) const
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
            out << ' ';
         }
      }
      out << std::endl;
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
