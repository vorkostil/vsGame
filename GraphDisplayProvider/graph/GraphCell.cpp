#include "graphCell.hpp"
#include "Grid.hpp"

// ctor
GraphCell::GraphCell( size_t x,
                      size_t y )
:
   x( x ),
   y( y ),
   father( NULL )
{
}

// create the child
// validity check is done on them
void GraphCell::getChildren( Grid* grid,
                             std::vector< GraphCell* >& children) const
{
   // north
   if ( grid->isValid( x, y - 1 ) == true )
   {
      children.push_back( grid->getCellAt( x, y - 1 ) );
   }
   // east
   if ( grid->isValid( x + 1, y ) == true )
   {
      children.push_back( grid->getCellAt( x + 1, y ) );
   }
   // south
   if ( grid->isValid( x, y + 1 ) == true )
   {
      children.push_back( grid->getCellAt( x, y + 1 ) );
   }
   // west
   if ( grid->isValid( x - 1, y ) == true )
   {
      children.push_back( grid->getCellAt( x - 1, y ) );
   }
}

// get the father
GraphCell* GraphCell::getFather() const
{
   return this->father;
}

// get the value
int GraphCell::getValue() const
{
   return this->value;
}

// set the value
void GraphCell::setValue( int value )
{
   this->value = value;
}

// modify the value
void GraphCell::modifyValue( int delta )
{
   this->value += delta;
}

// reset the father value
void GraphCell::resetFather()
{
   this->father = NULL;
}

// reset the father value
void GraphCell::setFather( GraphCell* father )
{
   this->father = father;
}
