// ============================================================
// codegen.h - Code Generation for CROP Compiler
// ============================================================
// Generates C code from the AST
// ============================================================

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "symbol.h"
#include "options.h"
#include <stdio.h>
#include <stdbool.h>

// ============================================================
// Function prototypes
// ============================================================

bool check_invariants(ASTNode* node);
bool generate_c_code(ASTNode* node, const char* output_file, CompilerOptions* opts);

// Code generation functions
void generate_program(ASTNode* node);
void generate_declaration(ASTNode* node);
void generate_statement(ASTNode* node);
void generate_expression(ASTNode* node);
void generate_sensor(ASTNode* node);
void generate_output(ASTNode* node);
void generate_var(ASTNode* node);
void generate_let(ASTNode* node);
void generate_rule(ASTNode* node);
void generate_if(ASTNode* node);
void generate_every(ASTNode* node);
void generate_fn(ASTNode* node);
void generate_machine(ASTNode* node);
void generate_state(ASTNode* node);
void generate_transition(ASTNode* node);
void generate_invariant(ASTNode* node);
void generate_emit(ASTNode* node);
void generate_init(ASTNode* node);
void generate_unsafe(ASTNode* node);
void generate_extern(ASTNode* node);

// Expression generation
void generate_expr(ASTNode* node);

// Helper functions
const char* c_type_name(DataType type);
const char* sanitize_name(const char* name);
void indent(int level);
int get_indent_level();

#endif
