#pragma once

#include "network/client/ConnectionToServer.hpp"
#include "network/client/NetworkClient.hpp"
#include "boost/thread/mutex.hpp"

class AbstractGameProvider;
class AbstractProviderManager : public NetworkClient
{
   // the maximum of available game at a given time
   static const int MAX_GAME_POOL_SIZE;

   // the client connected to this graph
   ConnectionToServerPtr connection;

   // the login to store name + connection info
   std::string login;

   // the game storer
   typedef std::map< std::string, AbstractGameProvider* > GamePool;
   GamePool gamePool;

   // the mutex of the game storer
   // allow many reader and 1 writer
   boost::mutex gamePoolMutex;

public:

   // ctor with the connection
   AbstractProviderManager( ConnectionToServerPtr connection );

   // connect to the BBServer
   void connect( const std::string& host,
                 int port );

   // forward the message on the network
   void sendMessage( const std::string& message );

private:
   // dump the current state of the manager
   // should be call inside the mutex
   void dumpCurrentState();

   // coming from Network Client
   //---------------------------

   // get the login
   virtual const std::string& getLogin() const;

   // get the passwd
   virtual const std::string& getPassword() const;

   // callback when the connection is accepted
   virtual void onConnection();

   // call back when the login procotol succeed
   virtual void onLoginSucced();

   // call back when a game creation message is received
   virtual void onNewGameCreation( const std::string& gameId,
                                   const std::string& gameKind );

   // callback used to handle the message of game closure
   // gameIds separator is |
   virtual void onGameClose( const std::string& gameIds,
                             const std::string& reason );

   // callback used to handle the message when logon
   virtual void onHandleMessage( const std::string& gameId,
                                 const std::string& message );

   // the abstract part
   //------------------

   // get the name of the providermanager instanciate for registration to the server
   virtual std::string getName() = 0;

   // get the list of game providing format being NAME minPlayer MaxPlayer IA
   virtual std::string getGameDefinitionForRegistration() = 0;

   // get the max number of game managed
   virtual size_t getMaxGameInPool() = 0;

   // return a game given its kind
   virtual AbstractGameProvider* requireNewGame( const std::string& gameKind ) = 0;
};