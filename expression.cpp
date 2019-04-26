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


void IsrEndDoc::AddMatches(Vector<Location>& matchesIn)
{
    matches = matchesIn;
}

IsrWord::IsrWord(String& wordIn)
: word(wordIn), curInd(0)
{
    String quick = "quick";
    String brown = "brown";
    String fox = "fox";
    String empty = "";
    if(word.Compare(quick))
    {
        matches.push_back(4);
        matches.push_back(5);
        matches.push_back(13);
        matches.push_back(22);
    }
    else if(word.Compare(brown))
    {
        matches.push_back(3);
        matches.push_back(7);
    }
    else if(word.Compare(fox))
    {
        matches.push_back(2);
        matches.push_back(14);
        matches.push_back(23);
    }
    else if(word.Compare(empty))
    {
        matches.push_back(10);
        matches.push_back(20);
        matches.push_back(30);
        matches.push_back(40);
    }
    else
    {
        printf("wrong word!!!!!!!!");
        return;
    }
    
    //populate current instance
    if(matches.empty())
        currentLocation = 0;
    else
        currentLocation = matches[0];
}

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

Location IsrWord::GetCurrentInstance()
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

DocumentAttributes IsrWord::GetDocInfo()
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

}

Location IsrOr::SeekToLocation(Location target){
   //Algorithm
   // 1. Seek all ISRs to the first occurrence beginning at the target location.
   // 2. Loop through all the terms and return the term location which is smallest
   //Returns 0 if there is no match
    Location closestTerm;
    if (nearestStartLocation != 0){
        closestTerm = nearestStartLocation;
    }
    else {
        Location tempTracker = 99999;
        
        for (int i = 0; i < terms.size(); ++i){
            if (terms[i]->GetCurrentInstance() < tempTracker){
                tempTracker = terms[i]->GetCurrentInstance();
                nearestTerm = i;
            }
        }
        closestTerm = tempTracker;
    }
   //Step 1: seek all ISRs to first occurrence after target location
   for (int i = 0; i < terms.size(); ++i){
      //Traverse each term's posting list until it goes past 'target' or reaches the end
      while (terms[i]->GetCurrentInstance() < target){
         //We've looped thru all terms, and nextInstance of the last term DNE,
         //so no term exists after location target
         if (terms[i]->nextInstance() == 0 && i == terms.size() - 1 && closestTerm == 0){
            return 0;
         }
         Location nextLocation = terms[i]->nextInstance();
         //Update closestTerm, the term that is closest to and > target
         if (nextLocation < closestTerm && nextLocation >= target){
            closestTerm = nextLocation;
            nearestTerm = i;
         }
          //First pass
         else if (closestTerm == 0){
             closestTerm = nextLocation;
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
      for (int i = 0; i < terms.size(); ++i){
         if (terms[i]->GetCurrentInstance() < closestTerm && terms[i]->GetCurrentInstance() != 0){
            closestTerm = terms[i]->GetCurrentInstance();
            nearestTerm = i;
         }
         //There are no next instances of any of the terms
         if (terms[i]->GetCurrentInstance() == 0 && i == terms.size() - 1 && closestTerm == ULLONG_MAX){
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
      return SeekToLocation(1);
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
   for (int i = 0; i < terms.size(); ++i){
      //Traverse each term's posting list until it goes past 'target' or reaches the end
      while (terms[i]->GetCurrentInstance() < target){
         //None of the posting list terms are on the same page
         if (terms[i]->nextInstance() == 0){
            return 0;
         }
         terms[i]->nextInstance();
      }
       String blank = "";
       IsrWord nextPage(blank);
       Location pageEnd = nextPage.SeekToLocation(terms[i]->GetCurrentInstance());
       std::pair<Isr*, Location> toInsert(terms[i], pageEnd);
       docTracker.push_back(toInsert);
      
   }
   
   Location latestPage = 0;
    unsigned latestPageIndex = 0;
   bool allSamePage = false;
   while (!allSamePage){
      //Step 2: Find the furthest term's page
      for (int i = 0; i < docTracker.size(); ++i){
         if (docTracker[i].second > latestPage){
            latestPage = docTracker[i].second;
             latestPageIndex = i;
         }
      }
      
      //Step 3: Scan other pages, if other pages are before furthest term, move them forward
      bool pageAltered = false;
      for (int i = 0; i < docTracker.size(); ++i){
         if (docTracker[i].second < latestPage){
             String blank = "";
             IsrWord nextPage(blank);
             //While term i's page is less than the current page
            while (docTracker[i].first->GetCurrentInstance() < latestPage ) {
               //Step 4: Check if any pages reach the end
               if (docTracker[i].first->nextInstance() == 0){
                  return 0;
               }
                //Check if the wordISR we're moving has reached the same document as the furthest term ISR
                Location valToCompare = nextPage.SeekToLocation(docTracker[i].first->GetCurrentInstance());
                if (valToCompare > docTracker[latestPageIndex].second){
                    pageAltered = true;
                }
                //Update the IsrWord index and latest page
                docTracker[i].second = valToCompare;
               docTracker[i].first->nextInstance();
            }
             //We must check the page end again in case the term appears after the furthest term, but on the same page
             Location valToCompare = nextPage.SeekToLocation(docTracker[i].first->GetCurrentInstance());
             if (valToCompare == docTracker[latestPageIndex].second){
                 pageAltered = false;
                 docTracker[i].second = valToCompare;
             }
             
         }
      }
      //Go back to step 2 if we moved any ISRs
      if (!pageAltered){
         allSamePage = true;
      }
   }
   
    Location earliestPost = 999999;
   Location latestPost = 0;
   for (int i = 0; i < docTracker.size(); ++i){
      if (docTracker[i].first->GetCurrentInstance() < earliestPost){
         earliestPost = docTracker[i].first->GetCurrentInstance();
         nearestTerm = i;
      }
      if (docTracker[i].first->GetCurrentInstance() > latestPost){
         latestPost = docTracker[i].first->GetCurrentInstance();
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
   for (int i = 0; i < terms.size(); ++i){
      //Traverse each term's posting list until it goes past 'target' or reaches the end
      while (terms[i]->GetCurrentInstance() < target){
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
            while (terms[i]->GetCurrentInstance() < expectedPosition){
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
            while (terms[i]->GetCurrentInstance() < expectedPosition){
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
   farthestTerm = terms.size() - 1;
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
