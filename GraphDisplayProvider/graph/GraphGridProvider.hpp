#pragma once

#include "GraphGrid.hpp"

class ProviderManager;

class GraphGridProvider
{
   // the graph grid used (owned)
   GraphGrid graphGrid;

   // the manager used to communicate on the network (not owned)
   ProviderManager* manager;

   // the id used to identify the instance of the game on the network
   std::string gameId;

public:
   // the name of the provider
   static const std::string NAME;

   // default ctor
   GraphGridProvider( size_t width,
                      size_t height );

   // dtor
   virtual ~GraphGridProvider();

   // modify a cell value
   void sendMessageChangeCell( size_t x,
                               size_t y,
                               const std::string& value );

   // call a DFS computation
   void callDFS();

   // network communication management
   //---------------------------------

   // set the manager used to forward message on network
   void setNetworkInformation( ProviderManager* manager,
                               const std::string& gameId );

   // callback used to handle the message of game closure
   void close( const std::string& reason );

   // call back for message managmeent
   void handleGameMessage( const std::string& message );
};