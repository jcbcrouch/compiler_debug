//
//  constraint_solver.cpp
//
//
//  Created by Bradley on 4/3/19.
//
//

#include "expression.h"
#include <climits>

/*
 IsrWord::IsrWord( String word )
 : validISR( true ), currentLocation(0){
 
 IsrInfo isrInfo = Postings::GetPostings( )->GetPostingList( word.CString( ) );
 nextPtr = isrInfo.nextPtr;
 
 if ( isrInfo.postingList )
 postingLists.push_back( isrInfo.postingList );
 else
 validISR = false;
 
 subBlocks.push_back( isrInfo.subBlock );
 }
 
 
 IsrWord::~IsrWord(){
 for( unsigned i = 0; i < postingLists.size(); i++ )
 delete postingLists[ i ];
 for( unsigned i = 0; i < subBlocks.size(); i++ )
 Postings::GetPostings()->MunmapSubBlock( subBlocks[i] );
 }
 
 
 Location IsrWord::nextInstance(){
 return SeekToLocation( 0 );
 }
 
 
 Location IsrWord::SeekToLocation (Location seekDestination){
 if ( !validISR || postingLists.empty( ) )
 return currentLocation = 0;
 
 for ( unsigned i = 0; i < postingLists.size( ); i++ )
 {
 Location posting = postingLists[ i ]->GetPosting( seekDestination );
 if ( posting != 0 )
 return currentLocation = posting;
 }
 
 while ( nextPtr != 0 )
 {
 IsrInfo isrInfo = Postings::GetPostings( )->GetPostingList( nextPtr );
 nextPtr = isrInfo.nextPtr;
 postingLists.push_back( isrInfo.postingList );
 subBlocks.push_back( isrInfo.subBlock );
 Location posting = isrInfo.postingList->GetPosting( seekDestination );
 if ( posting != 0 )
 return currentLocation = posting;
 }
 
 return currentLocation = 0;
 }
 
 
 Location IsrWord::CurInstance( ) const{
 return currentLocation;
 }
 
 //////////////
 //EndDoc ISR//
 //////////////
 IsrEndDoc::IsrEndDoc( ) : IsrWord( "" ){
 }
 */
/*
 Isr::Isr(Vector<Location> matchesIn)
 : matches(matchesIn), curInd(0) {}
 */
//JACOB TODO: DEFINE CONSTRUCTOR FOR ISR WORD

Location IsrWord::nextInstance()
{
   if(hasNextInstance())
      currentLocation = matches[++curInd];
   else
      currentLocation = IsrGlobals::IsrSentinel;
   return currentLocation;
}

void IsrWord::SetLocations (Vector<Location>& matchesIn)
{
   matches = matchesIn;
}

Location IsrWord::SeekToLocation(Location location)
{
   Location closestLocation = IsrGlobals::IsrSentinel;
   curInd = 0;
   if(matches.size() > 0)
   {
      closestLocation = matches[0];
   }
   
   while(curInd < matches.size() - 1 && matches[curInd] < closestLocation)
   {
      ++curInd;
      closestLocation = matches[curInd];
   }
   
   if(curInd == matches.size() - 1)
   {
      if(matches[curInd] > location)
      {
         closestLocation = matches[curInd];
      }
      else
         closestLocation = IsrGlobals::IsrSentinel;
   }
   currentLocation = closestLocation;
   return closestLocation;
}

bool IsrWord::hasNextInstance()
{
   return curInd < matches.size() - 1;
}

Location IsrWord::getClosestStartLocation()
{
   return currentLocation;
}

Location IsrWord::getClosestEndLocation(){
   String blank = "";
   IsrWord pageEnd(blank);
   return pageEnd.SeekToLocation(currentLocation);
}

DocumentAttributes IsrEndDoc::GetDocInfo()
{
   DocumentAttributes docInfo;
   docInfo.DocID = curInd;
   if(curInd == 0)
   {
      docInfo.DocumentLength = matches[curInd] - 1;
   }
   else
   {
      docInfo.DocumentLength = matches[curInd] - matches[curInd - 1];
   }
   return docInfo;
}

void IsrWord::AddWord(String &wordIn)
{
   word = wordIn;
}

