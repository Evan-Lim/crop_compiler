#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    NODE_PROGRAM,
    NODE_SENSOR,
    NODE_SENSOR_BUS,
    NODE_OUTPUT,
    NODE_VAR,
    NODE_ARRAY_VAR,
    NODE_LET,
    NODE_ASSIGN,
    NODE_RULE,
    NODE_IF,
    NODE_ELSE,
    NODE_ELSE_IF,
    NODE_EVERY,
    NODE_FN,
    NODE_RETURN,
    NODE_MACHINE,
    NODE_STATE_DECL,
    NODE_INITIAL_STATE,
    NODE_ON_ENTER,
    NODE_TRANSITION_COND,
    NODE_TRANSITION_TIME,
    NODE_INVARIANT,
    NODE_EMIT,
    NODE_INIT,
    NODE_EXTERN,
    NODE_UNSAFE,
    NODE_INTEGER,
    NODE_FLOAT,
    NODE_BOOLEAN,
    NODE_IDENTIFIER,
    NODE_TYPE,
    NODE_FIXED_TYPE,
    NODE_BINOP,
    NODE_UNOP,
    NODE_FUNCTION_CALL,
    NODE_QUESTION,
    NODE_IS,
    NODE_RANGE,
    NODE_POLL,
    NODE_STATE_LITERAL,
    NODE_PARAM,
    NODE_NONE
} NodeType;

typedef enum {
    TYPE_U8, TYPE_U16, TYPE_U32, TYPE_U64,
    TYPE_I8, TYPE_I16, TYPE_I32, TYPE_I64,
    TYPE_F32, TYPE_F64,
    TYPE_BOOL,
    TYPE_FIXED,
    TYPE_ARRAY,
    TYPE_PAYLOAD,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_UNKNOWN
} DataType;

typedef struct ASTNode {
    NodeType type;
    DataType data_type;
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* next;
    struct ASTList* children;
    union {
        int integer;
        float float_val;
        char* string;
        bool boolean;
    } value;
    char* name;
    char* type_name;
    int fixed_w;
    int fixed_s;
    int array_size;
    int line;
    int column;
} ASTNode;

typedef struct ASTList {
    ASTNode* node;
    struct ASTList* next;
} ASTList;

ASTNode* create_node(NodeType type);
ASTNode* create_program(ASTList* statements);
ASTNode* create_integer(int value);
ASTNode* create_float(float value);
ASTNode* create_boolean(int value);
ASTNode* create_identifier(char* name);
ASTNode* create_type(char* name);
ASTNode* create_fixed_type(char* w, char* s);
ASTNode* create_sensor(char* name, char* mode, char* pin, ASTNode* range, ASTNode* poll);
ASTNode* create_sensor_bus(char* name, char* protocol, ASTNode* address, ASTNode* type, ASTNode* poll);
ASTNode* create_output(char* name, char* pin, ASTNode* state);
ASTNode* create_poll(ASTNode* time);
ASTNode* create_var(char* name, ASTNode* type, ASTNode* value);
ASTNode* create_array_var(char* name, ASTNode* type, int size, ASTNode* value);
ASTNode* create_let(char* name, ASTNode* value);
ASTNode* create_assignment(ASTNode* target, ASTNode* value);
ASTNode* create_rule(char* name, ASTList* body);
ASTNode* create_if(ASTNode* condition, ASTList* body, ASTNode* else_block);
ASTNode* create_else(ASTList* body);
ASTNode* create_else_if(ASTNode* condition, ASTList* body, ASTNode* next_else);
ASTNode* create_every(ASTNode* interval, ASTList* body);
ASTNode* create_fn(char* name, ASTList* params, ASTNode* return_type, ASTList* body);
ASTNode* create_param(char* name, ASTNode* type);
ASTNode* create_return(ASTNode* value);
ASTNode* create_machine(char* name, ASTList* body);
ASTNode* create_state_decl(char* name, ASTList* body);
ASTNode* create_initial_state(char* name, ASTList* body);
ASTNode* create_on_enter(ASTList* body);
ASTNode* create_transition_condition(char* target, ASTNode* condition);
ASTNode* create_transition_time(char* target, ASTNode* time);
ASTNode* create_invariant(char* name, ASTNode* condition);
ASTNode* create_emit(char* payload, ASTList* args, char* protocol, char* address);
ASTNode* create_init(ASTList* body);
ASTNode* create_extern(char* lang, char* name, ASTList* params, ASTNode* return_type);
ASTNode* create_unsafe(char* code);
ASTNode* create_binop(char* op, ASTNode* left, ASTNode* right);
ASTNode* create_unop(char* op, ASTNode* expr);
ASTNode* create_function_call(char* name, ASTList* args);
ASTNode* create_question(char* expr);
ASTNode* create_is_expr(ASTNode* left, char* right);
ASTNode* create_range(ASTNode* start, ASTNode* end);
ASTNode* create_state_literal(char* name);

ASTList* create_list(ASTNode* node, ASTList* next);
ASTList* append_list(ASTList* list, ASTNode* node);
void free_list(ASTList* list);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);
const char* node_type_name(NodeType type);
const char* data_type_name(DataType type);

#endif
