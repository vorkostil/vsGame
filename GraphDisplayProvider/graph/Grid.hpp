#pragma once

#include <vector>
#include <list>

class Grid
{   
   // store the cells information
   std::vector< int > cells;

   // store the structure information
   size_t width;
   size_t height;

public:
   // ctor with the structure parameter
   // set all cells at 0
   Grid();
   
   // initialize the graph with the size of the grid
   // and the provider to send message
   void initialize( size_t width, 
                    size_t height );

   // the dtor
   virtual ~Grid();

   // change the value at the given point
   // return true if the value really change
   virtual bool setValueAt( size_t x,
                            size_t y,
                            int v );

   // get the value at given point
   virtual int getValueAt( size_t x,
                           size_t y ) const;

   // display the cells on the stream
   virtual void display( std::ostream& out,
                         bool flat = true ) const;

   // size accessor
   size_t getWidth() const;
   size_t getHeight() const;

protected:
   // return true if the point is in the structure
   bool isValid( size_t x,
                 size_t y ) const;

   // return the cells
   std::vector< int >& getCells();
};