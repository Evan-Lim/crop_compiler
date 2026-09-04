// ============================================================
// typecheck.c - Type Checking Implementation
// ============================================================

#include "typecheck.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// Error tracking
// ============================================================

static int error_count = 0;
static int warning_count = 0;

// ============================================================
// Helper functions
// ============================================================

const char* type_to_string(DataType type) {
    switch (type) {
        case TYPE_U8: return "u8";
        case TYPE_U16: return "u16";
        case TYPE_U32: return "u32";
        case TYPE_U64: return "u64";
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_F32: return "f32";
        case TYPE_F64: return "f64";
        case TYPE_BOOL: return "bool";
        case TYPE_FIXED: return "fixed";
        case TYPE_ARRAY: return "array";
        case TYPE_PAYLOAD: return "payload";
        case TYPE_VOID: return "void";
        default: return "unknown";
    }
}

DataType string_to_type(const char* str) {
    if (strcmp(str, "u8") == 0) return TYPE_U8;
    if (strcmp(str, "u16") == 0) return TYPE_U16;
    if (strcmp(str, "u32") == 0) return TYPE_U32;
    if (strcmp(str, "u64") == 0) return TYPE_U64;
    if (strcmp(str, "i8") == 0) return TYPE_I8;
    if (strcmp(str, "i16") == 0) return TYPE_I16;
    if (strcmp(str, "i32") == 0) return TYPE_I32;
    if (strcmp(str, "i64") == 0) return TYPE_I64;
    if (strcmp(str, "f32") == 0) return TYPE_F32;
    if (strcmp(str, "f64") == 0) return TYPE_F64;
    if (strcmp(str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(str, "void") == 0) return TYPE_VOID;
    return TYPE_UNKNOWN;
}

bool is_numeric_type(DataType type) {
    return (type >= TYPE_U8 && type <= TYPE_F64) || type == TYPE_FIXED;
}

bool is_integer_type(DataType type) {
    return (type >= TYPE_U8 && type <= TYPE_I64);
}

bool is_float_type(DataType type) {
    return (type == TYPE_F32 || type == TYPE_F64);
}

bool types_compatible(DataType a, DataType b) {
    if (a == TYPE_UNKNOWN || b == TYPE_UNKNOWN) return true;
    if (a == b) return true;
    if (is_numeric_type(a) && is_numeric_type(b)) return true;
    return false;
}

DataType wider_type(DataType a, DataType b) {
    // Return the "wider" type
    if (a == TYPE_F64 || b == TYPE_F64) return TYPE_F64;
    if (a == TYPE_F32 || b == TYPE_F32) return TYPE_F32;
    if (a == TYPE_U64 || b == TYPE_U64) return TYPE_U64;
    if (a == TYPE_I64 || b == TYPE_I64) return TYPE_I64;
    if (a == TYPE_U32 || b == TYPE_U32) return TYPE_U32;
    if (a == TYPE_I32 || b == TYPE_I32) return TYPE_I32;
    if (a == TYPE_U16 || b == TYPE_U16) return TYPE_U16;
    if (a == TYPE_I16 || b == TYPE_I16) return TYPE_I16;
    if (a == TYPE_U8 || b == TYPE_U8) return TYPE_U8;
    if (a == TYPE_I8 || b == TYPE_I8) return TYPE_I8;
    return TYPE_UNKNOWN;
}

// ============================================================
// Error reporting
// ============================================================

void type_error(const char* msg, ASTNode* node) {
    error_count++;
    fprintf(stderr, "TYPE ERROR");
    if (node) {
        fprintf(stderr, " at line %d, column %d", node->line, node->column);
    }
    fprintf(stderr, ": %s\n", msg);
}

void type_warning(const char* msg, ASTNode* node) {
    warning_count++;
    fprintf(stderr, "WARNING");
    if (node) {
        fprintf(stderr, " at line %d, column %d", node->line, node->column);
    }
    fprintf(stderr, ": %s\n", msg);
}

// ============================================================
// Type checking functions
// ============================================================

bool semantic_analyze(ASTNode* node, CompilerOptions* opts) {
    error_count = 0;
    warning_count = 0;
    
    // Create global scope if not exists
    if (!global_scope) {
        push_scope("global");
    }
    
    // First pass: collect declarations (symbols)
    // Second pass: type check
    // Third pass: verify
    
    if (!check_types(node)) {
        return false;
    }
    
    if (error_count > 0) {
        return false;
    }
    
    return true;
}

bool check_types(ASTNode* node) {
    if (!node) return true;
    
    bool result = true;
    
    // Check children first
    if (node->children) {
        ASTList* list = node->children;
        while (list) {
            if (!check_types(list->node)) {
                result = false;
            }
            list = list->next;
        }
    }
    
    // Check left and right
    if (node->left && !check_types(node->left)) result = false;
    if (node->right && !check_types(node->right)) result = false;
    
    // Now check this node
    switch (node->type) {
        case NODE_PROGRAM:
            break;
            
        case NODE_SENSOR:
        case NODE_SENSOR_BUS:
            // Sensor declaration - add to symbol table
            if (!lookup_symbol(node->name)) {
                Symbol* sym = create_symbol(node->name, SYM_SENSOR, TYPE_UNKNOWN);
                sym->pin = node->left ? strdup(node->left->name) : NULL;
                if (node->right && node->right->type == NODE_RANGE) {
                    sym->has_range = true;
                    sym->range_start = node->right->left;
                    sym->range_end = node->right->right;
                }
                if (node->children) {
                    ASTList* list = node->children;
                    while (list) {
                        if (list->node->type == NODE_POLL && list->node->left) {
                            sym->poll_ms = list->node->left->value.integer;
                        }
                        list = list->next;
                    }
                }
                if (node->type == NODE_SENSOR_BUS) {
                    sym->protocol = node->value.string ? strdup(node->value.string) : NULL;
                    sym->data_type = node->right ? node->right->data_type : TYPE_UNKNOWN;
                }
                add_symbol(sym);
            } else {
                type_error("Sensor already declared", node);
                result = false;
            }
            break;
            
        case NODE_OUTPUT:
            // Output declaration - add to symbol table
            if (!lookup_symbol(node->name)) {
                Symbol* sym = create_symbol(node->name, SYM_OUTPUT, TYPE_BOOL);
                sym->pin = node->left ? strdup(node->left->name) : NULL;
                if (node->right && node->right->type == NODE_STATE_LITERAL) {
                    // Initial state
                }
                add_symbol(sym);
            } else {
                type_error("Output already declared", node);
                result = false;
            }
            break;
            
        case NODE_VAR:
            // Variable declaration
            if (!lookup_symbol(node->name)) {
                Symbol* sym = create_symbol(node->name, SYM_VAR, node->left->data_type);
                if (node->right) {
                    sym->is_initialized = true;
                    // Check type compatibility with initializer
                    DataType init_type = get_expr_type(node->right);
                    if (!types_compatible(sym->data_type, init_type)) {
                        type_error("Type mismatch in variable initialization", node);
                        result = false;
                    }
                }
                add_symbol(sym);
            } else {
                type_error("Variable already declared", node);
                result = false;
            }
            break;
            
        case NODE_ARRAY_VAR:
            // Array variable declaration
            {
                char name_buf[256];
                sprintf(name_buf, "%s[%d]", node->name, node->array_size);
                if (!lookup_symbol(node->name)) {
                    Symbol* sym = create_symbol(node->name, SYM_VAR, TYPE_ARRAY);
                    sym->array_size = node->array_size;
                    // Store element type
                    if (node->left) {
                        sym->data_type = node->left->data_type;
                    }
                    add_symbol(sym);
                } else {
                    type_error("Array already declared", node);
                    result = false;
                }
            }
            break;
            
        case NODE_LET:
            // Local immutable variable
            {
                Symbol* sym = create_symbol(node->name, SYM_LET, TYPE_UNKNOWN);
                if (node->right) {
                    sym->data_type = get_expr_type(node->right);
                    sym->is_initialized = true;
                    sym->is_mutable = false;
                }
                add_symbol(sym);
            }
            break;
            
        case NODE_FN:
            // Function declaration
            {
                Symbol* sym = create_symbol(node->name, SYM_FN, node->left->data_type);
                sym->return_type = node->left->data_type;
                // Store parameters
                if (node->right && node->right->children) {
                    sym->params = NULL;
                    // We need to create symbols for params
                    ASTList* list = node->right->children;
                    Symbol* last = NULL;
                    while (list) {
                        if (list->node->type == NODE_PARAM) {
                            Symbol* param = create_symbol(list->node->name, SYM_PARAM, list->node->data_type);
                            if (!last) {
                                sym->params = param;
                            } else {
                                last->next = param;
                            }
                            last = param;
                            // Also add to current scope
                            add_symbol(param);
                        }
                        list = list->next;
                    }
                }
                add_symbol(sym);
                
                // Push scope for function body
                push_scope(node->name);
            }
            break;
            
        case NODE_RETURN:
            // Check return type matches function
            if (node->left) {
                DataType return_type = get_expr_type(node->left);
                // Need to check against enclosing function's return type
                // This would require tracking the current function context
            }
            break;
            
        case NODE_MACHINE:
            // State machine declaration
            if (!lookup_symbol(node->name)) {
                Symbol* sym = create_symbol(node->name, SYM_MACHINE, TYPE_UNKNOWN);
                add_symbol(sym);
                // Push scope for state machine
                push_scope(node->name);
            } else {
                type_error("Machine already declared", node);
                result = false;
            }
            break;
            
        case NODE_STATE_DECL:
        case NODE_INITIAL_STATE:
            // State declaration
            {
                Symbol* machine = lookup_symbol_in_scope(current_scope, current_scope->name);
                if (machine) {
                    Symbol* state_sym = create_symbol(node->name, SYM_STATE, TYPE_UNKNOWN);
                    // Add to machine's states list
                    state_sym->next = machine->states;
                    machine->states = state_sym;
                    if (node->type == NODE_INITIAL_STATE) {
                        machine->initial_state = state_sym;
                    }
                }
                // Push scope for state
                push_scope(node->name);
            }
            break;
            
        case NODE_ON_ENTER:
            // on_enter block - check inside state scope
            break;
            
        case NODE_TRANSITION_COND:
            // Transition with condition - check condition is boolean
            if (node->left) {
                DataType cond_type = get_expr_type(node->left);
                if (cond_type != TYPE_BOOL) {
                    type_error("Transition condition must be boolean", node);
                    result = false;
                }
            }
            break;
            
        case NODE_TRANSITION_TIME:
            // Transition with time - check time is numeric
            if (node->left) {
                DataType time_type = get_expr_type(node->left);
                if (!is_numeric_type(time_type)) {
                    type_error("Transition time must be numeric", node);
                    result = false;
                }
            }
            break;
            
        case NODE_INVARIANT:
            // Invariant - check condition is boolean
            if (node->left) {
                DataType cond_type = get_expr_type(node->left);
                if (cond_type != TYPE_BOOL) {
                    type_error("Invariant condition must be boolean", node);
                    result = false;
                }
            }
            break;
            
        case NODE_EMIT:
            // Emit - check payload exists and arguments match
            {
                Symbol* payload = lookup_symbol(node->name);
                if (!payload || payload->sym_type != SYM_PAYLOAD) {
                    type_error("Undefined payload type", node);
                    result = false;
                } else {
                    // Check argument count matches field count
                    int arg_count = 0;
                    ASTList* args = node->children;
                    while (args) {
                        arg_count++;
                        args = args->next;
                    }
                    int field_count = 0;
                    Symbol* field = payload->fields;
                    while (field) {
                        field_count++;
                        field = field->next;
                    }
                    if (arg_count != field_count) {
                        type_error("Argument count doesn't match payload fields", node);
                        result = false;
                    }
                }
            }
            break;
            
        case NODE_INIT:
            // init block - no special checking
            break;
            
        case NODE_EXTERN:
            // External function
            {
                Symbol* sym = create_symbol(node->name, SYM_EXTERN, node->left->data_type);
                sym->return_type = node->left->data_type;
                sym->extern_lang = node->value.string ? strdup(node->value.string) : strdup("C");
                // Check for bounded attribute
                if (node->right && node->right->type == NODE_BINOP) {
                    // Parse bounded(N)
                }
                add_symbol(sym);
            }
            break;
            
        case NODE_UNSAFE:
            // unsafe block - no type checking
            break;
            
        // Expressions
        case NODE_INTEGER:
            node->data_type = TYPE_U32;
            break;
            
        case NODE_FLOAT:
            if (options.no_fpu) {
                type_error("Floating point disabled with --no-fpu", node);
                result = false;
            }
            node->data_type = TYPE_F32;
            break;
            
        case NODE_IDENTIFIER:
            // Look up symbol
            {
                Symbol* sym = lookup_symbol(node->name);
                if (!sym) {
                    type_error("Undefined identifier", node);
                    result = false;
                } else {
                    node->data_type = sym->data_type;
                }
            }
            break;
            
        case NODE_BINOP:
            // Binary operation - check operand types
            if (node->left && node->right) {
                DataType left_type = get_expr_type(node->left);
                DataType right_type = get_expr_type(node->right);
                
                if (!types_compatible(left_type, right_type)) {
                    type_error("Incompatible types in binary operation", node);
                    result = false;
                }
                
                // Determine result type
                if (is_comparison_op(node->value.string)) {
                    node->data_type = TYPE_BOOL;
                } else {
                    node->data_type = wider_type(left_type, right_type);
                }
            }
            break;
            
        case NODE_UNOP:
            // Unary operation
            if (node->left) {
                DataType expr_type = get_expr_type(node->left);
                if (strcmp(node->value.string, "!") == 0) {
                    if (expr_type != TYPE_BOOL) {
                        type_error("Logical NOT requires boolean operand", node);
                        result = false;
                    }
                    node->data_type = TYPE_BOOL;
                } else if (strcmp(node->value.string, "-") == 0) {
                    if (!is_numeric_type(expr_type)) {
                        type_error("Unary minus requires numeric operand", node);
                        result = false;
                    }
                    node->data_type = expr_type;
                }
            }
            break;
            
        case NODE_FUNCTION_CALL:
            // Function call - check function exists
            {
                Symbol* sym = lookup_symbol(node->name);
                if (!sym || (sym->sym_type != SYM_FN && sym->sym_type != SYM_EXTERN)) {
                    type_error("Undefined function", node);
                    result = false;
                } else {
                    node->data_type = sym->return_type;
                    // Check argument count matches parameter count
                    // ... (implementation left as exercise)
                }
            }
            break;
            
        case NODE_QUESTION:
            // ? operator - safe unwrap
            {
                Symbol* sym = lookup_symbol(node->name);
                if (!sym || sym->sym_type != SYM_SENSOR) {
                    type_error("? can only be used on sensors", node);
                    result = false;
                } else {
                    node->data_type = sym->data_type;
                }
            }
            break;
            
        case NODE_IS:
            // is operator - check left is an output
            if (node->left && node->left->type == NODE_IDENTIFIER) {
                Symbol* sym = lookup_symbol(node->left->name);
                if (!sym || sym->sym_type != SYM_OUTPUT) {
                    type_error("'is' can only be used on outputs", node);
                    result = false;
                }
                node->data_type = TYPE_BOOL;
            }
            break;
            
        case NODE_RANGE:
            // Range - check both ends are numeric
            if (node->left && node->right) {
                DataType left_type = get_expr_type(node->left);
                DataType right_type = get_expr_type(node->right);
                if (!is_numeric_type(left_type) || !is_numeric_type(right_type)) {
                    type_error("Range bounds must be numeric", node);
                    result = false;
                }
                node->data_type = TYPE_UNKNOWN;
            }
            break;
            
        case NODE_POLL:
            // Poll - check time is numeric
            if (node->left) {
                DataType time_type = get_expr_type(node->left);
                if (!is_numeric_type(time_type)) {
                    type_error("Poll time must be numeric", node);
                    result = false;
                }
            }
            break;
            
        case NODE_STATE_LITERAL:
            // State literal (ON/OFF)
            node->data_type = TYPE_BOOL;
            break;
            
        default:
            break;
    }
    
    // Pop scopes after processing blocks
    if (node->type == NODE_FN || node->type == NODE_MACHINE || 
        node->type == NODE_STATE_DECL || node->type == NODE_INITIAL_STATE ||
        node->type == NODE_RULE || node->type == NODE_EVERY) {
        // Don't pop immediately - let the recursive traversal handle it
    }
    
    return result;
}

DataType get_expr_type(ASTNode* node) {
    if (!node) return TYPE_UNKNOWN;
    
    // If node already has type computed, return it
    if (node->data_type != TYPE_UNKNOWN) return node->data_type;
    
    // Compute type based on node type
    switch (node->type) {
        case NODE_INTEGER:
            return TYPE_U32;
        case NODE_FLOAT:
            return TYPE_F32;
        case NODE_IDENTIFIER: {
            Symbol* sym = lookup_symbol(node->name);
            if (sym) return sym->data_type;
            return TYPE_UNKNOWN;
        }
        case NODE_BINOP:
            if (is_comparison_op(node->value.string)) {
                return TYPE_BOOL;
            }
            if (node->left && node->right) {
                DataType lt = get_expr_type(node->left);
                DataType rt = get_expr_type(node->right);
                return wider_type(lt, rt);
            }
            return TYPE_UNKNOWN;
        case NODE_UNOP:
            if (node->left) {
                if (strcmp(node->value.string, "!") == 0) return TYPE_BOOL;
                return get_expr_type(node->left);
            }
            return TYPE_UNKNOWN;
        case NODE_FUNCTION_CALL: {
            Symbol* sym = lookup_symbol(node->name);
            if (sym) return sym->return_type;
            return TYPE_UNKNOWN;
        }
        case NODE_QUESTION: {
            Symbol* sym = lookup_symbol(node->name);
            if (sym) return sym->data_type;
            return TYPE_UNKNOWN;
        }
        case NODE_IS:
            return TYPE_BOOL;
        case NODE_STATE_LITERAL:
            return TYPE_BOOL;
        default:
            return TYPE_UNKNOWN;
    }
}

bool is_comparison_op(const char* op) {
    return (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
            strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0 ||
            strcmp(op, "==") == 0 || strcmp(op, "!=") == 0);
}
