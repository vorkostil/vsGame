#pragma once

#include "GraphGrid.hpp"

class ProviderManager;

// provider for the graph display client
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

   // network communication management
   //---------------------------------

   // set the manager used to forward message on network
   void setNetworkInformation( ProviderManager* manager,
                               const std::string& gameId );

   // callback used to handle the message of game closure
   void close( const std::string& reason );

   // call back for message managmeent
   void handleGameMessage( const std::string& message );

private:
   // functor typepef for the computation call
   typedef bool (*ComputeFunctionPtr)( GraphGrid* );

   // call a computation given the algorithm
   void callComputation( ComputeFunctionPtr algorithm );

   // call the reset on the graph and send the result as a compute result
   void callReset();

   // create he compute result message given the result expected
   // and send the message
   void sendComputeResult( const std::string& result );

};