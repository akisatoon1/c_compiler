#include "9cc.h"

// variable vector
static Obj *locals;
static Obj *globals;

// func
Function *funcs;
static void new_func(char *name, Type *ty);

// ebnf
static Type *declspec();
static Type *declarator(Type *type);
static Type *type_suffix(Type *ty);
static Type *struct_def();
static Member *struct_members();
static Obj *global_variable(Type *ty, Token *tok);
static Obj *function_def(Type *ty, Token *tok);
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *postfix();
static Node *primary();

// create node of tree
static Node *new_node_num(int);
static Node *new_node(NodeKind kind, Node *lhs);
static Node *new_node_var(Node *node, Token *tok);
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_add(Node *lhs, Node *rhs);
static Node *new_node_sub(Node *lhs, Node *rhs);

// create new variable
static Obj *new_lvar(Token *tok, Type *ty);
static Obj *new_gvar(Token *tok, Type *ty);

// find
static Obj *find_lvar(Token *tok);
static Obj *find_gvar(Token *tok);
static Member *find_member(Token *tok, Member *members);
Type *find_func(char *name);

// create member
static Member *create_new_member(Type *ty, Member *mems);

// align
static int align_to(int n, int align);

// (8,16)=>16, (17,16)=>32, (9,8)=>16
int align_to(int n, int align)
{
    return (n - 1 + align) / align * align;
}

void new_func(char *name, Type *ty)
{
    Function *func = calloc(1, sizeof(Function));
    func->name = name;
    func->ty = ty;
    func->next = funcs;
    funcs = func;
    return;
}

Node *new_node(NodeKind kind, Node *lhs)
{
    if (!lhs)
        error_at(token->str, "lhs is NULL in new_node_binary");
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    return node;
}

// rとlが全く同じ型
// (何回ポインタを指すか、最終的に指し示す型が何かが完全一致している型。)
// である前提。
Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    if (!lhs)
        error_at(token->str, "lhs is NULL in new_node_binary");
    if (!rhs)
        error_at(token->str, "rhs is NULL in new_node_binary");

    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    add_type(node);
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    add_type(node);
    return node;
}

Node *new_node_var(Node *node, Token *tok)
{
    if (!node)
        error_at(token->str, "node is NULL in new_node_var");
    if (!tok)
        error_at(token->str, "tok is NULL in new_node_var");
    Obj *lvar = find_lvar(tok);
    if (lvar)
    {
        node->kind = ND_LVAR;
        node->var = lvar;
        add_type(node);
        return node;
    }
    else
    {
        Obj *gvar = find_gvar(tok);

        if (gvar)
        {
            node->kind = ND_GVAR;
            node->var = gvar;
            add_type(node);
            return node;
        }
        else
            error_at(tok->str, "'%s'は宣言されていない変数名です。in parse.c new_node_var()", trim(tok->str, tok->len));
    }
}

Obj *new_lvar(Token *tok, Type *ty)
{
    if (!tok)
        error_at(token->str, "tok is NULL in new_lvar");
    Obj *lvar = calloc(1, sizeof(Obj));

    lvar->next = locals;
    lvar->name = trim(tok->str, tok->len);
    lvar->ty = ty;
    lvar->is_local = true;
    lvar->offset = locals->offset + ty->size;

    return lvar;
}

Obj *new_gvar(Token *tok, Type *ty)
{
    if (!tok)
        error_at(token->str, "tok is NULL in new_gvar");
    if (!ty)
        error_at(token->str, "ty is NULL in new_gvar");
    Obj *gvar = calloc(1, sizeof(Obj));

    gvar->name = trim(tok->str, tok->len);
    gvar->ty = ty;
    gvar->is_local = false;
    gvar->is_function = false;

    return gvar;
}

// declspec = "int"
//          | "char"
//          | "struct" struct_def
Type *declspec()
{
    if (consume_keyword("int"))
        return ty_int;
    else if (consume_keyword("char"))
        return ty_char;
    else if (consume_keyword("struct"))
        return struct_def();
    else
        return NULL;
}

// struct_def = "{" struct_members
Type *struct_def()
{
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_STRUCT;

    expect_reserved("{");
    Member *members = struct_members();

    ty->size = members->offset + members->size;
    ty->members = members;
    return ty;
}

