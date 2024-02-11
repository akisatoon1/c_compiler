#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "9cc.h"

// variable vector
Obj *locals;
Obj *globals;

Node *new_node(NodeKind kind, Node *lhs)
{
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
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;

    if (kind == ND_ADD || kind == ND_SUB)
    {
        if (lhs->ty->ptr_to && rhs->ty->ptr_to)
        {
            // ポインタ同士の演算, 引き算のみ
            if (kind != ND_SUB)
                error("ポインタ同士の演算は引き算のみです。");
            node->ty = ty_int;
            node->lhs = lhs;
            node->rhs = rhs;

            return new_node_binary(ND_DIV, node, new_node_num(lhs->ty->ptr_to->size));
        }
        else if (lhs->ty->ptr_to && !rhs->ty->ptr_to)
        {
            node->ty = lhs->ty;
            if (node->ty->kind == TY_ARRAY)
                node->ty->size = 8;
            rhs = new_node_binary(ND_MUL, rhs, new_node_num(lhs->ty->ptr_to->size));
        }
        else if (!lhs->ty->ptr_to && rhs->ty->ptr_to)
        {
            node->ty = rhs->ty;
            if (node->ty->kind == TY_ARRAY)
                node->ty->size = 8;
            lhs = new_node_binary(ND_MUL, lhs, new_node_num(rhs->ty->ptr_to->size));
        }
        else if (!lhs->ty->ptr_to && !rhs->ty->ptr_to)
        {
            node->ty = ty_int;
        }
        else
            error("型が存在しません");
    }
    else if (kind == ND_ASSIGN)
        node->ty = lhs->ty;
    else
        node->ty = ty_int;

    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->ty = ty_int;
    node->val = val;
    return node;
}

Node *new_node_var(Node *node, Token *tok)
{
    Obj *lvar = find_lvar(tok);
    if (lvar)
    {
        node->kind = ND_LVAR;
        node->var = lvar;
        node->ty = lvar->ty;
    }
    else
    {
        Obj *gvar = find_gvar(tok);

        if (gvar)
        {
            node->kind = ND_GVAR;
            node->var = gvar;
            node->ty = gvar->ty;
        }
        else
            error_at(tok->str, "'%s'は宣言されていない変数名です。", trim(tok->str, tok->len));
    }
    return node;
}

Obj *new_lvar(Token *tok, Type *ty)
{
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
    Obj *gvar = calloc(1, sizeof(Obj));

    gvar->name = trim(tok->str, tok->len);
    gvar->ty = ty;
    gvar->is_local = false;
    gvar->is_function = false;

    return gvar;
}

// declspec = "int" | "char"
Type *declspec()
{
    if (consume_type("int"))
        return ty_int;
    else if (consume_type("char"))
        return ty_char;
    else
        return NULL;
}

// program = (declspec "*"* ident function-def | declspec "*"* ident global-variable)*
Obj *program()
{
    globals = calloc(1, sizeof(Obj));
    while (!at_eof())
    {
        // トークンを読み込む
        Type *base_type = declspec();
        if (!base_type)
            error_at(token->str, "存在しない型です。parse.c:153");
        Type *ty = new_type(base_type);

        Token *tok = consume_ident();
        if (!strncmp(token->str, "(", 1))
        {
            locals = calloc(1, sizeof(Obj));
            Obj *func = function_def(ty, tok);
            func->locals = locals;
            func->next = globals;
            globals = func;
        }
        else
        {
            Obj *global_var = global_variable(ty, tok);
            global_var->next = globals;
            globals = global_var;
        }
    }

    // function or gvar vector
    return globals;
}

// globale-variable = ("[" num "]")? ";"
Obj *global_variable(Type *ty, Token *tok_ident)
{
    Obj *gvar;

    if (!tok_ident)
        error_at(token->str, "シンボルが見つかりません。");

    if (consume_reserved("["))
    {
        Type *type_array = calloc(1, sizeof(Type));
        type_array->kind = TY_ARRAY;
        type_array->array_len = expect_number();
        type_array->size = (ty->size) * (type_array->array_len);
        type_array->ptr_to = ty;

        ty = type_array;

        expect_reserved("]");
    }
    gvar = new_gvar(tok_ident, ty);
    expect_reserved(";");
    return gvar;
}

