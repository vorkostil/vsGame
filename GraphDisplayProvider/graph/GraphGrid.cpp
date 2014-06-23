#include "GraphGrid.hpp"
#include "GraphGridProvider.hpp"
#include "GraphCell.hpp"
#include <iostream>
#include <string>

static const std::string EMPTY_STR( "EMPTY" );
static const std::string BLOCK_STR( "BLOCK" );
static const std::string START_STR( "START" );
static const std::string EXIT_STR( "EXIT" );
static const std::string VISITED_STR( "VISITED" );
static const std::string PATH_STR( "PATH" );

static const double MAX_WEIGHT = 1e12;

static const std::string& cellToString( int value )
{
   if ( value == GraphGrid::PATH )
   {
      return PATH_STR;
   }
   else if ( value == GraphGrid::VISITED )
   {
      return VISITED_STR;
   }
   else if ( value == GraphGrid::ENTRY )
   {
      return START_STR;
   }
   else if ( value == GraphGrid::EXIT )
   {
      return EXIT_STR;
   }
   else if ( value == GraphGrid::BLOCK )
   {
      return BLOCK_STR;
   }
   return EMPTY_STR;
}

// ctor
GraphGrid::GraphGrid()
:
   Grid(),
   provider( NULL ),
   entryPointX( 0 ),
   entryPointY( 0 ),
   exitPointX( 0 ),
   exitPointY( 0 ),
   blockMessage( false )
{
}

// initialize the graph with the size of the grid
// and the provider to send message
void GraphGrid::initializeGraph( GraphGridProvider* provider,
                                 size_t width,
                                 size_t height )
{
   // create the grid
   Grid::initialize( width,
                     height );

   // set the entry point @0,0
   Grid::setValueAt( entryPointX,
                     entryPointY,
                     ENTRY );

   // and initialize the provider
   this->provider = provider;

   // and reset the information for the search purpose
   this->reset();
}

// change the value at the given point
// overrided to store the entry point
bool GraphGrid::setValueAt( size_t x,
                            size_t y,
                            int v )
{
   if ( v == ENTRY )
   {
      Grid::setValueAt( entryPointX,
                        entryPointY,
                        EMPTY );

      if ( blockMessage == false )
      {
         provider->sendMessageChangeCell( entryPointX,
                                          entryPointY,
                                          EMPTY_STR );
      }

      entryPointX = x;
      entryPointY = y;
   }

   if ( Grid::setValueAt( x, y, v ) == true )
   {
      if ( blockMessage == false )
      {
         provider->sendMessageChangeCell( x,
                                          y,
                                          cellToString( v ) );
      }
      return true;
   }
   return false;
}

// change the value at the given point
// overrided to store the entry point
// return true if the cell is really modified
bool GraphGrid::setValueAt( size_t x,
                            size_t y,
                            const std::string& value )
{
   int v = EMPTY;
   if (  ( value == BLOCK_STR )
       &&( getValueAt( x,
                       y ) != BLOCK )  )
   { 
      v = BLOCK;
   }
   else if ( value == START_STR )
   {
      v = ENTRY;
   }
   else if (  ( value == EXIT_STR )
            &&( getValueAt( x,
                            y ) != EXIT )  )
   {
      v = EXIT;
   }

   return setValueAt( x, y, v );
}

// do a DFS search on the graph
bool GraphGrid::computeDFS()
{
   // block message sending
   blockMessage = true;

   // prepare the result
   bool result = false;

   // check the graph computation visibility
   if ( isValid() == true )
   {
      // compute the DFS
      result = internalComputeDFS( entryPointX, 
                                   entryPointY );

      // put back the entryPoint to ENTRY (instead of PATH)
      // TODO manage to exclude the Entry from the Path
      setValueAt( entryPointX,
                  entryPointY,
                  ENTRY );
   }

   // allow message sending bakc to normal
   blockMessage = false;

   // rturn the computation result
   return result;
}

