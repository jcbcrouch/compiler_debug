/*
 * driver.cpp
 *
 * Driver for the math parser program
 *
 * Lab3: You do not have to modify this file, but you may choose to do so
 *
 * If you do, note that your output is expected to match ours
 */

#include <iostream>

#include "expression.h"
#include "parser.h"

int main( )
{
    std::string input;
    while(std::getline( std::cin, input )) {
        
        Parser parser( input );
        ISR *expr = parser.Parse( );
        if ( expr  && parser.fullParsed())
        {
            std::cout << expr->nextInstance( ) << "\n";
            //for(int i = 0; i < expr->queries.size(); i++) {
              //  for(int j = 0; j < expr->queries[i]->queries.size(); j++) {
                //    std::cout << expr->queries[i]->queries[j] << "\n";
                //}
                std::cout << "\n";
            //}
            
            delete expr;
        }
        else
        {
            help_message();
        }
    }
}
