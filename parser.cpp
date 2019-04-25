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

ISR *Parser::FindPhrase( ) // "bill (nye|gates)" turn into??
{
   ISR *word = stream.parseWord();
   if( word ) {
      ISRPhrase *self = new ISRPhrase( );
      self->addTerm( word );
      while( ( word = stream.parseWord() )) {
         self->addTerm( word );
      }
      return self;
   }
   
   return nullptr;
   
}

ISR *Parser::FindOr( )
{
   ISR *left = FindAnd( );
   if ( left )
   {
      ISR *self = new ISROr( );
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

ISR *Parser::FindAnd( )
{
   ISR *left = FindSimple( );
   if ( left )
   {
      ISR *self = new ISRAnd( );
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


ISR *Parser::FindSimple( )
{
   
   if ( stream.Match( '"' ) ) {// PHRASE
    ISR *left = FindPhrase( );
    if ( left )
    {
    ISR *self = new ISRPhrase( );
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
       ISR *left = FindOr( );
       if ( left )
       {
          ISROr *self = new ISROr( );
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

ISR *Parser::Parse( )
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