// do a recursive DFS search on the graph given the currentPoint
bool GraphGrid::internalComputeDFS( size_t x,
                                    size_t y )
{
   // check the end of the recursion
   int currentValue;
   if ( ( currentValue = getValueAt( x, y ) ) == EXIT )
   {
      // we find the exit
      return true;
   }
   else if (  ( ( currentValue & BLOCK ) == BLOCK )
            ||( ( currentValue & VISITED ) == VISITED )
            ||( currentValue == -1 )  )
   {
      // we find an obstacle or an invalid cell
      return false;
   }

   // set the visited flag
   setValueAt( x, y, currentValue + VISITED );

   // compute the E child
   if ( internalComputeDFS( x + 1, y ) == true )
   {
      // set the path flag
      setValueAt( x, y, currentValue + PATH );

      // and return as a path
      return true;
   }

   // compute the S child
   if ( internalComputeDFS( x, y + 1 ) == true )
   {
      // set the path flag
      setValueAt( x, y, currentValue + PATH );

      // and return as a path
      return true;
   }

   // compute the W child
   if ( internalComputeDFS( x - 1, y ) == true )
   {
      // set the path flag
      setValueAt( x, y, currentValue + PATH );

      // and return as a path
      return true;
   }

   // compute the N child
   if ( internalComputeDFS( x, y - 1 ) == true )
   {
      // set the path flag
      setValueAt( x, y, currentValue + PATH );

      // and return as a path
      return true;
   }

   // no path found on this exploration choice
   return false;
}

// call the BFS on the grid
bool GraphGrid::computeBFS()
{
   // block message sending
   blockMessage = true;

   // prepare the result
   bool result = false;

   // check the graph computation visibility
   if ( isValid() == true )
   {
      // create the node list to visit
      std::vector< GraphCell* > nodeToVisit;
      nodeToVisit.push_back( getCellAt( entryPointX,
                                        entryPointY ) );

      // and visit them
      GraphCell* currentCell;
      std::vector< GraphCell* > children;
      while ( nodeToVisit.size() > 0 )
      {
         // get the first cell
         currentCell = *nodeToVisit.begin();
         nodeToVisit.erase( nodeToVisit.begin() );

         // check if the node has already been visited
         if ( ( currentCell->getValue() & VISITED ) != VISITED )
         {
            // change the value of the node
            currentCell->modifyValue( VISITED );

            // check if the exit is found
            if ( ( currentCell->getValue() & EXIT ) == EXIT )
            {
               // reset the exit value to EXIT
               currentCell->modifyValue( - VISITED );

               // create the path back
               currentCell = currentCell->getFather();
               while ( currentCell != NULL )
               {
                  currentCell->modifyValue( - VISITED + PATH );
                  currentCell = currentCell->getFather();
               }

               // and ask to end
               result = true;
               break;
            }

            // get the children
            children.clear();
            currentCell->getChildren( this,
                                      children );

            // parse all the children
            for ( std::vector< GraphCell* >::iterator it = children.begin();
                  it != children.end();
                  it++ )
            {
               GraphCell* cell = *it;

               // add them if there are not block neither visited
               if (  ( ( cell->getValue() & BLOCK ) != BLOCK )
                   &&( ( cell->getValue() & VISITED ) != VISITED )  )
               {
                  cell->setFather( currentCell );
                  nodeToVisit.push_back( cell );
               }
            }
         }
      }

      // put back the entryPoint to ENTRY (instead of PATH)
      // TODO manage to exclude the Entry from the Path
      setValueAt( entryPointX,
                  entryPointY,
                  ENTRY );
   }

   // allow message sending bakc to normal
   blockMessage = false;

   // if we are here, no path found
   return result;
}

