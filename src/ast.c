// ============================================================
// ast.c - AST Implementation
// ============================================================
// This file implements all the AST creation and manipulation
// functions declared in ast.h.
// ============================================================

#include "ast.h"
#include <stdarg.h>

// ============================================================
// Node creation
// ============================================================

ASTNode* create_node(NodeType type) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = type;
    node->data_type = TYPE_UNKNOWN;
    node->line = 0;  // Will be filled by lexer later
    node->column = 0;
    return node;
}

ASTNode* create_program(ASTList* statements) {
    ASTNode* node = create_node(NODE_PROGRAM);
    node->children = statements;
    return node;
}

ASTNode* create_integer(int value) {
    ASTNode* node = create_node(NODE_INTEGER);
    node->value.integer = value;
    node->data_type = TYPE_U32;
    return node;
}

ASTNode* create_float(float value) {
    ASTNode* node = create_node(NODE_FLOAT);
    node->value.float_val = value;
    node->data_type = TYPE_F32;
    return node;
}

ASTNode* create_identifier(char* name) {
    ASTNode* node = create_node(NODE_IDENTIFIER);
    node->name = strdup(name);
    return node;
}

ASTNode* create_type(char* name) {
    ASTNode* node = create_node(NODE_TYPE);
    node->name = strdup(name);
    
    // Map type name to DataType enum
    if (strcmp(name, "u8") == 0) node->data_type = TYPE_U8;
    else if (strcmp(name, "u16") == 0) node->data_type = TYPE_U16;
    else if (strcmp(name, "u32") == 0) node->data_type = TYPE_U32;
    else if (strcmp(name, "u64") == 0) node->data_type = TYPE_U64;
    else if (strcmp(name, "i8") == 0) node->data_type = TYPE_I8;
    else if (strcmp(name, "i16") == 0) node->data_type = TYPE_I16;
    else if (strcmp(name, "i32") == 0) node->data_type = TYPE_I32;
    else if (strcmp(name, "i64") == 0) node->data_type = TYPE_I64;
    else if (strcmp(name, "f32") == 0) node->data_type = TYPE_F32;
    else if (strcmp(name, "f64") == 0) node->data_type = TYPE_F64;
    else if (strcmp(name, "bool") == 0) node->data_type = TYPE_BOOL;
    
    return node;
}

ASTNode* create_fixed_type(char* w, char* s) {
    ASTNode* node = create_node(NODE_FIXED_TYPE);
    node->fixed_w = atoi(w);
    node->fixed_s = atoi(s);
    node->data_type = TYPE_FIXED;
    return node;
}

// ============================================================
// Hardware nodes
// ============================================================

ASTNode* create_sensor(char* name, char* mode, char* pin, ASTNode* range, ASTNode* poll) {
    ASTNode* node = create_node(NODE_SENSOR);
    node->name = strdup(name);
    node->left = create_identifier(pin);
    node->right = range;
    node->children = create_list(poll, NULL);
    return node;
}

ASTNode* create_sensor_bus(char* name, char* protocol, ASTNode* address, ASTNode* type, ASTNode* poll) {
    ASTNode* node = create_node(NODE_SENSOR_BUS);
    node->name = strdup(name);
    node->value.string = strdup(protocol);
    node->left = address;
    node->right = type;
    node->children = create_list(poll, NULL);
    return node;
}

ASTNode* create_output(char* name, char* pin, ASTNode* state) {
    ASTNode* node = create_node(NODE_OUTPUT);
    node->name = strdup(name);
    node->left = create_identifier(pin);
    node->right = state;
    return node;
}

ASTNode* create_poll(ASTNode* time) {
    ASTNode* node = create_node(NODE_POLL);
    node->left = time;
    return node;
}

// ============================================================
// Data nodes
// ============================================================

ASTNode* create_var(char* name, ASTNode* type, ASTNode* value) {
    ASTNode* node = create_node(NODE_VAR);
    node->name = strdup(name);
    node->left = type;
    node->right = value;
    node->data_type = type->data_type;
    return node;
}

ASTNode* create_array_var(char* name, ASTNode* type, int size, ASTNode* value) {
    ASTNode* node = create_node(NODE_ARRAY_VAR);
    node->name = strdup(name);
    node->left = type;
    node->right = value;
    node->array_size = size;
    node->data_type = TYPE_ARRAY;
    return node;
}

