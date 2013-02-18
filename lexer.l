%option yylineno
%pointer

%{
#include <stdlib.h>
#include <string.h>
#include "ast.hpp"
#include "parser.hpp"

void yyerror(const char *);
%}

%x COMMENT
/* 
  WRITE ME: put any definitions you need here.  Add your own regular expressions
            below for ALL tokens defined in the manual. You can choose not to use
            definitions here.

*/

/*
  The example below is very simple, but it shows almost everything you will need
  to do for this first part.

  The syntax is as follows:

  regular_expression      {  c code  }

  The lexer will try to match each regular expression.  When it does it will execute
  the c code.

*/

%%

"[$"          { BEGIN(COMMENT); }
<COMMENT>"$]" { BEGIN(INITIAL); }
<COMMENT>.    ; /* skip comments */
<COMMENT>\n   ; /* skip comments */

bool     { return BOOL;     }
else     { return ELSE;     }
if       { return IF;       }
int      { return INT;      }
while    { return WHILE;    }
var      { return VAR;      }
function { return FUNCTION; }
intarray { return INTARRAY; }
return   { return RETURN;   }

==       { return EQEQ;     }
>=       { return GTEQ;     }
\<=      { return LEEQ;     }
!=       { return NOTEQ;    }
&&       { return AND;      }
\|\|     { return OR;       }

true     { return TRUE;  }
false    { return FALSE; }

[><!\+\-\*\/;,|{}()\[\]=] { return *yytext;    }
[01]+b                    { yylval.u_base_int = strtol(yytext, NULL, 2);
                            return NUMBER; /* binary */ }
0[xX][0-9A-F]+            { yylval.u_base_int = strtol(yytext, NULL, 16);
                            return NUMBER; /* hexadecimal */ }
0[0-7]+                   { yylval.u_base_int = strtol(yytext, NULL, 8);
                            return NUMBER; /* octal */ }
[0-9]+                    { yylval.u_base_int = strtol(yytext, NULL, 10);
                            return NUMBER; /* decimal */ }
[a-zA-Z][a-zA-Z0-9]*      { yylval.u_base_charptr =
                              (char*) malloc(sizeof(char)*strlen(yytext));
                            strcpy(yylval.u_base_charptr, yytext); 
                            return IDENTIFIER; }

[ \t\n]                   ; /* skip whitespace */
.                         yyerror((char *) "invalid character");

%%

/* You shouldn't need to edit any part of this*/

int yywrap(void) {
  return 1;
}