// struct_members = ( declspec declarator type_suffix ";" )* "}"
Member *struct_members()
{
    Member *members = calloc(1, sizeof(Member));
    while (!consume_reserved("}"))
    {
        Type *base_ty = declspec();
        if (!base_ty)
            error_at(token->str, "base_ty is NULL in struct_def");

        Type *member_ty = declarator(base_ty);
        if (!member_ty)
            error_at(token->str, "member_ty is NULL in struct_def");

        member_ty = type_suffix(member_ty);

        Member *member = create_new_member(member_ty, members);
        members = member;

        expect_reserved(";");
    }
    return members;
}

// declarator = "*"* ident
Type *declarator(Type *ty)
{
    if (!ty)
        error_at(token->str, "type is NULL in declarator");
    while (consume_reserved("*"))
    {
        ty = pointer_to(ty);
    }

    ty->name = consume_ident();
    if (!ty->name)
        error_at(token->str, "識別子がありません。in declarator");
    return ty;
}

// type_suffix = "[" num "]" type_suffix
//             | ε
Type *type_suffix(Type *base_ty)
{
    if (!base_ty)
        error_at(token->str, "base_ty is null in type_suffix");

    if (consume_reserved("["))
    {
        int array_len = expect_number();
        expect_reserved("]");
        base_ty = type_suffix(base_ty);
        return array_of(base_ty, array_len);
    }
    return base_ty;
}

// program = (declspec declarator ( "(" function-def | global-variable ) )*
Obj *program()
{
    globals = calloc(1, sizeof(Obj));
    funcs = calloc(1, sizeof(Function));

    // header file内のfuncはここでfuncsに追加する。
    new_func("printf", ty_void);

    while (!at_eof())
    {
        // トークンを読み込む
        Type *base_type = declspec();
        if (!base_type)
            error_at(token->str, "存在しない型です。in parse.c program()");
        Type *ty = declarator(base_type);

        if (consume_reserved("("))
        {
            locals = calloc(1, sizeof(Obj));
            Obj *func = function_def(ty, ty->name);
            func->locals = locals;
            func->next = globals;
            globals = func;
        }
        else
        {
            Obj *global_var = global_variable(ty, ty->name);
            global_var->next = globals;
            globals = global_var;
        }
    }

    // function or gvar vector
    return globals;
}

// globale-variable = type_suffix ";"
Obj *global_variable(Type *ty, Token *tok_ident)
{
    Obj *gvar;
    ty = type_suffix(ty);
    gvar = new_gvar(tok_ident, ty);
    expect_reserved(";");
    return gvar;
}

// function-def = ")" "{" stmt* "}"
//          | declspec  declarator ("," declspec  declarator)* ")" "{" stmt* "}"
Obj *function_def(Type *return_ty, Token *tok_ident)
{
    new_func(trim(tok_ident->str, tok_ident->len), return_ty);

    Obj *func = calloc(1, sizeof(Obj));
    func->is_local = false;
    func->is_function = true;
    func->ty = return_ty;

    if (!tok_ident)
    {
        error("関数名がありません。");
    }

    func->name = trim(tok_ident->str, tok_ident->len);

    while (!consume_reserved(")"))
    {
        // トークンを読み込む
        Type *base_type = declspec();
        if (!base_type)
            error_at(token->str, "存在しない型です。parse.c:225");
        Type *ty = declarator(base_type);

        Obj *lvar = find_lvar(ty->name);
        if (lvar)
        {
            error("既に使われているローカル変数です。");
        }
        else
        {
            lvar = new_lvar(ty->name, ty);
            locals = lvar;
        }

        consume_reserved(",");
    }

    func->params = locals; // params vector
    func->body = stmt();
    func->stack_size = align_to(locals->offset, 16);

    return func;
}

