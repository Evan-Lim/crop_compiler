// ============================================================
// crop.l - CROP Lexical Analyzer (Flex)
// ============================================================
// This file defines the tokens for the CROP language.
// Flex generates lexer.c which provides yylex() function.
// ============================================================

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"  // Token definitions from Bison

// Helper to create string tokens
#define YYSTYPE char*

// Location tracking
int yylineno = 1;
int yycolumn = 1;

// Forward declaration
extern int yylex();
extern void yyerror(const char* msg);

%}

%option noyywrap
%option yylineno

// ============================================================
// Helper definitions (regular expressions)
// ============================================================

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

%%

// ============================================================
// Keywords
// ============================================================

sensor       { return TOKEN_SENSOR; }
output       { return TOKEN_OUTPUT; }
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

// State machine keywords
initial      { return TOKEN_INITIAL; }
transition   { return TOKEN_TRANSITION; }
on_enter     { return TOKEN_ON_ENTER; }
when         { return TOKEN_WHEN; }
after        { return TOKEN_AFTER; }

// Output states
ON           { return TOKEN_ON; }
OFF          { return TOKEN_OFF; }

// ============================================================
// Identifiers and literals
// ============================================================

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

// ============================================================
// Special syntax
// ============================================================

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

// ============================================================
// Comments and whitespace
// ============================================================

{COMMENT}     { /* ignore */ }
{ML_COMMENT}  { /* ignore */ }
{WHITESPACE}  { /* ignore */ }

// Newline tracking
\n {
    yylineno++;
    yycolumn = 1;
}

// ============================================================
// Error handling
// ============================================================

. {
    fprintf(stderr, "ERROR at line %d, column %d: Unexpected character '%s'\n",
            yylineno, yycolumn, yytext);
    return TOKEN_ERROR;
}

%%

// ============================================================
// Helper function for error reporting
// ============================================================

void yyerror(const char* msg) {
    fprintf(stderr, "ERROR at line %d, column %d: %s\n", yylineno, yycolumn, msg);
}
