extern char *user_input;

typedef enum
{
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
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

typedef enum
{
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_ASSIGN, // =
    ND_LVAR,   // ローカル変数
    ND_NUM,    // Integer
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset;
};

extern Node *code[100];

typedef struct LVar LVar;

struct LVar
{
    LVar *next;
    char *name;
    int len;
    int offset;
};

extern LVar *locals;

// generate
void gen(Node *node);
void gen_lval(Node *node);

// ENBF
void program();
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
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

// tokenize
Token *tokenize(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool is_ident1(char c);
bool is_ident2(char c);

// use token
bool at_eof();
int expect_number();
void expect(char *op);
bool consume(char *op);
Token *consume_ident();

// error
void error_at(char *, char *, ...);
void error(char *, ...);

// var
LVar *find_lvar(Token *tok);