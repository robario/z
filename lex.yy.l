%{
int yywrap(void) {
    return 1;
}
%}
%option noinput
%option nounput
dotall (.|\n)
%%
{dotall} { return yytext[0]; }