// stmt = expr ";"
//      | declspec declarator type_suffix ("=" expr )? ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "{" stmt* "}"
Node *stmt()
{
    Node *node;
    Type *base_type = declspec();
    if (base_type)
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        Node head = {};
        Node *cur = &head;

        Type *ty = declarator(base_type);
        ty = type_suffix(ty);

        Obj *lvar = find_lvar(ty->name);
        if (lvar)
        {
            error_at(ty->name->str, "'%s'は既に使われている変数または配列名です。", trim(ty->name->str, ty->name->len));
        }
        lvar = new_lvar(ty->name, ty);
        node->var = lvar;
        locals = lvar;

        if (consume_reserved("="))
        {
            cur = cur->next = new_node_binary(ND_ASSIGN, new_node_var(calloc(1, sizeof(Node)), ty->name), expr());
        }
        node->body = head.next;
        expect_reserved(";");
        return node;
    }
    else if (consume_keyword("return"))
    {
        node = new_node(ND_RETURN, expr());
        expect_reserved(";");
    }
    else if (consume_reserved("{"))
    {
        Node head;
        head.next = NULL;
        Node *cur = &head;
        while (!consume_reserved("}"))
        {
            cur->next = stmt();
            cur = cur->next;
        }
        cur->next = NULL;
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->body = head.next;
    }
    else if (consume_keyword("if"))
    {
        expect_reserved("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->cond = expr();
        expect_reserved(")");
        node->then = stmt();
        if (consume_keyword("else"))
        {
            node->_else = stmt();
        }
        else
        {
            node->_else = NULL;
        }
    }
    else if (consume_keyword("while"))
    {
        expect_reserved("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->cond = expr();
        expect_reserved(")");
        node->then = stmt();
    }
    else if (consume_keyword("for"))
    {
        expect_reserved("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        if (consume_reserved(";"))
        {
            node->init = NULL;
        }
        else
        {
            node->init = expr();
            expect_reserved(";");
        }
        if (consume_reserved(";"))
        {
            node->cond = NULL;
        }
        else
        {
            node->cond = expr();
            expect_reserved(";");
        }
        if (consume_reserved(")"))
        {
            node->inc = NULL;
        }
        else
        {
            node->inc = expr();
            expect_reserved(")");
        }
        node->then = stmt();
    }
    else
    {
        node = expr();
        expect_reserved(";");
    }
    return node;
}

// expr = assign
Node *expr()
{
    return assign();
}

// assign = equality ("=" assign)?
Node *assign()
{
    Node *node = equality();
    if (consume_reserved("="))
    {
        node = new_node_binary(ND_ASSIGN, node, assign());
    }
    add_type(node);
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume_reserved("=="))
        {
            node = new_node_binary(ND_EQ, node, relational());
        }
        else if (consume_reserved("!="))
        {
            node = new_node_binary(ND_NE, node, relational());
        }
        else
        {
            add_type(node);
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume_reserved("<="))
        {
            node = new_node_binary(ND_LE, node, add());
        }
        else if (consume_reserved(">="))
        {
            node = new_node_binary(ND_LE, add(), node);
        }
        else if (consume_reserved("<"))
        {
            node = new_node_binary(ND_LT, node, add());
        }
        else if (consume_reserved(">"))
        {
            node = new_node_binary(ND_LT, add(), node);
        }
        else
        {
            add_type(node);
            return node;
        }
    }
}

Node *new_node_add(Node *lhs, Node *rhs)
{
    if (!lhs)
        error_at(token->str, "lhs is null in parse.c new_node_add()");
    if (!rhs)
        error_at(token->str, "rhs is null in parse.c new_node_add()");

    add_type(lhs);
    add_type(rhs);

    // num + num
    if (is_integer(lhs->ty) && is_integer(rhs->ty))
        return new_node_binary(ND_ADD, lhs, rhs);

    // num + ptr
    if (is_integer(lhs->ty) && rhs->ty->ptr_to)
    {
        Node *tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    // ptr + num
    if (lhs->ty->ptr_to && is_integer(rhs->ty))
    {
        rhs = new_node_binary(ND_MUL, rhs, new_node_num(lhs->ty->ptr_to->size));
        return new_node_binary(ND_ADD, lhs, rhs);
    }

    error_at(token->str, "cannot caluculate ptr in parse.c new_add_node()");
}

Node *new_node_sub(Node *lhs, Node *rhs)
{
    if (!lhs)
        error_at(token->str, "lhs is null in parse.c new_sub_node()");
    if (!rhs)
        error_at(token->str, "rhs is null in parse.c new_sub_node()");

    add_type(lhs);
    add_type(rhs);

    // num - num
    if (is_integer(lhs->ty) && is_integer(rhs->ty))
    {
        return new_node_binary(ND_SUB, lhs, rhs);
    }

    // ptr - num
    if (lhs->ty->ptr_to && is_integer(rhs->ty))
    {
        rhs = new_node_binary(ND_MUL, rhs, new_node_num(lhs->ty->ptr_to->size));
        return new_node_binary(ND_SUB, lhs, rhs);
    }

    // ptr - ptr
    if (lhs->ty->ptr_to && rhs->ty->ptr_to)
    {
        lhs = new_node_binary(ND_SUB, lhs, rhs);
        return new_node_binary(ND_DIV, lhs, new_node_num(rhs->ty->ptr_to->size));
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume_reserved("+"))
        {
            node = new_node_add(node, mul());
        }
        else if (consume_reserved("-"))
        {
            node = new_node_sub(node, mul());
        }
        else
        {
            add_type(node);
            return node;
        }
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume_reserved("*"))
        {
            node = new_node_binary(ND_MUL, node, unary());
        }
        else if (consume_reserved("/"))
        {
            node = new_node_binary(ND_DIV, node, unary());
        }
        else
        {
            add_type(node);
            return node;
        }
    }
}