// call the DIJ on the grid
bool GraphGrid::computeDIJ()
{
   // block message sending
   blockMessage = true;

   // prepare the result
   bool result = false;

   // check the graph computation visibility
   if ( isValid() == true )
   {
      // create the node list to visit
      std::vector< GraphCell* > nodeToVisit;
      nodeToVisit.push_back( getCellAt( entryPointX,
                                        entryPointY ) );
      (*nodeToVisit.begin())->setWeight( 0 );

      // and visit them
      GraphCell* currentCell;
      std::vector< GraphCell* > children;
      while ( nodeToVisit.size() > 0 )
      {
         // get the first cell
         currentCell = *nodeToVisit.begin();
         nodeToVisit.erase( nodeToVisit.begin() );

         // check if the node has already been visited
         if ( ( currentCell->getValue() & VISITED ) != VISITED )
         {
            // change the value of the node
            currentCell->modifyValue( VISITED );

            // check if the exit is found
            if ( ( currentCell->getValue() & EXIT ) == EXIT )
            {
               // reset the exit value to EXIT
               currentCell->modifyValue( - VISITED );

               // create the path back
               currentCell = currentCell->getFather();
               while ( currentCell != NULL )
               {
                  currentCell->modifyValue( - VISITED + PATH );
                  currentCell = currentCell->getFather();
               }

               // and ask to end
               result = true;
               break;
            }

            // get the children
            children.clear();
            currentCell->getChildren( this,
                                       children );

            // parse all the children
            for ( std::vector< GraphCell* >::iterator it = children.begin();
                  it != children.end();
                  it++ )
            {
               GraphCell* cell = *it;

               // add them if there are not block neither visited
               // and if the cost of movement is less than the current cost
               if (  ( ( cell->getValue() & BLOCK ) != BLOCK )
                     &&( cell->getWeight() > currentCell->getWeight() + cell->getPathValue() )  )
               {
                  cell->setFather( currentCell );
                  cell->setWeight( currentCell->getWeight() + cell->getPathValue() );

                  bool added = false;
                  for ( std::vector< GraphCell* >::const_iterator it = nodeToVisit.begin();
                        it != nodeToVisit.end();
                        it++ )
                  {
                     if ( cell->getWeight() < (*it)->getWeight() )
                     {
                        nodeToVisit.insert( it,
                                            cell );
                        added = true;
                        break;
                     }
                  }
                  if ( added == false )
                  {
                     nodeToVisit.push_back( cell );
                  }
               }
            }
         }
      }

      // put back the entryPoint to ENTRY (instead of PATH)
      // TODO manage to exclude the Entry from the Path
      setValueAt( entryPointX,
                  entryPointY,
                  ENTRY );
   }

   // allow message sending bakc to normal
   blockMessage = false;

   // if we are here, no path found
   return result;
}

// call the A* on the grid
bool GraphGrid::computeAstar()
{
   // block message sending
   blockMessage = true;

   // prepare the result
   bool result = false;

   // check the graph computation visibility
   if ( isValidForAstar() == true )
   {
      // create the node list to visit
      std::vector< GraphCell* > nodeToVisit;
      nodeToVisit.push_back( getCellAt( entryPointX,
                                        entryPointY ) );
      (*nodeToVisit.begin())->setWeight( 0 );

      // and visit them
      bool result = false;
      GraphCell* currentCell;
      std::vector< GraphCell* > children;
      while ( nodeToVisit.size() > 0 )
      {
         // get the first cell
         currentCell = *nodeToVisit.begin();
         nodeToVisit.erase( nodeToVisit.begin() );

         // change the value of the node to mark it as visited
         if ( ( currentCell->getValue() & VISITED ) != VISITED )
         {
            currentCell->modifyValue( VISITED );

            // check if the exit is found
            if ( ( currentCell->getValue() & EXIT ) == EXIT )
            {
               // reset the exit value to EXIT
               currentCell->modifyValue( - VISITED );

               // create the path back
               currentCell = currentCell->getFather();
               while ( currentCell != NULL )
               {
                  currentCell->modifyValue( - VISITED + PATH );
                  currentCell = currentCell->getFather();
               }

               // and ask to end
               result = true;
               break;
            }

            // get the children
            children.clear();
            currentCell->getChildren( this,
                                       children );

            // parse all the children
            for ( std::vector< GraphCell* >::iterator it = children.begin();
                  it != children.end();
                  it++ )
            {
               GraphCell* cell = *it;

               // add them if there are not block neither visited
               // and if the cost of movement is less than the current cost
               if (  ( ( cell->getValue() & BLOCK ) != BLOCK )
                   &&( cell->getWeight() > currentCell->getWeight() + cell->getPathValue() )  )
               {
                  cell->setFather( currentCell );
                  cell->setWeight(currentCell->getWeight() + cell->getPathValue() );

                  bool added = false;
                  for ( std::vector< GraphCell* >::const_iterator it = nodeToVisit.begin();
                        it != nodeToVisit.end();
                        it++ )
                  {
                     if ( cell->getWeight() + cell->getEuclide( exitPointX,
                                                                exitPointY ) < (*it)->getWeight() + (*it)->getEuclide( exitPointX,
                                                                                                                       exitPointY ) )
                     {
                        nodeToVisit.insert( it,
                                            cell );
                        added = true;
                        break;
                     }
                  }
                  if ( added == false )
                  {
                     nodeToVisit.push_back( cell );
                  }
               }
            }
         }
      }

      // put back the entryPoint to ENTRY (instead of PATH)
      // TODO manage to exclude the Entry from the Path
      setValueAt( entryPointX,
                  entryPointY,
                  ENTRY );
   }

   // allow message sending bakc to normal
   blockMessage = false;

   // if we are here, no path found
   return result;
}

