#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

typedef struct Node Node;
typedef struct Token Token;
typedef struct Obj Obj;
typedef struct Type Type;
typedef struct Member Member;

// TokenKind
typedef enum
{
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_KEYWORD,  // keyword
    TK_STRING,   // string
    TK_EOF       // 入力の終わりを示すトークン
} TokenKind;

struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

extern Token *token;

// 型
typedef enum
{
    TY_INT,
    TY_PTR,
    TY_ARRAY,
    TY_CHAR,
    TY_STRUCT,
} TypeKind;

struct Type
{
    TypeKind kind;
    Type *ptr_to;
    int size; // sizeof() value

    // Array
    int array_len;

    // declaration
    Token *name;

    // struct
    Member *members;
};

extern Type *ty_int;
extern Type *ty_char;

// variable or function
struct Obj
{
    Obj *next;
    char *name;
    Type *ty;
    bool is_local; // local or global/function

    // local variable
    int offset;

    // global variable or function
    bool is_function;

    // funciton
    Obj *params;
    Node *body;
    Obj *locals;
    int stack_size;
};

// NodeKind
typedef enum
{
    ND_ADD,      // +
    ND_SUB,      // -
    ND_MUL,      // *
    ND_DIV,      // /
    ND_EQ,       // ==
    ND_NE,       // !=
    ND_LT,       // <
    ND_LE,       // <=
    ND_ASSIGN,   // =
    ND_LVAR,     // ローカル変数
    ND_GVAR,     // global variable
    ND_RETURN,   // return
    ND_IF,       // if
    ND_WHILE,    // while
    ND_FOR,      // for
    ND_BLOCK,    // {}
    ND_FUNCCALL, // call function
    ND_NUM,      // num literal
    ND_DEREF,    // *
    ND_ADDR,     // &
    ND_TYPE_DEF, // int or char(declaration)
    ND_STRING,   // string literal
    ND_MEMBER,   // member
} NodeKind;

struct Node
{
    NodeKind kind;
    Node *next;
    Type *ty; // Type, e.g. int or pointer to int

    Node *lhs;
    Node *rhs;

    // "if" or "for" statement
    Node *cond; // condition
    Node *then;
    Node *_else;
    Node *init;
    Node *inc; // increment

    // Block
    Node *body; //{...}

    // Function call
    char *funcname; // function name
    Node *args;     // arguments

    int val;        // Used if kind == ND_NUM
    Obj *var;       // Used if kind == ND_LVAR/ND_GVAR
    char *str;      // Used if kind == ND_STRING
    Member *member; // Used if kind == ND_STRUCT
};

struct Member
{
    char *name;
    int offset;
    int size;
    Type *ty;
    Member *next;
};

// generate
void gen_function(Obj *func);
void gen_gvar(Obj *gvar);

// ENBF
Obj *program();

// tokenize
Token *tokenize(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool is_ident1(char c);
bool is_ident2(char c);

// use token
bool at_eof();
int expect_number();
void expect_reserved(char *op);
bool consume_reserved(char *op);
bool consume_keyword(char *op);
Token *consume_ident();
Token *consume_string();

// trim
char *trim(char *s, int size_t);

// error
void error_at(char *loc, char *fmt, ...);
void error(char *, ...);

// type
Type *pointer_to(Type *ptr_to);
Type *array_of(Type *ptr_to, int len);
void add_type(Node *node);