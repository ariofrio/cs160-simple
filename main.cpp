#include "ast.hpp"
#include "parser.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "typecheck.cpp"
#include "constantfolding.cpp"
#include "codegen.cpp"
#include <assert.h>
#include <getopt.h>

extern int yydebug; // set this to 1 if you want yyparse to dump a trace
extern int yyparse(); // this actually the parser which then calls the scanner

void dopass_ast2dot(Program_ptr ast); /*this is defined in ast2dot.cpp*/

void dopass_typecheck(Program_ptr ast, SymTab* st) {
  Typecheck* typecheck = new Typecheck(stderr, st); //create the visitor
  ast->accept(typecheck); //walk the tree with the visitor above
  delete typecheck;
}

void dopass_constantfolding(Program_ptr ast, SymTab* st) {
  ConstantFolding* constant_folding = new ConstantFolding(stderr, st); //create the visitor
  LatticeElemMap *map = new LatticeElemMap();
  map = ast->accept(constant_folding, map); //walk the tree with the visitor above
  delete map;
  delete constant_folding;
}

void dopass_codegen(Program_ptr ast, SymTab * st)
{
	Codegen *codegen = new Codegen(stdout, st);	// create the visitor
	ast->accept(codegen);				// walk the tree with the visitor above
	delete codegen;
}

Program_ptr ast; /* make sure to set this to the final syntax tree in parser.ypp*/

void printUsage(char* command) {
  printf("Usage: %s [--debug]\n", command);
}

int main(int argc, char** argv) {
  SymTab st; //symbol table

  static struct option long_options[] = {
    {"debug", no_argument, 0, 'd'},
    {0, 0, 0, 0}
  };

  int long_index = 0, opt = 0;
  while((opt = getopt_long(
          argc, argv, "d", long_options, &long_index)) != -1) {
    switch(opt) {
      case 'd': yydebug = 1; break;
      default: printUsage(argv[0]); exit(EXIT_FAILURE); break;
    }
  }

	// after parsing, the global "ast" should be set to the
	// syntax tree that we have built up during the parse
  yyparse();  

	if (ast) {
		dopass_typecheck( ast, &st );
		dopass_constantfolding(ast, &st);

		// do codegen!
		dopass_codegen( ast, &st );
	}
  return 0;
}

