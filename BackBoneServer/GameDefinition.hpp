#pragma once

#include <map>

struct GameDefinition
{
   std::string kind;
   size_t minPlayer;
   size_t maxPlayer;
   bool iaAvailable;

   GameDefinition( const std::string& kind,
                   int minPlayer,
                   int maxPlayer,
                   int iaAvailable )
   :
      kind( kind ),
      minPlayer( minPlayer ),
      maxPlayer( maxPlayer ),
      iaAvailable( iaAvailable != 0 )
   {
   }
};

// useable typdef for ease the coding
typedef std::map< std::string, GameDefinition > GameDefinitionMap;