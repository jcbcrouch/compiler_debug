//
//  constraint_solver.hpp
//
//
//  Created by Bradley on 4/3/19.
//
//

#ifndef constraint_solver_hpp
#define constraint_solver_hpp
#include <stdio.h>
#include "String.hpp"
#include "Vector.h"
#include "IndexInterface.h"

typedef unsigned long long Location;
typedef size_t FileOffset;

namespace IsrGlobals
{
   const Location IsrSentinel = 0;
   //TODO: CHange to 1
   const Location IndexStart = 1;
}



class Isr{
public:
   Isr(){}
   //Isr pointer to the end of the current doc
   Isr* docEnd;
   
   //Find next instance of a term
   virtual Location nextInstance() = 0;
   
   //Similar to nextDocument, finds the first occurrence of a term just at 'target' or later
   virtual Location SeekToLocation(Location target) = 0;
   
   //First number in the posting list of a term
   virtual Location GetCurrentInstance() = 0;
   
   //Last number in the posting list of a term
   virtual Location getClosestEndLocation() = 0;
   
   //Returns whatever document you're looking at
   Isr* getDocIsr();
   
   //virtual Location CurInstance() const;
   virtual ~Isr(){}
};

class IsrWord : public Isr {
public:
    IsrWord() {}
    IsrWord( String &word_in );
   void SetLocations( Vector<Location>& matchesIn );
   //    ~IsrWord( );
   
   Location nextInstance( )override;
   Location SeekToLocation( Location seekDestination = 0 )override;
   Location GetCurrentInstance() override;
   Location getClosestEndLocation() override;
   //Location CurInstance() const override;
   void AddWord(String &wordIn);
   DocumentAttributes GetDocInfo();

   void SetImportance(unsigned importanceIn);
   
   
   operator bool( ) const;
   friend class IsrEndDoc;
   
private:
   bool validIsr;
   Vector<Location> matches;
   Location currentLocation;
   String word;
   unsigned nextPtr;
   unsigned importance;
   unsigned curInd;
   bool hasNextInstance();
};


class IsrOr : public Isr{
public:
   //List of Isrs that we keep track of
   Vector<Isr*> terms;
   
   void addTerm(Isr* word) {
      terms.push_back(word);
   }
   
   //Variable to keep track of how many terms are in 'terms' (because resize/reserve isn't implemented)
//   unsigned numOfTerms = 0;
   
   //Constructor for IsrOr, MUST be in a vector<Isr> format, otherwise it wont compile
   IsrOr(Vector<Isr*> phrasesToInsert);
   IsrOr() {}
   
   //Destructor
   //    ~IsrOr( ){}
   
   //Points to the closest 'beginning of page'
   Location GetCurrentInstance() override {
       if (nearestStartLocation != 0){
           return nearestStartLocation;
       }
       //Find the first instance of a word. Only go down here on first call to nextInstance()
       Location tempTracker = ULLONG_MAX;
       for (int i = 0; i < terms.size(); ++i){
           if (terms[i]->GetCurrentInstance() < tempTracker){
               tempTracker = terms[i]->GetCurrentInstance();
           }
       }
       nearestStartLocation = tempTracker;
       return nearestStartLocation;
   }
   
   //Points to the closest 'end of page'
   Location getClosestEndLocation() override {
       if (nearestEndLocation != 0){
           return nearestEndLocation;
       }
       
       //Since nearestEndLocation is initialized to 0, we find the first endDoc thru here
       IsrWord pageEnd;
       String blank = "";
       pageEnd.AddWord(blank);
       nearestEndLocation = pageEnd.SeekToLocation(nearestStartLocation);
       return nearestEndLocation;
   }
   
   //Move all Isrs to the first occurrence of their respective word at 'target' or later
   //Returns ULLONG_MAX if there is no match
   Location SeekToLocation(Location target) override;
   //TODO:
   // 1. Seek all the Isrs to the first occurrence beginning at
   //    the target location.
   // 2. Move the document end Isr to just past the furthest
   //    word, then calculate the document begin location.
   // 3. Seek all the other terms to past the document begin.
   // 4. If any term is past the document end, return to step 2
   // 5. If any Isr reaches the end, there is no match.
   
   //Find the next instance of ANY of the words in 'terms'
   Location nextInstance() override;
   