void IsrWord::SetImportance(unsigned importanceIn)
{
   importance = importanceIn;
}

//////////
//OR ISR//
//////////
IsrOr::IsrOr(Vector<Isr*> phrasesToInsert){
   for (int i = 0; i < phrasesToInsert.size(); ++i){
      
      terms.push_back(phrasesToInsert[i]);
   }
   numOfTerms += phrasesToInsert.size();
}

Location IsrOr::SeekToLocation(Location target){
   //Algorithm
   // 1. Seek all ISRs to the first occurrence beginning at the target location.
   // 2. Loop through all the terms and return the term location which is smallest
   //Returns 0 if there is no match
   Location closestTerm = ULLONG_MAX;
   //Step 1: seek all ISRs to first occurrence after target location
   for (int i = 0; i < numOfTerms; ++i){
      //Traverse each term's posting list until it goes past 'target' or reaches the end
      while (terms[i]->getClosestStartLocation() < target){
         //We've looped thru all terms, and nextInstance of the last term DNE,
         //so no term exists after location target
         if (terms[i]->nextInstance() == 0 && i == numOfTerms - 1 && closestTerm == ULLONG_MAX){
            return 0;
         }
         Location nextLocation = terms[i]->nextInstance();
         //Update closestTerm, the term that is closest to and > target
         if (nextLocation < closestTerm && nextLocation >= target){
            closestTerm = nextLocation;
            nearestTerm = i;
         }
      }
   }
   //nearestStartLocation is the first term that appears in the OR ISR
   nearestStartLocation = closestTerm;
   //Update nearestEndLocation, the end of the page of nearestStartLocation
   String blank = "";
   IsrWord endPage(blank);
   nearestEndLocation = endPage.SeekToLocation(closestTerm);
   return closestTerm;
   
}



Location IsrOr::nextInstance(){
   
   //Retrieve the next instance of the first occurring term
   //Case 1: nearestTerm is initialized (because we initialize nearestTerm to 99999)
   //and is an index to the closest term
   if (nearestTerm != 99999){
      terms[nearestTerm]->nextInstance();
      Location closestTerm = ULLONG_MAX;
      for (int i = 0; i < numOfTerms; ++i){
         if (terms[i]->getClosestStartLocation() < closestTerm && terms[i]->getClosestStartLocation() != 0){
            closestTerm = terms[i]->getClosestStartLocation();
            nearestTerm = i;
         }
         //There are no next instances of any of the terms
         if (terms[i]->getClosestStartLocation() == 0 && i == numOfTerms - 1 && closestTerm == ULLONG_MAX){
            return 0;
         }
      }
      nearestStartLocation = closestTerm;
      String blank = "";
      IsrWord endPage(blank);
      nearestEndLocation = endPage.SeekToLocation(closestTerm);
      return closestTerm;
   }
   //Case 2: nearestTerm is not initialized yet, this is the first call of nextInstance()
   //so we must find the term that is closest to the start of a posting list
   else {
      return SeekToLocation(0);
   }
   
}


/////////////////
//End of OR ISR//
/////////////////

///////////
//AND ISR//
///////////

IsrAnd::IsrAnd(Vector<Isr*> phrasesToInsert){
   for (int i = 0; i < phrasesToInsert.size(); ++i){
      terms.push_back(phrasesToInsert[i]);
   }
   numOfTerms += phrasesToInsert.size();
}


