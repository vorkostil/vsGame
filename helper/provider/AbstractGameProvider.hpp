#pragma once

#include <string>

class AbstractProviderManager;

// provider for the graph display client
class AbstractGameProvider
{
protected:
   // the manager used to communicate on the network (not owned)
   AbstractProviderManager* manager;

   // the id used to identify the instance of the game on the network
   std::string gameId;

public:
   // default ctor
   AbstractGameProvider();

   // dtor
   virtual ~AbstractGameProvider();

   // network communication management
   //---------------------------------

   // set the manager used to forward message on network
   virtual void setNetworkInformation( AbstractProviderManager* manager,
                                       const std::string& gameId );

   // callback used to handle the message of game closure
   virtual void close( const std::string& reason );

   // call back for message managmeent
   virtual void handleGameMessage( const std::string& message ) = 0;

   // get the name of the provider
   virtual const std::string& getName() = 0;
};