   //Seek all Isrs to the first occurrence JUST PAST the end of the current document
   Location nextDocument(){
      return SeekToLocation(docEnd->getClosestEndLocation() + 1);
   }
private:
   //
   unsigned nearestTerm = 99999;
    Location nearestStartLocation = 0;
    Location nearestEndLocation = 0;
};

class IsrAnd : public Isr{
public:
   //Container for terms
   Vector<Isr*> terms;
   
   //Keeps track of how many terms we have
   //unsigned numOfTerms = 0;
   
   //Constructor for IsrAnd
   IsrAnd(Vector<Isr*> phrasesToInsert);
   
   IsrAnd() {}
   
   //Destructor
   ~IsrAnd(){};
   
   Location SeekToLocation(Location target) override;
   //TODO:
   // 1. Seek all the Isrs to the first occurrence beginning at
   //    the target location.
   // 2. Move the document end Isr to just past the furthest
   //    word, then calculate the document begin location.
   // 3. Seek all the other terms to past the document begin.
   // 4. If any term is past the document end, return to step 2
   // 5. If any Isr reaches the end, there is no match.
   
   //Finds next instance of all terms in a page
   Location nextInstance() override{
      return SeekToLocation(nearestStartLocation + 1);
   }
   
   //Points to the closest 'beginning of page'
   Location GetCurrentInstance() override {
       if (nearestStartLocation != 0){
           return nearestStartLocation;
       }
       Location closestTerm = 9999999;
       Location farthestTerm = 0;
       for (int i = 0; i < terms.size(); ++i){
           if (terms[i]->GetCurrentInstance() < closestTerm){
               nearestTerm = i;
               closestTerm = terms[i]->GetCurrentInstance();
           }
           if (terms[i]->GetCurrentInstance() > farthestTerm){
               farthestTerm = i;
           }
       }
       nearestStartLocation = closestTerm;
      return nearestStartLocation;
   }
   
   //Points to the closest 'end of page'
   Location getClosestEndLocation() override {
       if (nearestEndLocation != 0){
           return nearestEndLocation;
       }
       
       //Since nearestEndLocation is initialized to 0, we find the first endDoc thru here
       IsrWord pageEnd;
       String blank = "";
       pageEnd.AddWord(blank);
       nearestEndLocation = pageEnd.SeekToLocation(nearestStartLocation);
       return nearestEndLocation;
   }
   
   void addTerm(Isr* word) {
      terms.push_back(word);
   }
   
private:
    unsigned nearestTerm = 0;
    unsigned farthestTerm = 0;
    Location nearestStartLocation = 0;
    Location nearestEndLocation = 0;
};

class IsrPhrase : public Isr{
public:
   //Container for terms
   Vector<Isr*> terms;
   
   //Keeps track of how many terms we have
   //unsigned numOfTerms = 0;
   
   IsrPhrase() {}
   
   //Constructor for IsrPhrase
   IsrPhrase(String phraseToStore);
   
   //Destrucor
   //    ~IsrPhrase(){};
   
   //Finds next instance after target location
   Location SeekToLocation(Location target) override;
   
   //Finds next instance of phrase match
   Location nextInstance() override{
      return SeekToLocation(nearestStartLocation + 1);
   }
   
   //end me
   Location GetCurrentInstance() override{
      return terms[0]->GetCurrentInstance();
   }
   
   //end me 2
   Location getClosestEndLocation() override {
      return terms[0]->getClosestEndLocation();
   }
   
   void addTerm(Isr* word) {
      terms.push_back(word);
   }
   
private:
    unsigned nearestTerm = 0;
    unsigned farthestTerm = terms.size() - 1;
   Location nearestStartLocation, nearestEndLocation;
};

/*
 //ULONG_MAX for failure
 //Should have an enddoc Isr pointing to end of pages
 
 class IsrEndDoc : IsrWord{
 public:
 IsrEndDoc( );
 };
 
 //Advanced feature, may implement later
 class IsrContainer : public Isr {
 public:
 Isr** contained, excluded;
 IsrEndDoc* docEnd;
 unsigned countContained, countExcluded;
 //Location Next();
 Location seek(Location target);
 Location next();
 private:
 unsigned nearestTerm, farthestTerm;
 Location nearestStartLocation, nearestEndLocation;
 };
 */

class IsrEndDoc : public IsrWord
{
public:
   IsrEndDoc() {}
   DocumentAttributes GetDocInfo();
   void AddMatches(Vector<Location>& matchesIn);
};

#endif /* constraint_solver_hpp */
