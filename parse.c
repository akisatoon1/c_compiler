#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// program = function*
Function *program()
{
    Function head = {};
    Function *cur = &head;
    while (!at_eof())
    {
        Function *func = calloc(1, sizeof(Function));
        locals = calloc(1, sizeof(LVar));
        func = function(func);
        func->locals = locals;
        cur = cur->next = func;
    }
    cur->next = NULL;

    // function vector
    return head.next;
}

// function = "int" ident "(" ")" "{" stmt* "}"
//          | "int" ident "(" ident ("," ident)* ")" "{" stmt* "}"
Function *function(Function *func)
{
    expect_type("int");
    Token *tok = consume_ident();
    if (!tok)
    {
        error("関数名がありません。");
    }
    func->name = trim(tok->str, tok->len);
    expect_reserved("(");
    LVar head = {};
    LVar *cur = &head;
    while (!consume_reserved(")"))
    {
        LVar *param = calloc(1, sizeof(LVar));
        expect_type("int");
        Token *tok_param = consume_ident();
        if (!tok_param)
        {
            error("引数には変数を指定してください。（関数定義時）");
        }
        LVar *lvar = find_lvar(tok_param);
        if (lvar)
        {
            error("既に使われているローカル変数です。");
        }
        else
        {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok_param->str;
            lvar->len = tok_param->len;
            lvar->offset = locals->offset + 8;

            param->name = lvar->name;
            param->len = lvar->len;
            param->offset = lvar->offset;

            locals = lvar;
        }
        cur = cur->next = param;
        consume_reserved(",");
    }

    // params vector
    func->params = head.next;
    func->body = stmt();
    func->stack_size = align_to(locals->offset, 16);

    return func;
}

// stmt = expr ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "int" "*"* ident ";"
//      | "{" stmt* "}"
Node *stmt()
{
    Node *node;
    if (consume_return())
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
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
    else if (consume_type("int"))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_TYPE_DEF;
        Type *type = calloc(1, sizeof(Type));
        type->ty = TY_INT;
        while (consume_reserved("*"))
        {
            Type *type_ptr = calloc(1, sizeof(Type));
            type_ptr->ty = TY_PTR;
            type_ptr->ptr_to = type;
            type = type_ptr;
        }
        Token *tok = consume_ident();
        if (tok)
        {
            LVar *lvar = find_lvar(tok);
            if (lvar)
            {
                error_at(tok->str, "'%s'は既に使われている変数名です。", trim(tok->str, tok->len));
            }
            else
            {
                lvar = calloc(1, sizeof(LVar));
                lvar->next = locals;
                lvar->name = tok->str;
                lvar->ty = type;
                lvar->len = tok->len;
                lvar->offset = locals->offset + 8;
                node->var = lvar;
                node->ty = lvar->ty;
                locals = lvar;
            }
        }
        else
        {
            error_at(token->str, "変数名ではありません。");
        }
        expect_reserved(";");
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
        node = new_node(ND_ASSIGN, node, assign());
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
            node = new_node(ND_EQ, node, relational());
        }
        else if (consume_reserved("!="))
        {
            node = new_node(ND_NE, node, relational());
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
            node = new_node(ND_LE, node, add());
        }
        else if (consume_reserved(">="))
        {
            node = new_node(ND_LE, add(), node);
        }
        else if (consume_reserved("<"))
        {
            node = new_node(ND_LT, node, add());
        }
        else if (consume_reserved(">"))
        {
            node = new_node(ND_LT, add(), node);
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
            node = new_node(ND_ADD, node, mul());
        }
        else if (consume_reserved("-"))
        {
            node = new_node(ND_SUB, node, mul());
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
            node = new_node(ND_MUL, node, unary());
        }
        else if (consume_reserved("/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        else
        {
            return node;
        }
    }
}

// unary = primary
//       | ("+" | "-")? unary
//       | ("*" | "&") unary
Node *unary()
{
    if (consume_reserved("+"))
    {
        return unary();
    }
    if (consume_reserved("-"))
    {
        return new_node(ND_SUB, new_node_num(0), unary());
    }
    if (consume_reserved("*"))
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->lhs = unary();

        if (node->lhs->kind == ND_LVAR)
        {
            node->ty = node->lhs->ty->ptr_to;
        }

        return node;
    }
    if (consume_reserved("&"))
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_ADDR;
        node->lhs = unary();
        return node;
    }
    return primary();
}

// primary = "(" expr ")"
//         | ident ("(" ")")?
//         | ident "(" expr ("," expr)* ")"
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
            node->kind = ND_LVAR;
            LVar *lvar = find_lvar(tok);
            if (lvar)
            {
                node->var = lvar;
                node->ty = lvar->ty;
            }
            else
            {
                error_at(tok->str, "'%s'は宣言されていない変数名です。", trim(tok->str, tok->len));
            }
        }
        return node;
    }
    return new_node_num(expect_number());
}

LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
    {
        if (var->len == tok->len && !memcmp(var->name, tok->str, var->len))
        {
            return var;
        }
    }
    return NULL;
}

char *trim(char *s, int size_t)
{
    char *ss = calloc(size_t + 1, sizeof(char));
    int i;
    for (i = 0; i < size_t; i++)
    {
        ss[i] = s[i];
    }
    ss[i] = '\0';
    return ss;
}