// ============================================================
// crop.y - CROP Grammar (Bison)
// ============================================================
// This file defines the grammar rules for the CROP language.
// Bison generates parser.c and parser.h.
// ============================================================

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Forward declarations
extern int yylineno;
extern int yycolumn;
extern char* yytext;
int yylex();

// Error function
void yyerror(const char* msg);

// The resulting AST
ASTNode* program_ast = NULL;

%}

// ============================================================
// Union for token values
// ============================================================

%union {
    int number;
    float float_val;
    char* string;
    ASTNode* node;
    ASTList* list;
}

// ============================================================
// Token declarations
// ============================================================

%token TOKEN_SENSOR TOKEN_OUTPUT TOKEN_VAR TOKEN_LET
%token TOKEN_RULE TOKEN_IF TOKEN_ELSE TOKEN_FOR TOKEN_EVERY
%token TOKEN_FN TOKEN_RETURN TOKEN_MACHINE TOKEN_STATE
%token TOKEN_INIT TOKEN_UNSAFE TOKEN_INVARIANT
%token TOKEN_REQUIRES TOKEN_ENSURE TOKEN_EMIT TOKEN_EXTERN
%token TOKEN_INITIAL TOKEN_TRANSITION TOKEN_ON_ENTER
%token TOKEN_WHEN TOKEN_AFTER
%token TOKEN_ON TOKEN_OFF
%token TOKEN_AT TOKEN_ARROW TOKEN_RANGE
%token TOKEN_QUESTION TOKEN_COLON TOKEN_ASSIGN
%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACE TOKEN_RBRACE
%token TOKEN_LBRACKET TOKEN_RBRACKET
%token TOKEN_SEMICOLON TOKEN_COMMA TOKEN_DOT
%token TOKEN_PLUS TOKEN_MINUS TOKEN_STAR TOKEN_SLASH TOKEN_PERCENT
%token TOKEN_NOT TOKEN_LT TOKEN_GT TOKEN_LTE TOKEN_GTE
%token TOKEN_EQ TOKEN_NEQ TOKEN_HASH

%token <number> TOKEN_INTEGER TOKEN_MS
%token <float_val> TOKEN_FLOAT
%token <string> TOKEN_IDENTIFIER
%token TOKEN_ERROR

// ============================================================
// Type declarations for grammar rules
// ============================================================

%type <node> program statement
%type <node> sensor_decl output_decl var_decl let_decl
%type <node> rule_decl if_stmt every_decl
%type <node> fn_decl return_stmt
%type <node> machine_decl state_decl transition_decl on_enter_decl
%type <node> invariant_decl emit_decl init_decl extern_decl
%type <node> expr primary_expr binop_expr unop_expr
%type <node> type_expr param_list opt_range opt_poll
%type <list> statement_list expr_list

// ============================================================
// Precedence rules
// ============================================================

%right TOKEN_ASSIGN
%left TOKEN_EQ TOKEN_NEQ
%left TOKEN_LT TOKEN_GT TOKEN_LTE TOKEN_GTE
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_STAR TOKEN_SLASH TOKEN_PERCENT
%right TOKEN_NOT

%%

// ============================================================
// Grammar rules
// ============================================================

program
    : statement_list {
        program_ast = create_program($1);
    }
    ;

statement_list
    : statement {
        $$ = create_list($1, NULL);
    }
    | statement_list statement {
        $$ = append_list($1, $2);
    }
    ;

statement
    : sensor_decl
    | output_decl
    | var_decl
    | let_decl
    | rule_decl
    | if_stmt
    | every_decl
    | fn_decl
    | return_stmt
    | machine_decl
    | invariant_decl
    | emit_decl
    | init_decl
    | extern_decl
    | unsafe_block
    ;

// ============================================================
// Hardware & I/O
// ============================================================

sensor_decl
    : TOKEN_SENSOR TOKEN_IDENTIFIER TOKEN_IDENTIFIER TOKEN_IDENTIFIER opt_range opt_poll {
        // Pin mode: sensor name pin pin_id [range] [@poll=time]
        $$ = create_sensor($2, "pin", $4, $5, $6);
    }
    | TOKEN_SENSOR TOKEN_IDENTIFIER TOKEN_IDENTIFIER TOKEN_LPAREN expr TOKEN_RPAREN TOKEN_ARROW type_expr opt_poll {
        // Bus mode: sensor name protocol(address) -> type [@poll=time]
        $$ = create_sensor_bus($2, $3, $5, $8, $9);
    }
    ;

