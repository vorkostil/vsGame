#pragma once

#include <string>
#include <list>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

class AsyncLogger
{
   // the logger mutex to prevent slicing the log
   boost::mutex loggerMutex;

   // the singleton logger
   static AsyncLogger* logger;

   // the facet for output
   boost::posix_time::time_facet* facet;

   // the copy ctor
   AsyncLogger( AsyncLogger& )
   {
      throw std::exception( "should never happend" );
   }

   // the default ctor
   AsyncLogger()
   {
      facet = new boost::posix_time::time_facet("%d-%b-%Y %H:%M:%S.%f");
      std::cout.imbue( std::locale( std::cout.getloc(), 
                                    facet) );
   }

   // the async logger (somehow)
   void run()
   {
#ifdef __DEBUG__
      while( true )
      {
         if ( buffer.size() > 0 )
         {
            loggerMutex.lock();
            /*|*/ // get the current time
            /*|*/ boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
            /*|*/ 
            /*|*/ // log the buffer
            /*|*/ for ( std::list< std::string >::const_iterator it = buffer.begin();
            /*|*/       it != buffer.end();
            /*|*/       it++ )
            /*|*/ {
            /*|*/    std::cout << "[" << now << "] " << *it << std::endl;
            /*|*/ }
            /*|*/ 
            /*|*/ // clear the buffer
            /*|*/ buffer.clear();
            loggerMutex.unlock();
         }

         boost::this_thread::sleep_for( boost::chrono::milliseconds( 4 ) );
      }
#endif
   }

   // the writing buffer 
   std::list< std::string > buffer;

public:
   static AsyncLogger* getInstance()
   {
      if ( logger == NULL )
      {
         logger = new AsyncLogger();

         boost::thread worker( &AsyncLogger::run,
                               logger );
      }

      return logger;
   }

   void log( const std::string& line )
   {
#ifdef __DEBUG__
      loggerMutex.lock();
      /*|*/ buffer.push_back( line );
      loggerMutex.unlock();
#endif
   }
};
