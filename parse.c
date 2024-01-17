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
void program()
{
    int i = 0;
    while (!at_eof())
    {
        locals = calloc(1, sizeof(LVar));
        code[i++] = function();
    }
    code[i] = NULL;
}

// function = ident "(" ")" "{" stmt* "}"
//          | ident "(" ident ("," ident)* ")" "{" stmt* "}"
Node *function()
{
    Node *node = calloc(1, sizeof(Node));
    Token *tok = consume_ident();
    if (!tok)
    {
        error("関数が読み込めません。");
    }
    expect_reserved("(");
    Node head = {};
    Node *cur = &head;
    node->kind = ND_FUNC;
    node->funcname = trim(tok->str, tok->len);
    while (!consume_reserved(")"))
    {
        Node *node_var = calloc(1, sizeof(Node));
        node_var->kind = ND_LVAR;
        Token *tok_var = consume_ident();
        if (!tok_var)
        {
            error("引数には変数を指定してください。（関数定義時）");
        }
        LVar *lvar = find_lvar(tok_var);
        if (lvar)
        {
            error("既に使われているローカル変数です。");
        }
        else
        {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok_var->str;
            lvar->len = tok_var->len;
            lvar->offset = locals->offset + 8;
            node_var->offset = lvar->offset;
            locals = lvar;
        }
        cur = cur->next = node_var;
        consume_reserved(",");
    }
    node->args = head.next;
    node->body = stmt();

    return node;
}

// stmt = expr ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
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

// unary = ("+" | "-")? primary
//       | ("*" | "&") unary
Node *unary()
{
    if (consume_reserved("+"))
    {
        return primary();
    }
    if (consume_reserved("-"))
    {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    if (consume_reserved("*"))
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->lhs = unary();
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

// primary = num
//         | ident ("(" ")")?
//         | ident "(" expr ("," expr)* ")"
//         | "(" expr ")"
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
                node->offset = lvar->offset;
            }
            else
            {
                lvar = calloc(1, sizeof(LVar));
                lvar->next = locals;
                lvar->name = tok->str;
                lvar->len = tok->len;
                lvar->offset = locals->offset + 8;
                node->offset = lvar->offset;
                locals = lvar;
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