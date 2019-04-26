/*
 * parser.cpp
 *
 * Implementation of parser.h
 *
 */

#include "expression.h"
#include "parser.h"

/* <OrConstraint>  ::=   <AndConstraint> { <OrOp> <AndConstraint> }
 *
 * <AndConstraint>  ::= <SimpleConstraint> { [ <AndOp> ] <SimpleConstraint> }
 *
 * <SimpleConstraint>  ::= <Phrase> | ‘(’ <OrConstraint> ‘)’| <UnaryOp> <SimpleConstraint> | <SearchWord>
 *
 * <Phrase>  ::= '"' { <SearchWord> } '"'
 */

Isr *Parser::FindPhrase( ) // "bill (nye|gates)" turn into??
{
   Isr *word = stream.parseWord();
   if( word ) {
      IsrPhrase *self = new IsrPhrase( );
      self->addTerm( word );
      while( ( word = stream.parseWord() )) {
         self->addTerm( word );
      }
      return self;
   }
   
   return nullptr;
   
}

Isr *Parser::FindOr( )
{
   Isr *left = FindAnd( );
   if ( left )
   {
      IsrOr *self = new IsrOr( );
      self->addTerm( left );
      while ( stream.Match( '|' ) || stream.Match( "OR" ) || stream.Match( "||" ) )
      {
         left = FindOr( );
         if( !left ) {
            return nullptr;
         }
         self->addTerm( left );
         // ...
      }
      return self;
   }
   return nullptr;
}

Isr *Parser::FindAnd( )
{
   Isr *left = FindSimple( );
   if ( left )
   {
      IsrAnd *self = new IsrAnd( );
      self->addTerm( left );
      while ( ( left = FindSimple( ) ) || stream.Match( '&' ) || stream.Match( "&&" ) || stream.Match( "AND" ) )
      {
         if( stream.match_and ) {
            left = FindSimple( );
            stream.match_and = false;
         }
         if( !left ) {
            return nullptr;
         }
         self->addTerm( left );
         if( stream.last_char( ) ) {
            return self;
         }
         // ...
      }
      return self;
   }
   return nullptr;
}


Isr *Parser::FindSimple( )
{
   
   if ( stream.Match( '"' ) ) {// PHRASE
    Isr *left = FindPhrase( );
    if ( left )
    {
    IsrPhrase *self = new IsrPhrase( );
    self->addTerm( left );
    if( !stream.Match( '"' ) )
    {
    return nullptr;//must be closing
    }
    return self;
    }
    }
    else if(stream.Match( '(' ) )
    {
       Isr *left = FindOr( );
       if ( left )
       {
          IsrOr *self = new IsrOr( );
          self->addTerm( left );
          if( !stream.Match( ')' ) )
          {   //must close
             return nullptr;
          }
          return self;
       }
    }
   /*else if( stream.Match('+') || stream.Match('-')) {//UNARY + SIMPLE
    ISR *left = FindSimple( );
    if ( left )
    {
    SubExpression *self = new SubExpression( );
    self->addTerm( left );
    return self;
    }
    }*/
   
    else {
       return stream.parseWord( );
    }
   
   
   
   return nullptr;
}

Isr *Parser::Parse( )
{
   return FindOr( );
   
}

bool Parser::fullParsed( )
{
   return stream.AllConsumed();
}

Parser::Parser( const std::string &in ) :
stream( in )
{
}