ASTNode* create_let(char* name, ASTNode* value) {
    ASTNode* node = create_node(NODE_LET);
    node->name = strdup(name);
    node->right = value;
    return node;
}

// ============================================================
// Control flow nodes
// ============================================================

ASTNode* create_rule(char* name, ASTList* body) {
    ASTNode* node = create_node(NODE_RULE);
    node->value.string = strdup(name);
    node->children = body;
    return node;
}

ASTNode* create_if(ASTNode* condition, ASTList* body, ASTNode* else_block) {
    ASTNode* node = create_node(NODE_IF);
    node->left = condition;
    node->children = body;
    node->right = else_block;
    return node;
}

ASTNode* create_else(ASTList* body) {
    ASTNode* node = create_node(NODE_ELSE);
    node->children = body;
    return node;
}

ASTNode* create_else_if(ASTNode* condition, ASTList* body, ASTNode* next_else) {
    ASTNode* node = create_node(NODE_ELSE_IF);
    node->left = condition;
    node->children = body;
    node->right = next_else;
    return node;
}

ASTNode* create_every(ASTNode* interval, ASTList* body) {
    ASTNode* node = create_node(NODE_EVERY);
    node->left = interval;
    node->children = body;
    return node;
}

// ============================================================
// Function nodes
// ============================================================

ASTNode* create_fn(char* name, ASTList* params, ASTNode* return_type, ASTList* body) {
    ASTNode* node = create_node(NODE_FN);
    node->name = strdup(name);
    node->left = return_type;
    node->children = body;
    node->right = create_node(NODE_NONE);
    node->right->children = params;
    return node;
}

ASTNode* create_param(char* name, ASTNode* type) {
    ASTNode* node = create_node(NODE_PARAM);
    node->name = strdup(name);
    node->left = type;
    node->data_type = type->data_type;
    return node;
}

ASTNode* create_return(ASTNode* value) {
    ASTNode* node = create_node(NODE_RETURN);
    node->left = value;
    return node;
}

// ============================================================
// State machine nodes
// ============================================================

ASTNode* create_machine(char* name, ASTList* body) {
    ASTNode* node = create_node(NODE_MACHINE);
    node->name = strdup(name);
    node->children = body;
    return node;
}

ASTNode* create_state_decl(char* name, ASTList* body) {
    ASTNode* node = create_node(NODE_STATE_DECL);
    node->name = strdup(name);
    node->children = body;
    return node;
}

ASTNode* create_initial_state(char* name, ASTList* body) {
    ASTNode* node = create_node(NODE_INITIAL_STATE);
    node->name = strdup(name);
    node->children = body;
    return node;
}

ASTNode* create_on_enter(ASTList* body) {
    ASTNode* node = create_node(NODE_ON_ENTER);
    node->children = body;
    return node;
}

ASTNode* create_transition_condition(char* target, ASTNode* condition) {
    ASTNode* node = create_node(NODE_TRANSITION_COND);
    node->value.string = strdup(target);
    node->left = condition;
    return node;
}

ASTNode* create_transition_time(char* target, ASTNode* time) {
    ASTNode* node = create_node(NODE_TRANSITION_TIME);
    node->value.string = strdup(target);
    node->left = time;
    return node;
}

// ============================================================
// Verification nodes
// ============================================================

ASTNode* create_invariant(char* name, ASTNode* condition) {
    ASTNode* node = create_node(NODE_INVARIANT);
    node->value.string = strdup(name);
    node->left = condition;
    return node;
}

// ============================================================
// Network nodes
// ============================================================

ASTNode* create_emit(char* payload, ASTList* args, char* protocol, char* address) {
    ASTNode* node = create_node(NODE_EMIT);
    node->name = strdup(payload);
    node->children = args;
    node->value.string = strdup(protocol);
    node->left = create_identifier(address);
    return node;
}

// ============================================================
// Other nodes
// ============================================================

ASTNode* create_init(ASTList* body) {
    ASTNode* node = create_node(NODE_INIT);
    node->children = body;
    return node;
}

ASTNode* create_extern(char* lang, char* name, ASTList* params, ASTNode* return_type) {
    ASTNode* node = create_node(NODE_EXTERN);
    node->value.string = strdup(lang);
    node->name = strdup(name);
    node->children = params;
    node->left = return_type;
    return node;
}

