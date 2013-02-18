#include "ast.hpp"
#include "parser.hpp"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

extern int yydebug; // set this to 1 if you want yyparse to dump a trace
extern int yyparse(); // this actually the parser which then calls the scanner
extern int yylex();
extern void print_token(int);

void dopass_ast2dot(Program_ptr ast); /*this is defined in ast2dot.cpp*/
Program_ptr ast; /* make sure to set this to the final syntax tree in parser.ypp*/

void printUsage(char* command) {
  printf("Usage: %s [--debug] [--only-scanner]\n", command);
}

int main(int argc, char** argv) {
  bool onlyScanner = false;

  static struct option long_options[] = {
    {"debug", no_argument, 0, 'd'},
    {"only-scanner", no_argument, 0, 's'},
    {0, 0, 0, 0}
  };

  int long_index = 0, opt = 0;
  while((opt = getopt_long(
          argc, argv, "ds", long_options, &long_index)) != -1) {
    switch(opt) {
      case 'd': yydebug = 1; break;
      case 's': onlyScanner = true; break;
      default: printUsage(argv[0]); exit(EXIT_FAILURE); break;
    }
  }

  if(onlyScanner) {
    unsigned int token;
    while((token = yylex()) != 0) {
      print_token(token);
    }
    printf("\n");
  } else {
    yyparse();
 
    // walk over the ast and print it out as a dot file
    if (ast) dopass_ast2dot( ast );
  }

  return 0;

}

