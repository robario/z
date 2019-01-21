%{
int yywrap(void) {
    return 1;
}
%}
%option noinput
%option nounput
%%
. { return yytext[0]; }
