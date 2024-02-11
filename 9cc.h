#include <stdbool.h>

extern char *user_input;

typedef struct Node Node;
typedef struct Token Token;
typedef struct Obj Obj;
typedef struct Type Type;

// TokenKind
typedef enum
{
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_CONTROLS, // 制御文字列
    TK_TYPE,     // 型
    TK_SIZEOF,   // sizeof
    TK_RETURN,   // return
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
} TypeKind;

struct Type
{
    TypeKind kind;
    Type *ptr_to;
    int size; // sizeof() value

    // Array
    int array_len;
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

extern Obj *locals;
extern Obj *globals;

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
    ND_NUM,      // Integer
    ND_DEREF,    // *
    ND_ADDR,     // &
    ND_TYPE_DEF, // int
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

    int val;  // Used if kind == ND_NUM
    Obj *var; // Used if kind == ND_VAR
};

// label id
extern int Lend;
extern int Lelse;
extern int Lbegin;

// 引数のレジスタ名
extern char *argreg_64[];
extern char *argreg_32[];

// generate
void gen(Node *node);
void gen_lval_address(Node *node);
void gen_gvar_address(Node *node);
void gen_stmt(Node *node);
void gen_function(Obj *func);
void gen_gvar(Obj *gvar);
void gen_expr(Node *node);

// ENBF
Obj *program();
Obj *global_variable(Type *ty, Token *tok);
Obj *function_def(Type *ty, Token *tok);
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// create node of tree
Node *new_node_num(int);
Node *new_node(NodeKind kind, Node *lhs);
Node *new_node_lvar(Node *node, Token *tok);
Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);

// create new variable
Obj *new_lvar(Token *tok, Type *ty);
Obj *new_gvar(Token *tok, Type *ty);

// create new type
Type *new_type(Type *type);

// tokenize
Token *tokenize(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool is_ident1(char c);
bool is_ident2(char c);
bool is_alnum(char c);

// キーワードsかどうか判定する。
bool is_keyword(char *p, char *s);

// use token
bool at_eof();
int expect_number();
void expect_reserved(char *op);
bool consume_reserved(char *op);
Token *consume_ident();
bool consume_controls(char *s);
bool consume_sizeof();
bool consume_type(char *s);
void expect_type(char *s);
bool consume_return();

// error
void error_at(char *loc, char *fmt, ...);
void error(char *, ...);

// find
Obj *find_lvar(Token *tok);
Obj *find_gvar(Token *tok);

// trim
char *trim(char *s, int size_t);

// align
int align_to(int n, int align);