Location IsrAnd::SeekToLocation(Location target){
   //Algorithm
   // 1. Seek all ISRs to the first occurrence beginning at the target location.
   // 2. Pick the furthest term and attempt to seek all the other terms to the
   //first location beginning where they should appear relative to the furthest term.
   // 3. If any term is past the desired location, return to step 2.
   // 4. If any ISR reaches the end, there is no match.
   Vector<std::pair<Isr*, Location>>docTracker;
   
   //Step 1: seek all ISRs to first occurrence after target location
   for (int i = 0; i < numOfTerms; ++i){
      //Traverse each term's posting list until it goes past 'target' or reaches the end
      while (terms[i]->getClosestStartLocation() < target){
         //None of the posting list terms are on the same page
         if (terms[i]->nextInstance() == 0){
            return 0;
         }
         Location nextLocation = terms[i]->nextInstance();
         String blank = "";
         IsrWord nextPage(blank);
         Location pageEnd = nextPage.SeekToLocation(nextLocation);
         std::pair<Isr*, Location> toInsert(terms[i], pageEnd);
         docTracker.push_back(toInsert);
      }
      
   }
   
   Location latestPage = 0;
   bool allSamePage = false;
   while (!allSamePage){
      //Step 2: Find the furthest term's page
      for (int i = 0; i < docTracker.size(); ++i){
         if (docTracker[i].second > latestPage){
            latestPage = docTracker[i].second;
         }
      }
      
      //Step 3: Scan other pages, if other pages are before furthest term, move them forward
      bool pageAltered = false;
      for (int i = 0; i < docTracker.size(); ++i){
         if (docTracker[i].second < latestPage){
            while (docTracker[i].first->getClosestStartLocation() < latestPage){
               //Step 4: Check if any pages reach the end
               if (docTracker[i].first->nextInstance() == 0){
                  return 0;
               }
               //Update the IsrWord index and latest page
               docTracker[i].first->nextInstance();
               String blank = "";
               IsrWord nextPage(blank);
               Location valToCompare = nextPage.SeekToLocation(docTracker[i].first->getClosestStartLocation());
               if (valToCompare != docTracker[i].second){
                  pageAltered = true;
               }
               docTracker[i].second = nextPage.SeekToLocation(docTracker[i].first->getClosestStartLocation());
            }
         }
      }
      //Go back to step 2 if we moved any ISRs
      if (!pageAltered){
         allSamePage = true;
      }
   }
   
   Location earliestPost = ULLONG_MAX;
   Location latestPost = 0;
   for (int i = 0; i < docTracker.size(); ++i){
      if (docTracker[i].first->getClosestStartLocation() < earliestPost){
         earliestPost = docTracker[i].first->getClosestStartLocation();
         nearestTerm = i;
      }
      if (docTracker[i].first->getClosestStartLocation() > latestPost){
         latestPost = docTracker[i].first->getClosestStartLocation();
         farthestTerm = i;
      }
   }
   nearestStartLocation = earliestPost;
   nearestEndLocation = docTracker[0].second;
   //Returns the end of the document that all the terms appear on
   return nearestStartLocation;
}

//////////////////
//End of AND ISR//
//////////////////

//////////////
//Phrase ISR//
//////////////

IsrPhrase::IsrPhrase(String phraseToStore){
   String currWord = "";
   for (int i = 0; i < phraseToStore.Size(); ++i){
      if (isalpha(phraseToStore[i])){
         currWord += &phraseToStore[i];
      }
      else {
         IsrWord* subPhrase = new IsrWord(currWord);
         currWord = "";
         terms.push_back(subPhrase);
      }
   }
}


//TODO
// 1. Seek all ISRs to the first occurrence beginning at the target location.
// 2. Pick the furthest term and attempt to seek all the other terms to the
//first location beginning where they should appear relative to the furthest term.
// 3. If any term is past the desired location, return to step 2.
// 4. If any ISR reaches the end, there is no match.