// call the A* manhattan on the grid
bool GraphGrid::computeAstarM()
{
   // block message sending
   blockMessage = true;

   // prepare the result
   bool result = false;

   // check the graph computation visibility
   if ( isValidForAstar() == true )
   {
      // create the node list to visit
      std::vector< GraphCell* > nodeToVisit;
      nodeToVisit.push_back( getCellAt( entryPointX,
                                        entryPointY ) );
      (*nodeToVisit.begin())->setWeight( 0 );

      // and visit them
      bool result = false;
      GraphCell* currentCell;
      std::vector< GraphCell* > children;
      while ( nodeToVisit.size() > 0 )
      {
         // get the first cell
         currentCell = *nodeToVisit.begin();
         nodeToVisit.erase( nodeToVisit.begin() );

         // change the value of the node to mark it as visited
         if ( ( currentCell->getValue() & VISITED ) != VISITED )
         {
            currentCell->modifyValue( VISITED );

            // check if the exit is found
            if ( ( currentCell->getValue() & EXIT ) == EXIT )
            {
               // reset the exit value to EXIT
               currentCell->modifyValue( - VISITED );

               // create the path back
               currentCell = currentCell->getFather();
               while ( currentCell != NULL )
               {
                  currentCell->modifyValue( - VISITED + PATH );
                  currentCell = currentCell->getFather();
               }

               // and ask to end
               result = true;
               break;
            }

            // get the children
            children.clear();
            currentCell->getChildren( this,
                                       children );

            // parse all the children
            for ( std::vector< GraphCell* >::iterator it = children.begin();
                  it != children.end();
                  it++ )
            {
               GraphCell* cell = *it;

               // add them if there are not block neither visited
               // and if the cost of movement is less than the current cost
               if (  ( ( cell->getValue() & BLOCK ) != BLOCK )
                   &&( cell->getWeight() > currentCell->getWeight() + cell->getPathValue() )  )
               {
                  cell->setFather( currentCell );
                  cell->setWeight(currentCell->getWeight() + cell->getPathValue() );

                  bool added = false;
                  for ( std::vector< GraphCell* >::const_iterator it = nodeToVisit.begin();
                        it != nodeToVisit.end();
                        it++ )
                  {
                     if ( cell->getWeight() + cell->getManahattan( exitPointX,
                                                                   exitPointY ) < (*it)->getWeight() + (*it)->getManahattan( exitPointX,
                                                                                                                             exitPointY ) )
                     {
                        nodeToVisit.insert( it,
                                            cell );
                        added = true;
                        break;
                     }
                  }
                  if ( added == false )
                  {
                     nodeToVisit.push_back( cell );
                  }
               }
            }
         }
      }

      // put back the entryPoint to ENTRY (instead of PATH)
      // TODO manage to exclude the Entry from the Path
      setValueAt( entryPointX,
                  entryPointY,
                  ENTRY );
   }

   // allow message sending bakc to normal
   blockMessage = false;

   // if we are here, no path found
   return result;
}

