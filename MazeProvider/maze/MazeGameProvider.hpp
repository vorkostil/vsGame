#pragma once

#include "provider/AbstractGameProvider.hpp"

// provider for the graph display client
class MazeGameProvider : public AbstractGameProvider
{
public:
   // the name of the provider
   static const std::string NAME;

   // and its definition
   static const std::string DEFINITION;

   // default ctor
   MazeGameProvider();

   // dtor
   virtual ~MazeGameProvider();

   // abstract part
   //--------------

   // call back for message managmeent
   virtual void handleGameMessage( const std::string& message );

   // get the name of the provider
   virtual const std::string& getName();
};