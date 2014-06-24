#include "graphCell.hpp"
#include "Grid.hpp"
#include "common.hpp"

// ctor
GraphCell::GraphCell( size_t x,
                      size_t y )
:
   x( x ),
   y( y ),
   father( NULL ),
   weight( 0 ),
   value( 0 ),
   pathValue( GRASS_PATH_VALUE )
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

// get the x (and it doesn't mark the spot)
size_t GraphCell::getX() const
{
   return x;
}

// get the y
size_t GraphCell::getY() const
{
   return y;
}

// get the weight
double GraphCell::getWeight() const
{
   return weight;
}

// set the weight
void GraphCell::setWeight( double newWeight )
{
   this->weight = newWeight;
}

// get the euclidian distance to another point
double GraphCell::getEuclide( size_t targetX,
                              size_t targetY ) const
{
   int dx = targetX - x;
   int dy = targetY - y;

   return sqrt( (double)( ( dx * dx ) + ( dy * dy ) ) );
}

// get the manhattan distance to another point
double GraphCell::getManahattan( size_t targetX,
                                 size_t targetY ) const
{
   int dx = targetX - x;
   int dy = targetY - y;

   return abs( dx ) + abs( dy );
}

// get the manhattan distance to another point plus a little epsilon for the fun
double GraphCell::getManahattanEpsilon( size_t targetX,
                                        size_t targetY ) const
{
   int dx = targetX - x;
   int dy = targetY - y;

   return ( abs( dx ) + abs( dy ) ) * 1.0001;
}

// set the path value of the cell (different from the value which represent the kind of the cell (entry, exit ... )
// will have an impact when different type of search value will appears (road, plain, hill, mountain, water)
void GraphCell::setPathValue( int pathValue )
{
   this->pathValue = pathValue;
}

// get the search value (different from the value which represent the kind of the cell)
// will have an impact when different type of search value will appears (road, plain, hill, mountain, water)
int GraphCell::getPathValue() const
{
   return this->pathValue;
}
