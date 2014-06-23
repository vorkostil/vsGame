#pragma once

#include <vector>

class Grid;
class GraphCell
{
   // the coordinate of the cell
   size_t x;
   size_t y;

   // the father, default is NULL
   GraphCell* father;

   // the value which represent the kind of the cell
   // entry, exit, visited, ...
   int value;

   // the path value of the cell 
   // road, plain, mountain, ...
   int pathValue;

   // the weight for dijsktra and A* algorithm
   double weight;

public:
   // ctor
   GraphCell( size_t x,
              size_t y );

   // create the child
   // validity check is done on them
   void getChildren( Grid* grid,
                     std::vector< GraphCell* >& children ) const;

   // get the father
   GraphCell* getFather() const;

   // get the value
   int getValue() const;

   // set the value
   void setValue( int value );

   // modify the value
   void modifyValue( int delta );

   // reset the father value
   void resetFather();

   // reset the father value
   void setFather( GraphCell* father );

   // get the x (and it doesn't mark the spot)
   size_t getX() const;

   // get the y
   size_t getY() const;

   // get the weight
   double getWeight() const;

   // set the weight
   void setWeight( double newWeight );

   // get the euclidian distance to another point
   double getEuclide( size_t targetX,
                      size_t targetY ) const;

   // get the manhattan distance to another point
   double getManahattan( size_t targetX,
                         size_t targetY ) const;

   // get the manhattan distance to another point plus a little epsilon for the fun
   double getManahattanEpsilon( size_t targetX,
                                size_t targetY ) const;

   // set the path value of the cell (different from the value which represent the kind of the cell (entry, exit ... )
   // will have an impact when different type of search value will appears (road, plain, hill, mountain, water)
   void setPathValue( int pathValue );

   // get the search value (different from the value which represent the kind of the cell)
   // will have an impact when different type of search value will appears (road, plain, hill, mountain, water)
   int getPathValue() const;
};