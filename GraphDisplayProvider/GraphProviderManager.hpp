#pragma once

#include "provider/AbstractProviderManager.hpp"

class GraphProviderManager : public AbstractProviderManager
{
public:
   // ctor with the connection
   GraphProviderManager( ConnectionToServerPtr connection );

private:
   // get the name of the providermanager instanciate for registration to the server
   virtual std::string getName();

   // get the list of game providing format being NAME minPlayer MaxPlayer IA
   virtual std::string getGameDefinitionForRegistration();

   // get the max number of game managed
   virtual size_t getMaxGameInPool();

   // return a game given its kind
   virtual AbstractGameProvider* requireNewGame( const std::string& gameKind );
};