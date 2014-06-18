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

   // the value
   int value;

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
};