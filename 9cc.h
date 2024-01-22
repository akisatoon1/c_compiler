#include <stdbool.h>

extern char *user_input;

typedef enum
{
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_CONTROLS, // 制御文字列
    TK_TYPE,     // 型
    TK_RETURN,   // return
    TK_EOF       // 入力の終わりを示すトークン
} TokenKind;

typedef struct Token Token;

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
typedef struct Type Type;
struct Type
{
    enum
    {
        TY_INT,
        TY_PTR
    } ty;
    Type *ptr_to;
};

typedef struct LVar LVar;

struct LVar
{
    LVar *next;
    char *name;
    Type *ty;
    int len;
    int offset;
};

extern LVar *locals;

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
    ND_RETURN,   // return
    ND_IF,       // if
    ND_WHILE,    // while
    ND_FOR,      // for
    ND_BLOCK,    //{}
    ND_FUNCCALL, // call function
    ND_FUNC_DEF, // function definition
    ND_NUM,      // Integer
    ND_DEREF,    // *
    ND_ADDR,     // &
    ND_TYPE_DEF, // int
} NodeKind;

typedef struct Node Node;

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
    char *funcname;
    Node *args; // arguments

    int val;   // Used if kind == ND_NUM
    LVar *var; // Used if kind == ND_VAR

    int stack_size;
};

extern Node *code[100];

extern int Lend;
extern int Lelse;
extern int Lbegin;

// 引数のレジスタ名
extern char *argreg[];

// generate
void gen(Node *node);
void gen_lval(Node *node);
void gen_function(Node *node);
void gen_stmt(Node *node);
void gen_expr(Node *node);

// ENBF
void program();
Node *function();
Node *stmt();
// Node *compound_stmt();
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
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

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
bool consume_type(char *s);
void expect_type(char *s);
bool consume_return();

// error
void error_at(char *loc, char *fmt, ...);
void error(char *, ...);

// var
LVar *find_lvar(Token *tok);

// trim
char *trim(char *s, int size_t);

// align
int align_to(int n, int align);