// call the A* manhattan epsilon on the grid
bool GraphGrid::computeAstarME()
{
   // block message sending
   blockMessage = true;

   // prepare the result
   bool result = false;

   // check the graph computation visibility
   if ( isValidForAstar() == true )
   {
      // create the node list to visit
      std::vector< GraphCell* > nodeToVisit;
      nodeToVisit.push_back( getCellAt( entryPointX,
                                        entryPointY ) );
      (*nodeToVisit.begin())->setWeight( 0 );

      // and visit them
      bool result = false;
      GraphCell* currentCell;
      std::vector< GraphCell* > children;
      while ( nodeToVisit.size() > 0 )
      {
         // get the first cell
         currentCell = *nodeToVisit.begin();
         nodeToVisit.erase( nodeToVisit.begin() );

         // change the value of the node to mark it as visited
         if ( ( currentCell->getValue() & VISITED ) != VISITED )
         {
            currentCell->modifyValue( VISITED );

            // check if the exit is found
            if ( ( currentCell->getValue() & EXIT ) == EXIT )
            {
               // reset the exit value to EXIT
               currentCell->modifyValue( - VISITED );

               // create the path back
               currentCell = currentCell->getFather();
               while ( currentCell != NULL )
               {
                  currentCell->modifyValue( - VISITED + PATH );
                  currentCell = currentCell->getFather();
               }

               // and ask to end
               result = true;
               break;
            }

            // get the children
            children.clear();
            currentCell->getChildren( this,
                                       children );

            // parse all the children
            for ( std::vector< GraphCell* >::iterator it = children.begin();
                  it != children.end();
                  it++ )
            {
               GraphCell* cell = *it;

               // add them if there are not block neither visited
               // and if the cost of movement is less than the current cost
               if (  ( ( cell->getValue() & BLOCK ) != BLOCK )
                   &&( cell->getWeight() > currentCell->getWeight() + cell->getPathValue() )  )
               {
                  cell->setFather( currentCell );
                  cell->setWeight(currentCell->getWeight() + cell->getPathValue() );

                  bool added = false;
                  for ( std::vector< GraphCell* >::const_iterator it = nodeToVisit.begin();
                        it != nodeToVisit.end();
                        it++ )
                  {
                     if ( cell->getWeight() + cell->getManahattanEpsilon( exitPointX,
                                                                          exitPointY ) < (*it)->getWeight() + (*it)->getManahattanEpsilon( exitPointX,
                                                                                                                                           exitPointY ) )
                     {
                        nodeToVisit.insert( it,
                                            cell );
                        added = true;
                        break;
                     }
                  }
                  if ( added == false )
                  {
                     nodeToVisit.push_back( cell );
                  }
               }
            }
         }
      }

      // put back the entryPoint to ENTRY (instead of PATH)
      // TODO manage to exclude the Entry from the Path
      setValueAt( entryPointX,
                  entryPointY,
                  ENTRY );
   }

   // allow message sending bakc to normal
   blockMessage = false;

   // if we are here, no path found
   return result;
}

// check the grpah validity before computation
// at least one EXIT
bool GraphGrid::isValid()
{
   for ( std::vector< GraphCell* >::const_iterator it = getCells().begin();
         it != getCells().end();
         it++ )
   {
      if ( (*it)->getValue() == EXIT )
      {
         return true;
      }
   }
   return false;
}

// check the grpah validity before A* computation
// only one EXIT
bool GraphGrid::isValidForAstar()
{
   int exitNumber = 0;
   for ( std::vector< GraphCell* >::const_iterator it = getCells().begin();
         it != getCells().end();
         it++ )
   {
      if ( (*it)->getValue() == EXIT )
      {
         exitPointX = (*it)->getX();
         exitPointY = (*it)->getY();

         exitNumber++;
      }
   }
   return ( exitNumber == 1 );
}

// display the cells on the stream
void GraphGrid::display( std::ostream& out,
                         bool flat ) const
{
   for( size_t y = 0;
        y < getHeight();
        ++y )
   {
      for( size_t x = 0;
           x < getWidth();
           ++x )
      {
         int currentValue = getValueAt( x, y );
         if ( ( currentValue & PATH ) == PATH )
         {
            out << 'P';
         }
         else if ( ( currentValue & VISITED ) == VISITED )
         {
            out << 'V';
         }
         else if ( ( currentValue & ENTRY ) == ENTRY )
         {
            out << 'S';
         }
         else if ( ( currentValue & EXIT ) == EXIT )
         {
            out << 'E';
         }
         else if ( ( currentValue & BLOCK ) == BLOCK )
         {
            out << 'X';
         }
         else 
         {
            out << '0';
         }
      }

      if ( flat == false )
      {
         out << std::endl;
      }
   }
}

// reset the graph information to remove the VISITED and PATH informatio
// also reweight each cell to MAX_WEIGHT and reset the exitPoint to 0,0
void GraphGrid::reset()
{
   for ( std::vector< GraphCell* >::iterator it = getCells().begin();
         it != getCells().end();
         it++ )
   {
      if ( ( (*it)->getValue() & VISITED ) == VISITED )
      {
         (*it)->modifyValue( -VISITED );
         (*it)->resetFather();
      }
      else if ( ( (*it)->getValue() & PATH ) == PATH )
      {
         (*it)->modifyValue( -PATH );
         (*it)->resetFather();
      }

      (*it)->setWeight( MAX_WEIGHT );
   }

   exitPointX = 0;
   exitPointY = 0;
}
