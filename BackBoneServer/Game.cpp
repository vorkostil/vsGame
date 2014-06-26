#define _WIN32_WINNT 0x0501

#include "Game.hpp"
#include "network/NetworkMessage.hpp"

// unique identifier creation
static size_t uniqueIdentifier = 0;
static std::string createUniqueId( const std::string& name )
{
   char tmp[1024];
   sprintf_s( tmp,
              1024,
              "%s_%d",
              name.c_str(),
              uniqueIdentifier++ );
   return std::string( tmp );
}

// create a game with its kind and its provider
Game::Game( const GameDefinition& gameDefinition,
            ClientConnectionPtr provider )
:
   gameDefinition( gameDefinition ),
   id( createUniqueId( gameDefinition.kind ) ),
   provider( provider ),
   consumers()
{
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

      // send the add consumer message to the provider
      provider->sendMessage( GAME_MESSAGE + " " + id + " " + PLAYER_JOIN_MESSAGE + " " + consumer->getLogin() );
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

         // send the add consumer message to the provider
         provider->sendMessage( GAME_MESSAGE + " " + id + " " + PLAYER_LEAVE_MESSAGE + " " + connection->getLogin() );

         break;
      }
   }
   return false;
}

// close the game, ie send the close message to all consumers and to the provider
void Game::close( const std::string& reason )
{
   // close the provider if any 
   if ( provider != NULL )
   {
      provider->sendMessage( GAME_MESSAGE + " " + CLOSE_MESSAGE + " " + id + " " + reason );
   }

   // close the consumers
   for( ClientList::iterator itConsumer = consumers.begin();
         itConsumer != consumers.end();
         itConsumer++ )
   {
      (*itConsumer)->sendMessage( GAME_MESSAGE + " " + CLOSE_MESSAGE + " " + id + " " + reason );
   }
}

// return true if there is still some room for a player in the game
bool Game::placeAvailable() const
{
   return (  ( gameDefinition.maxPlayer == -1 )
           ||( consumers.size() < gameDefinition.maxPlayer )  );
}

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

// return the kind of the game
const std::string& Game::getKind() const
{
   return gameDefinition.kind;
}
