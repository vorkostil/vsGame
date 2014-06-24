#pragma once

#include "Grid.hpp"

class GraphGridProvider;

class GraphGrid : public Grid
{
   // the provider to send message
   GraphGridProvider* provider;

   // the entry point, default position is 0,0
   size_t entryPointX;
   size_t entryPointY;

   // prevent sending message during graph computation
   bool blockMessage;

   // the exit point for A* computation
   // value is set by the validForAstar method
   size_t exitPointX;
   size_t exitPointY;

public:
   // cells content
   typedef enum
   {
      GRASS   = 0,
      ENTRY   = 1,
      EXIT    = 2,
      BLOCK   = 4,
      WATER   = 8,
      ROAD    = 16,
      FOREST  = 32,
      HILL    = 64,
      VISITED = 128,
      PATH    = 256
   } Content;

   // ctor 
   GraphGrid();

   // initialize the graph with the size of the grid
   // and the provider to send message
   void initializeGraph( GraphGridProvider* provider,
                         size_t width,
                         size_t height );

   // change the value at the given point
   // overrided to store the entry point
   // return true if the cell is really modified
   virtual bool setValueAt( size_t x,
                            size_t y,
                            int v );

   // change the value at the given point
   // overrided to store the entry point
   // return true if the cell is really modified
   virtual bool setValueAt( size_t x,
                            size_t y,
                            const std::string& value );

   // do a DFS search on the graph
   bool computeDFS();

   // call the BFS on the grid
   bool computeBFS();

   // call the DIJ on the grid
   bool computeDIJ();

   // call the A* on the grid
   bool computeAstar();

   // call the A* manhattan on the grid
   bool computeAstarM();

   // call the A* manhattan epsilon on the grid
   bool computeAstarME();

   // display the cells on the stream
   virtual void display( std::ostream& out,
                         bool flat = true ) const;

   // reset the graph information to remove the VISITED and PATH information
   void reset();

   // clear the graph information to remove all information and set the entry back to 0,0
   void clear();

protected:
   // check the grpah validity before computation
   // only one ENTRY, at least one EXIT
   bool isValid();

   // check the grpah validity before A* computation
   // only one EXIT
   bool isValidForAstar();

   // do a recursive DFS search on the graph given the currentPoint
   bool internalComputeDFS( size_t x,
                            size_t y );
};