// unary = ("+" | "-" | "*" | "&" | "sizeof") unary
//       | postfix
Node *unary()
{
    if (consume_reserved("+"))
    {
        return unary();
    }
    if (consume_reserved("-"))
    {
        return new_node_binary(ND_SUB, new_node_num(0), unary());
    }
    if (consume_reserved("*"))
    {
        Node *node = new_node(ND_DEREF, unary());

        add_type(node);
        return node;
    }
    if (consume_reserved("&"))
    {
        Node *node = calloc(1, sizeof(Node));
        node->lhs = unary();

        if (node->lhs->ty->kind == TY_ARRAY)
            return new_node_var(node, token);

        node->kind = ND_ADDR;
        add_type(node);
        return node;
    }
    if (consume_keyword("sizeof"))
    {
        Node *node = unary();
        return new_node_num(node->ty->size);
    }
    return postfix();
}

// postfix = primary ( "[" expr "]" | "." ident )*
Node *postfix()
{
    Node *node = primary();

    for (;;)
    {
        if (consume_reserved("["))
        {
            node = new_node(ND_DEREF, new_node_add(node, expr()));
            expect_reserved("]");
            add_type(node);
        }
        else if (consume_reserved("."))
        {
            Token *tok = consume_ident();
            Member *member = find_member(tok, node->ty->members);
            if (!member)
                error_at(token->str, "member is not found. in parse.c postfix");
            node = new_node(ND_MEMBER, node);
            node->member = member;

            add_type(node);
        }
        else
        {
            add_type(node);
            return node;
        }
    }
}

// primary = "(" expr ")"
//         | ident ("(" ")")?
//         | ident "(" expr ("," expr)* ")"
//         | num
//         | string
Node *primary()
{
    if (consume_reserved("("))
    {
        Node *node = expr();
        expect_reserved(")");

        add_type(node);
        return node;
    }
    Token *tok = consume_ident();
    if (tok)
    {
        Node *node = calloc(1, sizeof(Node));
        if (consume_reserved("("))
        {
            Node head = {};
            Node *cur = &head;
            node->kind = ND_FUNCCALL;
            node->funcname = trim(tok->str, tok->len);
            find_func(node->funcname);

            while (!consume_reserved(")"))
            {
                cur = cur->next = expr();
                consume_reserved(",");
            }
            node->args = head.next;

            add_type(node);
            return node;
        }
        else
        {
            return new_node_var(node, tok);
        }
    }

    tok = consume_string();
    if (tok)
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_STRING;
        node->str = trim(tok->str, tok->len);

        Type *ty = pointer_to(ty_char);
        node->ty = ty;

        add_type(node);
        return node;
    }

    return new_node_num(expect_number());
}

Type *find_func(char *name)
{
    for (Function *func = funcs; func->name; func = func->next)
    {
        if (!strcmp(func->name, name))
            return func->ty;
    }
    error_at(token->str, "funcが見つかりません。in parse.c find_func");
}

Obj *find_lvar(Token *tok)
{
    for (Obj *var = locals; var->name; var = var->next)
    {
        if (!strcmp(var->name, trim(tok->str, tok->len)))
            return var;
    }
    return NULL;
}

Obj *find_gvar(Token *tok)
{
    for (Obj *var = globals; var->name; var = var->next)
    {
        if (var->is_function)
            continue;
        if (!strcmp(var->name, trim(tok->str, tok->len)))
            return var;
    }
    return NULL;
}

Member *find_member(Token *tok, Member *members)
{
    for (Member *member = members; member->name; member = member->next)
    {
        if (!strcmp(member->name, trim(tok->str, tok->len)))
            return member;
    }
    return NULL;
}

Member *create_new_member(Type *ty, Member *mems)
{
    Member *mem = calloc(1, sizeof(Member));
    mem->name = trim(ty->name->str, ty->name->len);
    mem->ty = ty;
    mem->size = ty->size;
    mem->offset = mems->offset + mems->size;
    mem->next = mems;
    return mem;
}

char *trim(char *s, int size_t)
{
    char *trimed_s = calloc(size_t + 1, sizeof(char));
    int i;
    for (i = 0; i < size_t; i++)
    {
        if (!s[i])
        {
            break;
        }
        trimed_s[i] = s[i];
    }
    trimed_s[i] = '\0';
    return trimed_s;
}
