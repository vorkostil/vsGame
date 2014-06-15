#include "Grid.hpp"
#include <iostream>

Grid::Grid( size_t width,
            size_t height )
:
   width( width ),
   height( height ),
   cells( width * height )
{
}

Grid::~Grid()
{
   cells.clear();
}

bool Grid::setValueAt( size_t x,
                       size_t y,
                       int v )
{
   if (  ( isValid( x, y ) == true )
       &&( cells[ y * width + x ] != v )  )
   {
      cells[ y * width + x ] = v;
      return true;
   }
   return false;
}

int Grid::getValueAt( size_t x,
                      size_t y ) const
{
   if ( isValid( x, y ) == true )
   {
      return cells[ y * width + x ];
   }
   return -1;
}

void Grid::display( std::ostream& out ) const
{
   for( size_t y = 0;
        y < height;
        ++y )
   {
      for( size_t x = 0;
           x < width;
           ++x )
      {
         out << getValueAt( x, y );
         if ( x < width - 1 )
         {
            out << " ";
         }
      }
      out << std::endl;
   }
}

bool Grid::isValid( size_t x,
                    size_t y ) const
{
   return (  ( x < width )
           &&( y < height )  );
}

std::vector< int >& Grid::getCells()
{
   return cells;
}

size_t Grid::getWidth() const
{
   return width;
}

size_t Grid::getHeight() const
{
   return height;
}