output_decl
    : TOKEN_OUTPUT TOKEN_IDENTIFIER TOKEN_IDENTIFIER TOKEN_IDENTIFIER opt_state {
        $$ = create_output($2, $4, $5);
    }
    ;

opt_range
    : /* empty */ { $$ = NULL; }
    | TOKEN_RANGE expr TOKEN_DOT TOKEN_DOT expr {
        $$ = create_range($2, $5);
    }
    ;

opt_poll
    : /* empty */ { $$ = NULL; }
    | TOKEN_AT TOKEN_IDENTIFIER TOKEN_ASSIGN expr {
        $$ = create_poll($4);
    }
    ;

opt_state
    : /* empty */ { $$ = NULL; }
    | TOKEN_ON { $$ = create_state("ON"); }
    | TOKEN_OFF { $$ = create_state("OFF"); }
    ;

// ============================================================
// Data & Variables
// ============================================================

var_decl
    : TOKEN_VAR TOKEN_IDENTIFIER TOKEN_COLON type_expr TOKEN_ASSIGN expr {
        $$ = create_var($2, $4, $6);
    }
    | TOKEN_VAR TOKEN_IDENTIFIER TOKEN_COLON TOKEN_LBRACKET type_expr TOKEN_SEMICOLON TOKEN_INTEGER TOKEN_RBRACKET TOKEN_ASSIGN TOKEN_LBRACKET expr TOKEN_SEMICOLON TOKEN_INTEGER TOKEN_RBRACKET {
        $$ = create_array_var($2, $5, $7, $11);
    }
    ;

let_decl
    : TOKEN_LET TOKEN_IDENTIFIER TOKEN_ASSIGN expr {
        $$ = create_let($2, $4);
    }
    ;

type_expr
    : TOKEN_IDENTIFIER { $$ = create_type($1); }
    | TOKEN_FIXED LT TOKEN_IDENTIFIER TOKEN_COMMA TOKEN_IDENTIFIER GT {
        $$ = create_fixed_type($3, $5);
    }
    ;

// ============================================================
// Reactive Control Flow
// ============================================================

rule_decl
    : TOKEN_RULE TOKEN_STRING TOKEN_COLON statement_list {
        $$ = create_rule($2, $4);
    }
    ;

if_stmt
    : TOKEN_IF expr TOKEN_COLON statement_list opt_else {
        $$ = create_if($2, $4, $5);
    }
    ;

opt_else
    : /* empty */ { $$ = NULL; }
    | TOKEN_ELSE TOKEN_COLON statement_list { $$ = create_else($3); }
    | TOKEN_ELSE TOKEN_IF expr TOKEN_COLON statement_list opt_else {
        $$ = create_else_if($3, $5, $6);
    }
    ;

every_decl
    : TOKEN_EVERY expr TOKEN_COLON statement_list {
        $$ = create_every($2, $4);
    }
    ;

// ============================================================
// Computative Layer
// ============================================================

fn_decl
    : TOKEN_FN TOKEN_IDENTIFIER TOKEN_LPAREN param_list TOKEN_RPAREN TOKEN_ARROW type_expr TOKEN_COLON statement_list {
        $$ = create_fn($2, $4, $7, $9);
    }
    ;

param_list
    : /* empty */ { $$ = NULL; }
    | param { $$ = create_list($1, NULL); }
    | param_list TOKEN_COMMA param {
        $$ = append_list($1, $3);
    }
    ;

param
    : TOKEN_IDENTIFIER TOKEN_COLON type_expr {
        $$ = create_param($1, $3);
    }
    ;

return_stmt
    : TOKEN_RETURN expr {
        $$ = create_return($2);
    }
    | TOKEN_RETURN {
        $$ = create_return(NULL);
    }
    ;

// ============================================================
// State Machines
// ============================================================

machine_decl
    : TOKEN_MACHINE TOKEN_IDENTIFIER TOKEN_COLON statement_list {
        $$ = create_machine($2, $4);
    }
    ;

state_decl
    : TOKEN_STATE TOKEN_IDENTIFIER TOKEN_COLON statement_list {
        $$ = create_state_decl($2, $4);
    }
    | TOKEN_INITIAL TOKEN_STATE TOKEN_IDENTIFIER TOKEN_COLON statement_list {
        $$ = create_initial_state($3, $5);
    }
    ;

