#pragma once
#pragma warning( disable : 4018 )
#include <vector>

class StringUtils
{
public:
   // explode the str using needle and store the exploded parts in the out vector
   // return the size of the vector (ie. 1 means not exploded)
   // the exploded vector will have a size of maxNumberOfPart (if it differs from -1)
   static size_t explode( const std::string& str,
                          char needle,
                          std::vector< std::string >& out,
                          int maxNumberOfPart = -1 )
   {
      int charPos = 0;
      int lastPos = 0;
      while(  (  ( maxNumberOfPart == -1 )
               ||( maxNumberOfPart > out.size() + 1 )  )
            &&( charPos = str.find( needle,
                                   lastPos ) ) != std::string::npos )
      {
         out.push_back( str.substr( lastPos, 
                                    charPos - lastPos ) );
         lastPos = charPos + 1;
      }

      out.push_back( str.substr( lastPos,
                                 str.size() - lastPos ) );

      return out.size();
   }
};