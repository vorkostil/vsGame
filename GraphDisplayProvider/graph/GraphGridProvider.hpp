#pragma once

#include "GraphGrid.hpp"
#include "provider/AbstractGameProvider.hpp"

// provider for the graph display client
class GraphGridProvider : public AbstractGameProvider
{
   // the graph grid used (owned)
   GraphGrid graphGrid;

public:
   // the name of the provider
   static const std::string NAME;

   // and its definition
   static const std::string DEFINITION;

   // default ctor
   GraphGridProvider( size_t width,
                      size_t height );

   // dtor
   virtual ~GraphGridProvider();

   // modify a cell value
   void sendMessageChangeCell( size_t x,
                               size_t y,
                               const std::string& value );

   // the abstract part
   //------------------

   // call back for message managmeent
   virtual void handleGameMessage( const std::string& message );

   // get the name of the provider
   virtual const std::string& getName();

private:
   // functor typepef for the computation call
   typedef bool (*ComputeFunctionPtr)( GraphGrid* );

   // call a computation given the algorithm
   void callComputation( ComputeFunctionPtr algorithm );

   // call the reset on the graph and send the result as a compute result
   void callReset();

   // call the clear all on the graph and send the result as a compute result
   void callClear();

   // create he compute result message given the result expected
   // and send the message
   void sendComputeResult( const std::string& result );

};