#pragma once

#include "connection/ConnectionToServer.hpp"
#include "network/NetworkClient.hpp"

class GraphGrid;

class GraphGridProvider : public NetworkClient
{
   // the name of the provider
   static const std::string NAME;

   // the graph grid used
   GraphGrid* graphGrid;

   // the client connected to this graph
   ConnectionToServerPtr connection;

   // the login to store name + connection info
   std::string login;

public:
   // default ctor
   GraphGridProvider( ConnectionToServerPtr connection );

   // connect to the BBServer
   void connect( const std::string& host,
                 int port );

   // create a new graph withe the size
   void createGraph( size_t width,
                     size_t height );

   // modify a cell value
   void modifyCellValue( size_t x,
                         size_t y,
                         int value );

   // call a DFS computation
   void callDFS();

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

   // callback used to handle the message when logon
   virtual void onHandleMessage( const std::string& message );
};