%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.h"

int yycolumn = 1;
static int indent_stack[64] = {0};
static int indent_top = 0;
static int pending_indent = 0;
static int pending_dedents = 0;

int yylex_impl(void);
extern void yyerror(const char* msg);

#define YY_DECL int yylex_impl(void)

%}

%option noyywrap
%option yylineno

DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+
TIME_MS     {INTEGER}ms
TIME_S      {INTEGER}s
TIME_MINS   {INTEGER}mins
HEX         0x{DIGIT}+
COMMENT     "//".*
ML_COMMENT  "/*"([^*]|\*+[^*/])*\*+"/"
WHITESPACE  [ \t\r]+
STRING      \"[^\"\\]*(\\.[^\"\\]*)*\"

%%

sensor       { return TOKEN_SENSOR; }
output       { return TOKEN_OUTPUT; }
actuator     { return TOKEN_OUTPUT; }
var          { return TOKEN_VAR; }
let          { return TOKEN_LET; }
rule         { return TOKEN_RULE; }
if           { return TOKEN_IF; }
else         { return TOKEN_ELSE; }
for          { return TOKEN_FOR; }
every        { return TOKEN_EVERY; }
fn           { return TOKEN_FN; }
return       { return TOKEN_RETURN; }
machine      { return TOKEN_MACHINE; }
state        { return TOKEN_STATE; }
init         { return TOKEN_INIT; }
unsafe       { return TOKEN_UNSAFE; }
invariant    { return TOKEN_INVARIANT; }
requires     { return TOKEN_REQUIRES; }
ensures      { return TOKEN_ENSURE; }
emit         { return TOKEN_EMIT; }
extern       { return TOKEN_EXTERN; }
initial      { return TOKEN_INITIAL; }
transition   { return TOKEN_TRANSITION; }
on_enter     { return TOKEN_ON_ENTER; }
when         { return TOKEN_WHEN; }
after        { return TOKEN_AFTER; }
ON           { return TOKEN_ON; }
OFF          { return TOKEN_OFF; }
fixed        { return TOKEN_FIXED; }
to           { return TOKEN_TO; }
is           { return TOKEN_IS; }
true         { return TOKEN_TRUE; }
false        { return TOKEN_FALSE; }

{IDENTIFIER} {
    yylval.string = strdup(yytext);
    return TOKEN_IDENTIFIER;
}

{INTEGER} {
    yylval.number = atoi(yytext);
    return TOKEN_INTEGER;
}

{FLOAT} {
    yylval.float_val = atof(yytext);
    return TOKEN_FLOAT;
}

{HEX} {
    sscanf(yytext + 2, "%x", &yylval.number);
    return TOKEN_INTEGER;
}

{TIME_MS} {
    yylval.number = atoi(yytext);
    return TOKEN_MS;
}

{TIME_S} {
    yylval.number = atoi(yytext) * 1000;
    return TOKEN_MS;
}

{TIME_MINS} {
    yylval.number = atoi(yytext) * 60000;
    return TOKEN_MS;
}

{STRING} {
    yylval.string = strdup(yytext + 1);
    yylval.string[strlen(yylval.string)-1] = '\0';
    return TOKEN_STRING;
}

"@" { return TOKEN_AT; }
"->" { return TOKEN_ARROW; }
".." { return TOKEN_RANGE; }
"<=" { return TOKEN_LTE; }
">=" { return TOKEN_GTE; }
"==" { return TOKEN_EQ; }
"!=" { return TOKEN_NEQ; }
"?"  { return TOKEN_QUESTION; }
":"  { return TOKEN_COLON; }
"="  { return TOKEN_ASSIGN; }
"("  { return TOKEN_LPAREN; }
")"  { return TOKEN_RPAREN; }
"{"  { return TOKEN_LBRACE; }
"}"  { return TOKEN_RBRACE; }
"["  { return TOKEN_LBRACKET; }
"]"  { return TOKEN_RBRACKET; }
";"  { return TOKEN_SEMICOLON; }
","  { return TOKEN_COMMA; }
"."  { return TOKEN_DOT; }
"+"  { return TOKEN_PLUS; }
"-"  { return TOKEN_MINUS; }
"*"  { return TOKEN_STAR; }
"/"  { return TOKEN_SLASH; }
"%"  { return TOKEN_PERCENT; }
"!"  { return TOKEN_NOT; }
"<"  { return TOKEN_LT; }
">"  { return TOKEN_GT; }
"#"  { return TOKEN_HASH; }

{COMMENT}     { /* ignore */ }
{ML_COMMENT}  { /* ignore */ }
{WHITESPACE}  { yycolumn += yyleng; }

\n[ \t]* {
    int width = 0;
    for (size_t i = 1; i < yyleng; i++)
        width += yytext[i] == '\t' ? 4 : 1;

    int next = input();
    if (next != 0) unput(next);
    if (next == '\n' || next == '/')
        return TOKEN_NEWLINE;

    if (width > indent_stack[indent_top]) {
        if (indent_top == 63) {
            fprintf(stderr, "ERROR at line %d: indentation is too deep\n", yylineno);
            return TOKEN_ERROR;
        }
        indent_stack[++indent_top] = width;
        pending_indent = 1;
    } else if (width < indent_stack[indent_top]) {
        while (indent_top > 0 && width < indent_stack[indent_top]) {
            indent_top--;
            pending_dedents++;
        }
        if (width != indent_stack[indent_top]) {
            fprintf(stderr, "ERROR at line %d: inconsistent indentation\n", yylineno);
            return TOKEN_ERROR;
        }
    }
    yycolumn = width + 1;
    return TOKEN_NEWLINE;
}

. {
    fprintf(stderr, "ERROR at line %d, column %d: Unexpected character '%s'\n",
            yylineno, yycolumn, yytext);
    return TOKEN_ERROR;
}

%%

int yylex(void) {
    if (pending_indent) {
        pending_indent = 0;
        return TOKEN_INDENT;
    }
    if (pending_dedents) {
        pending_dedents--;
        return TOKEN_DEDENT;
    }

    int token = yylex_impl();
    if (token == 0 && indent_top > 0) {
        indent_top--;
        return TOKEN_DEDENT;
    }
    return token;
}
