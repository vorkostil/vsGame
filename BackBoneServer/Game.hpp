#pragma once

#include "ClientConnection.hpp"
#include "GameDefinition.hpp"

// a game representation from the server PoV
class Game
{
   static int uniqueIdentifier;

   // the identifier
   std::string id;

   // the game kind
   GameDefinition gameDefinition;

   // the game provider
   ClientConnectionPtr provider;

   // the game consumers
   ClientList consumers;

public:
   // create a game with its kind and its provider
   Game( const GameDefinition& gameDefinition,
         ClientConnectionPtr provider );

   // dtor
   ~Game();

   // get the id of the game
   const std::string& getId() const;

   // add consumer
   void addConsumer( ClientConnectionPtr consumer );

   // handle communication forward from P to C* or from C to S
   void handleMessage( ClientConnectionPtr connection,
                       const std::string& message );

   // return true if the game use the connection
   bool contains( ClientConnectionPtr connection ) const;

   // remove the connection from the game and return true if the connection was the provider
   bool remove( ClientConnectionPtr connection );

   // close the game, ie send the close message to all consumers and to the provider
   void close();

   // return true if there is still some room for a player in the game
   bool placeAvailable() const;

#ifdef __DEBUG__
   // return the provider
   ClientConnectionPtr getProvider() const;

   // return the clients
   ClientList getClients() const;
#endif
};