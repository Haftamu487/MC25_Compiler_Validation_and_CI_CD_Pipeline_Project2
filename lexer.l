%option noinput nounput


%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "y.tab.h"
%}

%%

"int"           { return INT; }

"return"        { return RETURN; }

"if"            { return IF; }

"else"          { return ELSE; }

"while"         { return WHILE; }

[0-9]+          {
                    yylval.num = atoi(yytext);
                    return NUMBER;
                }

[a-zA-Z_][a-zA-Z0-9_]* {
                    yylval.str = strdup(yytext);
                    return ID;
                }

"("             { return '('; }

")"             { return ')'; }

"{"             { return '{'; }

"}"             { return '}'; }

";"             { return ';'; }

"="             { return '='; }

"<"             { return '<'; }

">"             { return '>'; }

"+"             { return '+'; }

"-"             { return '-'; }

"*"             { return '*'; }

"/"             { return '/'; }

[ \t\n]+        { /* Ignore whitespace */ }

.               {
                    printf("Unknown character: %s\n", yytext);
                    return yytext[0];
                }

%%

int yywrap(void)
{
    return 1;
}
