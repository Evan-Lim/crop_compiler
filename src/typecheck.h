// ============================================================
// typecheck.h - Type Checking for CROP Compiler
// ============================================================
// Performs semantic analysis and type checking
// ============================================================

#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "ast.h"
#include "symbol.h"
#include "options.h"
#include <stdbool.h>

// ============================================================
// Function prototypes
// ============================================================

bool semantic_analyze(ASTNode* node, CompilerOptions* opts);

// Type checking functions
bool check_types(ASTNode* node);
DataType get_expr_type(ASTNode* node);
bool is_numeric_type(DataType type);
bool is_integer_type(DataType type);
bool is_float_type(DataType type);
bool types_compatible(DataType a, DataType b);
DataType wider_type(DataType a, DataType b);

// Specific checks
bool check_binop_types(ASTNode* node);
bool check_assignment(ASTNode* node);
bool check_function_call(ASTNode* node);
bool check_array_access(ASTNode* node);
bool check_sensor_read(ASTNode* node);

// Error reporting
void type_error(const char* msg, ASTNode* node);
void type_warning(const char* msg, ASTNode* node);

// Helper functions
const char* type_to_string(DataType type);
DataType string_to_type(const char* str);

#endif