ASTNode* create_unsafe(char* code) {
    ASTNode* node = create_node(NODE_UNSAFE);
    node->value.string = strdup(code);
    return node;
}

// ============================================================
// Expression nodes
// ============================================================

ASTNode* create_binop(char* op, ASTNode* left, ASTNode* right) {
    ASTNode* node = create_node(NODE_BINOP);
    node->value.string = strdup(op);
    node->left = left;
    node->right = right;
    return node;
}

ASTNode* create_unop(char* op, ASTNode* expr) {
    ASTNode* node = create_node(NODE_UNOP);
    node->value.string = strdup(op);
    node->left = expr;
    return node;
}

ASTNode* create_function_call(char* name, ASTList* args) {
    ASTNode* node = create_node(NODE_FUNCTION_CALL);
    node->name = strdup(name);
    node->children = args;
    return node;
}

ASTNode* create_question(char* expr) {
    ASTNode* node = create_node(NODE_QUESTION);
    node->name = strdup(expr);
    return node;
}

ASTNode* create_is_expr(ASTNode* left, char* right) {
    ASTNode* node = create_node(NODE_IS);
    node->left = left;
    node->value.string = strdup(right);
    return node;
}

ASTNode* create_range(ASTNode* start, ASTNode* end) {
    ASTNode* node = create_node(NODE_RANGE);
    node->left = start;
    node->right = end;
    return node;
}

ASTNode* create_state_literal(char* name) {
    ASTNode* node = create_node(NODE_STATE_LITERAL);
    node->value.string = strdup(name);
    return node;
}

// ============================================================
// List functions
// ============================================================

ASTList* create_list(ASTNode* node, ASTList* next) {
    ASTList* list = (ASTList*)malloc(sizeof(ASTList));
    list->node = node;
    list->next = next;
    return list;
}

ASTList* append_list(ASTList* list, ASTNode* node) {
    if (!list) return create_list(node, NULL);
    ASTList* current = list;
    while (current->next) current = current->next;
    current->next = create_list(node, NULL);
    return list;
}

void free_list(ASTList* list) {
    if (!list) return;
    free_list(list->next);
    free_ast(list->node);
    free(list);
}

// ============================================================
// Utility functions
// ============================================================

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free_list(node->children);
    if (node->name) free(node->name);
    if (node->value.string && node->type != NODE_IDENTIFIER) free(node->value.string);
    free(node);
}

const char* node_type_name(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "Program";
        case NODE_SENSOR: return "Sensor";
        case NODE_SENSOR_BUS: return "SensorBus";
        case NODE_OUTPUT: return "Output";
        case NODE_VAR: return "Var";
        case NODE_ARRAY_VAR: return "ArrayVar";
        case NODE_LET: return "Let";
        case NODE_RULE: return "Rule";
        case NODE_IF: return "If";
        case NODE_ELSE: return "Else";
        case NODE_ELSE_IF: return "ElseIf";
        case NODE_EVERY: return "Every";
        case NODE_FN: return "Fn";
        case NODE_RETURN: return "Return";
        case NODE_MACHINE: return "Machine";
        case NODE_STATE_DECL: return "State";
        case NODE_INITIAL_STATE: return "InitialState";
        case NODE_ON_ENTER: return "OnEnter";
        case NODE_TRANSITION_COND: return "TransitionCondition";
        case NODE_TRANSITION_TIME: return "TransitionTime";
        case NODE_INVARIANT: return "Invariant";
        case NODE_EMIT: return "Emit";
        case NODE_INIT: return "Init";
        case NODE_EXTERN: return "Extern";
        case NODE_UNSAFE: return "Unsafe";
        case NODE_INTEGER: return "Integer";
        case NODE_FLOAT: return "Float";
        case NODE_IDENTIFIER: return "Identifier";
        case NODE_TYPE: return "Type";
        case NODE_FIXED_TYPE: return "FixedType";
        case NODE_BINOP: return "BinOp";
        case NODE_UNOP: return "UnOp";
        case NODE_FUNCTION_CALL: return "FunctionCall";
        case NODE_QUESTION: return "Question";
        case NODE_IS: return "Is";
        case NODE_RANGE: return "Range";
        case NODE_POLL: return "Poll";
        case NODE_STATE_LITERAL: return "StateLiteral";
        case NODE_PARAM: return "Param";
        case NODE_NONE: return "None";
        default: return "Unknown";
    }
}

const char* data_type_name(DataType type) {
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
