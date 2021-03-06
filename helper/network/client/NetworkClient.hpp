#pragma once

#include <string>

// this class is the abstraction of a client on the network
class NetworkClient
{
public:

   // get the login
   virtual const std::string& getLogin() const = 0;

   // get the passwd
   virtual const std::string& getPassword() const = 0;

   // callback when the connection is accepted
   virtual void onConnection() = 0;

   // call back when the login procotol succeed
   virtual void onLoginSucced() = 0;

   // call back when a game creation message is received
   virtual void onNewGameCreation( const std::string& gameId,
                                   const std::string& gameKind ) = 0;

   // callback used to handle the message of game closure
   virtual void onGameClose( const std::string& gameId,
                             const std::string& reason ) = 0;

   // callback used to handle the message when logon
   virtual void onHandleMessage( const std::string& gameId,
                                 const std::string& message ) = 0;
};