// function-def = "(" ")" "{" stmt* "}"
//          | "(" declspec  "*"* ident ("," declspec  "*"* ident)* ")" "{" stmt* "}"
Obj *function_def(Type *return_ty, Token *tok_ident)
{
    Obj *func = calloc(1, sizeof(Obj));
    func->is_local = false;
    func->is_function = true;

    func->ty = return_ty;

    if (!tok_ident)
    {
        error("関数名がありません。");
    }

    func->name = trim(tok_ident->str, tok_ident->len);

    expect_reserved("(");
    while (!consume_reserved(")"))
    {
        // トークンを読み込む
        Type *base_type = declspec();
        if (!base_type)
            error_at(token->str, "存在しない型です。parse.c:225");
        Type *type = new_type(base_type);

        Token *tok_param = consume_ident();
        if (!tok_param)
        {
            error("引数には変数を指定してください。（関数定義時）");
        }

        Obj *lvar = find_lvar(tok_param);
        if (lvar)
        {
            error("既に使われているローカル変数です。");
        }
        else
        {
            lvar = new_lvar(tok_param, type);
            locals = lvar;
        }

        consume_reserved(",");
    }

    // params vector
    func->params = locals;

    func->body = stmt();
    func->stack_size = align_to(locals->offset, 16);

    return func;
}

// stmt = expr ";"
//      | declspec "*"* ident ("[" num "]")? ";"
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
        node->kind = ND_TYPE_DEF;
        Type *type = new_type(base_type);

        Token *tok = consume_ident();
        if (tok)
        {
            if (consume_reserved("["))
            {
                Type *type_array = calloc(1, sizeof(Type));
                type_array->kind = TY_ARRAY;
                type_array->array_len = expect_number();
                type_array->size = (type->size) * (type_array->array_len);
                type_array->ptr_to = type;

                type = type_array;

                expect_reserved("]");
            }

            Obj *lvar = find_lvar(tok);
            if (lvar)
            {
                error_at(tok->str, "'%s'は既に使われている変数または配列名です。", trim(tok->str, tok->len));
            }
            else
            {
                lvar = new_lvar(tok, type);

                node->var = lvar;

                locals = lvar;
            }
        }
        else
        {
            error_at(token->str, "変数または配列名ではありません。");
        }
        expect_reserved(";");
    }
    else if (consume_return())
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
    else if (consume_controls("if"))
    {
        expect_reserved("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->cond = expr();
        expect_reserved(")");
        node->then = stmt();
        if (consume_controls("else"))
        {
            node->_else = stmt();
        }
        else
        {
            node->_else = NULL;
        }
    }
    else if (consume_controls("while"))
    {
        expect_reserved("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->cond = expr();
        expect_reserved(")");
        node->then = stmt();
    }
    else if (consume_controls("for"))
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
            return node;
        }
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
            node = new_node_binary(ND_ADD, node, mul());
        }
        else if (consume_reserved("-"))
        {
            node = new_node_binary(ND_SUB, node, mul());
        }
        else
        {
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
            return node;
        }
    }
}

// unary = ("+" | "-" | "*" | "&" | "sizeof") unary
//       | primary
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
        node->ty = node->lhs->ty->ptr_to;

        return node;
    }
    if (consume_reserved("&"))
    {
        Type *ty = calloc(1, sizeof(Type));
        Node *node = calloc(1, sizeof(Node));

        node->lhs = unary();
        if (node->lhs->ty->kind == TY_ARRAY)
            return new_node_var(node, token);

        node->kind = ND_ADDR;
        ty->kind = TY_PTR;
        ty->size = 8;
        ty->ptr_to = node->lhs->ty;
        node->ty = ty;

        return node;
    }
    if (consume_sizeof())
    {
        Node *node = calloc(1, sizeof(Node));
        node->lhs = unary();

        return new_node_num(node->lhs->ty->size);
    }
    return primary();
}

// primary = "(" expr ")"
//         | ident ("(" ")")?
//         | ident "(" expr ("," expr)* ")"
//         | ident "[" expr "]"
//         | num
Node *primary()
{
    if (consume_reserved("("))
    {
        Node *node = expr();
        expect_reserved(")");
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
            // will
            node->ty = ty_int;
            node->funcname = trim(tok->str, tok->len);
            while (!consume_reserved(")"))
            {
                cur = cur->next = expr();
                consume_reserved(",");
            }
            node->args = head.next;
        }
        else
        {
            node = new_node_var(node, tok);
            if (consume_reserved("["))
            {
                node = new_node_binary(ND_ADD, node, expr());
                expect_reserved("]");

                Node *node_top = new_node(ND_DEREF, node);
                node_top->ty = node->lhs->ty->ptr_to;

                return node_top;
            }
        }
        return node;
    }

    return new_node_num(expect_number());
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