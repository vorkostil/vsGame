#include "Grid.hpp"
#include "graphCell.hpp"
#include <iostream>

Grid::Grid()
:
   width( 0 ),
   height( 0 ),
   cells()
{
}

// initialize the graph with the size of the grid
// and the provider to send message
void Grid::initialize( size_t width, 
                       size_t height,
                       int defaultValue /* = 0 */ )
{
   this->width = width;
   this->height = height;
   this->cells.resize( this->width * this->height );

   for ( size_t y = 0;
         y < height;
         ++y )
   {
      for ( size_t x = 0;
            x < width;
            ++x )
      {
         GraphCell* cell = new GraphCell( x, y );
         cell->setValue( defaultValue );

         this->cells[ y * width + x ] = cell;
      }
   }
}

Grid::~Grid()
{
}

bool Grid::setValueAt( size_t x,
                       size_t y,
                       int v )
{
   if (  ( isValid( x, y ) == true )
      &&( cells[ y * width + x ]->getValue() != v )  )
   {
      cells[ y * width + x ]->setValue( v );
      return true;
   }
   return false;
}

int Grid::getValueAt( size_t x,
                      size_t y ) const
{
   if ( isValid( x, y ) == true )
   {
      return cells[ y * width + x ]->getValue();
   }
   return -1;
}

GraphCell* Grid::getCellAt( size_t x,
                            size_t y ) const
{
   return cells[ y * width + x ];
}

void Grid::display( std::ostream& out,
                    bool flat ) const
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
      if ( flat == false )
      {
         out << std::endl;
      }
   }
}

bool Grid::isValid( size_t x,
                    size_t y ) const
{
   return (  ( x < width )
           &&( y < height )  );
}

std::vector< GraphCell* >& Grid::getCells()
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