Location IsrPhrase::SeekToLocation(Location target){
   Vector<Location>locationTracker;
   
   //Step 1: seek all ISRs to first occurrence after target location
   for (int i = 0; i < numOfTerms; ++i){
      //Traverse each term's posting list until it goes past 'target' or reaches the end
      while (terms[i]->getClosestStartLocation() < target){
         //None of the posting list terms are on the same page
         if (terms[i]->nextInstance() == 0){
            return 0;
         }
         Location nextLocation = terms[i]->nextInstance();
         if (nextLocation >= target){
            locationTracker.push_back(nextLocation);
         }
      }
      
   }
   
   // 2. Pick the furthest term and attempt to seek all the other terms to the first
   //    location beginning where they should appear relative to the furthest term.
   
   bool phraseExists = false;
   
   //loop thru all documents until we find a phrase or end of posting lists
   while (!phraseExists){
      
      //Step a: Picking the furthest term
      unsigned farthestTermIndex = -1;
      Location farthestTerm = 0;
      for (int i = 0; i < locationTracker.size(); ++i){
         if (locationTracker[i] > farthestTerm){
            farthestTerm = locationTracker[i];
            farthestTermIndex = i;
         }
      }
      
      //Step a2:
      //This is just a check in case we "happen" to find a phrase on the first try
      //Check if phraase exists in a document
      for (int i = 1; i < locationTracker.size(); ++i){
         //Break out of the loop anytime the indices dont match up
         if (locationTracker[i] != locationTracker[i - 1] + 1){
            phraseExists = false;
            break;
         }
         //check if the phrase exists
         else if (i == locationTracker.size() - 1){
            phraseExists = true;
         }
         
      }
      //Phrase exists
      if (phraseExists) {
         break;
      }
      
      //Move all ISRS to their relative position to the furthest term
      for (int i = 0; i < locationTracker.size(); ++i){
         if (i < farthestTermIndex){
            Location wordOffset = farthestTermIndex - i;
            Location expectedPosition = locationTracker[farthestTermIndex] - wordOffset;
            while (terms[i]->getClosestStartLocation() < expectedPosition){
               //Term does not exist in remaining posting lists
               if (terms[i]->nextInstance() == 0){
                  return 0;
               }
               locationTracker[i] = terms[i]->nextInstance();
            }
         }
         else if (i > farthestTermIndex){
            Location wordOffset = farthestTermIndex + i;
            Location expectedPosition = locationTracker[farthestTermIndex] + wordOffset;
            while (terms[i]->getClosestStartLocation() < expectedPosition){
               //Term does not exist in remaining posting lists
               if (terms[i]->nextInstance() == 0){
                  return 0;
               }
               locationTracker[i] = terms[i]->nextInstance();
            }
         }
      }//for
      
      
   }//while
   
   //Update ISRPhrase variables
   nearestTerm = 0;
   farthestTerm = numOfTerms - 1;
   nearestStartLocation = locationTracker[0];
   String blank = "";
   IsrWord endPage(blank);
   nearestEndLocation = endPage.SeekToLocation(nearestStartLocation);
   return locationTracker[0];
   
   
   
   
   //
   //    Location latestPage = 0;
   //    bool allSamePage = false;
   //    while (!allSamePage){
   //        //Step 2: Find the furthest term's page
   //        for (int i = 0; i < docTracker.size(); ++i){
   //            if (docTracker[i].second > latestPage){
   //                latestPage = docTracker[i].second;
   //            }
   //        }
   //
   //        //Step 3: Scan other pages, if other pages are before furthest term, move them forward
   //        bool pageAltered = false;
   //        for (int i = 0; i < docTracker.size(); ++i){
   //            if (docTracker[i].second < latestPage){
   //                while (docTracker[i].first->CurInstance() < latestPage){
   //                    //Step 4: Check if any pages reach the end
   //                    if (docTracker[i].first->nextInstance() == 0){
   //                        return 0;
   //                    }
   //                    //Update the IsrWord index and latest page
   //                    docTracker[i].first = docTracker[i].first->nextInstance();
   //                    IsrWord nextPage("");
   //                    Location valToCompare = nextPage.SeekToLocation(docTracker[i].first);
   //                    if (valToCompare != docTracker[i].second){
   //                        pageAltered = true;
   //                    }
   //                    docTracker[i].second = nextPage.SeekToLocation(docTracker[i].first);
   //                }
   //            }
   //        }
   //        //Go back to step 2 if we moved any ISRs
   //        if (!pageAltered){
   //            allSamePage = true;
   //        }
   //    }
   //
   //    Location earliestPost = ULLONG_MAX;
   //    Location latestPost = 0;
   //    for (int i = 0; i < docTracker.size(); ++i){
   //        if (docTracker[i].first->CurInstance() < earliestPost){
   //            earliestPost = docTracker[i].first->CurInstance();
   //            nearestTerm = i;
   //        }
   //        if (docTracker[i].first->CurInstance() > latestPost){
   //            latestPost = docTracker[i].first->CurInstance();
   //            farthestTerm = i;
   //        }
   //    }
   //    nearestStartLocation = earliestPost;
   //    nearestEndLocation = docTracker[0].second;
   //    //Returns the end of the document that all the terms appear on
   //    return nearestStartLocation;
}