#define _WIN32_WINNT 0x0501

#include "Game.hpp"
#include "network/NetworkMessage.hpp"

int Game::uniqueIdentifier = 0;

// create a game with its kind and its provider
Game::Game( const GameDefinition& gameDefinition,
            ClientConnectionPtr provider )
:
   gameDefinition( gameDefinition ),
   id( gameDefinition.kind ),
   provider( provider ),
   consumers()
{
   id += uniqueIdentifier++;
   provider->incLoad();
}

// dtor
Game::~Game()
{
   if ( provider != NULL )
   {
      provider->decLoad();
      provider = NULL;
   }
   consumers.clear();
}

// get the id of the game
const std::string& Game::getId() const
{
   return id;
}

// add consumer
void Game::addConsumer( ClientConnectionPtr consumer )
{
   if ( consumers.find( consumer ) == consumers.end() )
   {
      consumers.insert( consumer );
   }
}

// handle communication forward from P to C* or from C to S
void Game::handleMessage( ClientConnectionPtr connection,
                          const std::string& message )
{
   if ( connection == provider )
   {
      // forward to all clients
      for( ClientList::iterator itConsumer = consumers.begin();
           itConsumer != consumers.end();
           itConsumer++ )
      {
         (*itConsumer)->sendMessage( message );
      }
   }
   else
   {
      // sent the message to the provider
      provider->sendMessage( message );
   }
}

// return true if the game use the connection
bool Game::contains( ClientConnectionPtr connection ) const
{
   // check the provider
   if ( provider == connection )
   {
      return true;
   }
   
   // check the consumers
   for( ClientList::iterator itConsumer = consumers.begin();
         itConsumer != consumers.end();
         itConsumer++ )
   {
      if ( *itConsumer == connection )
      {
         return true;
      }
   }

   // none found
   return false;
}

// remove the connection from the game and return true if the connection was the provider
bool Game::remove( ClientConnectionPtr connection )
{
   if ( provider == connection )
   {
      // don't delete the provider as we aren't the owners, the connection manager is
      provider->decLoad();
      provider = NULL;
      return true;
   }
   
   // check the consumers
   for( ClientList::iterator itConsumer = consumers.begin();
         itConsumer != consumers.end();
         itConsumer++ )
   {
      if ( *itConsumer == connection )
      {
         consumers.erase( itConsumer );
         break;
      }
   }
   return false;
}

// close the game, ie send the close message to all consumers and to the provider
void Game::close()
{
   // close the provider if any 
   if ( provider != NULL )
   {
      provider->sendMessage( CREATE_MESSAGE( GAME_CLOSE_MESSAGE, id ) );
   }

   // close the consumers
   for( ClientList::iterator itConsumer = consumers.begin();
         itConsumer != consumers.end();
         itConsumer++ )
   {
      (*itConsumer)->sendMessage( CREATE_MESSAGE( GAME_CLOSE_MESSAGE, id ) );
   }
}

// return true if there is still some room for a player in the game
bool Game::placeAvailable() const
{
   return (  ( gameDefinition.maxPlayer == -1 )
           ||( consumers.size() < gameDefinition.maxPlayer )  );
}

#ifdef __DEBUG__
// return the provider
ClientConnectionPtr Game::getProvider() const
{
   return provider;
}

// return the clients
ClientList Game::getClients() const
{
   return consumers;
}
#endif
