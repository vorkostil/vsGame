#pragma once

#include "../../BackBoneServer/ClientConnection.hpp"

class GraphGrid;

class GraphGridServer
{
   // the graph grid used
   GraphGrid* graphGrid;

   // the client connected to this graph
   ClientConnection::ClientConnectionPtr client;

public:
   // default ctor
   GraphGridServer( ClientConnection::ClientConnectionPtr client );

   // create a new graph withe the size
   void createGraph( size_t width,
                     size_t height );

   // modify a cell value
   void modifyCellValue( size_t x,
                         size_t y,
                         int value );

   // call a DFS computation
   void callDFS();

};