on_enter_decl
    : TOKEN_ON_ENTER TOKEN_COLON statement_list {
        $$ = create_on_enter($3);
    }
    ;

transition_decl
    : TOKEN_TRANSITION TOKEN_TO TOKEN_IDENTIFIER TOKEN_WHEN expr {
        $$ = create_transition_condition($3, $5);
    }
    | TOKEN_TRANSITION TOKEN_TO TOKEN_IDENTIFIER TOKEN_AFTER expr {
        $$ = create_transition_time($3, $5);
    }
    ;

// ============================================================
// Formal Verification
// ============================================================

invariant_decl
    : TOKEN_INVARIANT TOKEN_STRING TOKEN_COLON expr {
        $$ = create_invariant($2, $4);
    }
    ;

// ============================================================
// Network Communication
// ============================================================

emit_decl
    : TOKEN_EMIT TOKEN_IDENTIFIER TOKEN_LPAREN expr_list TOKEN_RPAREN TOKEN_TO TOKEN_IDENTIFIER TOKEN_LPAREN TOKEN_STRING TOKEN_RPAREN {
        $$ = create_emit($2, $4, $7, $9);
    }
    ;

// ============================================================
// Startup / Initialisation
// ============================================================

init_decl
    : TOKEN_INIT TOKEN_COLON statement_list {
        $$ = create_init($3);
    }
    ;

// ============================================================
// Interoperability
// ============================================================

extern_decl
    : TOKEN_EXTERN TOKEN_STRING TOKEN_FN TOKEN_IDENTIFIER TOKEN_LPAREN param_list TOKEN_RPAREN TOKEN_ARROW type_expr {
        $$ = create_extern($2, $4, $6, $9);
    }
    ;

unsafe_block
    : TOKEN_UNSAFE TOKEN_LBRACE TOKEN_STRING TOKEN_RBRACE {
        $$ = create_unsafe($3);
    }
    ;

// ============================================================
// Expressions
// ============================================================

expr
    : primary_expr
    | binop_expr
    | unop_expr
    | TOKEN_IDENTIFIER TOKEN_QUESTION {
        $$ = create_question($1);
    }
    | expr TOKEN_IS TOKEN_IDENTIFIER {
        $$ = create_is_expr($1, $3);
    }
    ;

primary_expr
    : TOKEN_IDENTIFIER { $$ = create_identifier($1); }
    | TOKEN_INTEGER { $$ = create_integer($1); }
    | TOKEN_FLOAT { $$ = create_float($1); }
    | TOKEN_ON { $$ = create_state("ON"); }
    | TOKEN_OFF { $$ = create_state("OFF"); }
    | TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
    | TOKEN_IDENTIFIER TOKEN_LPAREN expr_list TOKEN_RPAREN {
        $$ = create_function_call($1, $3);
    }
    ;

binop_expr
    : expr TOKEN_PLUS expr { $$ = create_binop("+", $1, $3); }
    | expr TOKEN_MINUS expr { $$ = create_binop("-", $1, $3); }
    | expr TOKEN_STAR expr { $$ = create_binop("*", $1, $3); }
    | expr TOKEN_SLASH expr { $$ = create_binop("/", $1, $3); }
    | expr TOKEN_PERCENT expr { $$ = create_binop("%", $1, $3); }
    | expr TOKEN_LT expr { $$ = create_binop("<", $1, $3); }
    | expr TOKEN_GT expr { $$ = create_binop(">", $1, $3); }
    | expr TOKEN_LTE expr { $$ = create_binop("<=", $1, $3); }
    | expr TOKEN_GTE expr { $$ = create_binop(">=", $1, $3); }
    | expr TOKEN_EQ expr { $$ = create_binop("==", $1, $3); }
    | expr TOKEN_NEQ expr { $$ = create_binop("!=", $1, $3); }
    ;

unop_expr
    : TOKEN_NOT expr { $$ = create_unop("not", $2); }
    | TOKEN_MINUS expr { $$ = create_unop("-", $2); }
    ;

expr_list
    : expr { $$ = create_list($1, NULL); }
    | expr_list TOKEN_COMMA expr { $$ = append_list($1, $3); }
    ;

%%

// ============================================================
// Error handling
// ============================================================

void yyerror(const char* msg) {
    fprintf(stderr, "ERROR at line %d, column %d: %s\n", yylineno, yycolumn, msg);
}
