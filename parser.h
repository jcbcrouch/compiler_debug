/*
 * parser.h
 *
 * Basic query expression parser that supports phrase, and, or queries
 *
 * A basic BNF query language:
 *
 * <OrConstraint>  ::=   <AndConstraint> { <OrOp> <AndConstraint> }
 *
 * <AndConstraint>  ::= <SimpleConstraint> { [ <AndOp> ] <SimpleConstraint> }
 *
 * <SimpleConstraint>  ::= <Phrase> | ‘(’ <OrConstraint> ‘)’| <UnaryOp> <SimpleConstraint> | <SearchWord>
 *
 * <Phrase>  ::= '"' { <SearchWord> } '"'
 *
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "expression.h"
#include "tokenstream.h"

/**
 * The actual expression parser
 */
class Parser
{
    // Stream of tokens to consume input from
    TokenStream stream;
    
    /**
     * Find the appropriate nonterminal
     *
     * Return nullptr if it could not be found
     */
    
    Isr *FindPhrase( );
    
    Isr *FindSimple( );
    
    Isr *FindAnd( );
    
    Isr *FindOr( );
    
    
public:
    
    /**
     * Construct parser based on given input
     */
    Parser( const std::string &in );
    
    bool fullParsed();
    
    /**
     * The public interface of the parser. Call this function,
     * rather than the private internal functions.
     */
    Isr *Parse( );
};
// class Parser

#endif /* PARSER_H_ */
