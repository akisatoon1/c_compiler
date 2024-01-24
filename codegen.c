#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "9cc.h"

// ラベルid
int Lbegin = 0;
int Lend = 0;
int Lelse = 0;

char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error("代入の左辺値が変数ではありません。");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->var->offset);
    printf("    push rax # address of variable\n");
}

void gen_function(Function *func)
{
    printf(".globl %s\n", func->name);
    printf("%s:\n", func->name);
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d # allocate function stack for local variables\n", func->stack_size);
    int nargs = 0;
    for (LVar *param = func->params; param; param = param->next)
    {
        if (!param->offset)
            break;
        printf("    mov rax, rbp\n");
        printf("    sub rax, %d\n", param->offset);
        printf("    push rax\n");
        nargs++;
    }
    for (int i = 0; i < nargs; i++)
    {
        printf("    pop rax\n");
        printf("    mov [rax], %s\n", argreg[i]);
    }
    gen_stmt(func->body);

    return;
}

void gen_stmt(Node *node)
{
    switch (node->kind)
    {
    case ND_RETURN:
        gen_expr(node->lhs);
        printf("    pop rax # return value\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        gen_expr(node->cond);
        printf("    pop rax # if condition\n");
        printf("    cmp rax, 0\n");
        if (node->_else)
        {
            printf("    je .Lelse%d\n", Lelse);
            gen_stmt(node->then);
            printf("    jmp .Lend%d\n", Lend);
            printf(".Lelse%d:\n", Lelse);
            gen_stmt(node->_else);
            Lelse++;
        }
        else
        {
            printf("    je .Lend%d\n", Lend);
            gen_stmt(node->then);
        }
        printf(".Lend%d:\n", Lend);
        Lend++;
        return;
    case ND_WHILE:
        printf(".Lbegin%d:\n", Lbegin);
        gen_expr(node->cond);
        printf("    pop rax # while condition\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", Lend);
        gen_stmt(node->then);
        printf("    jmp .Lbegin%d\n", Lbegin);
        printf(".Lend%d:\n", Lend);
        Lend++;
        Lbegin++;
        return;
    case ND_FOR:
        if (node->init)
        {
            gen_expr(node->init);
            printf("    pop rax\n");
        }
        printf(".Lbegin%d:\n", Lbegin);
        if (node->cond)
        {
            gen_expr(node->cond);
        }
        else
        {
            printf("    push 1\n");
        }
        printf("    pop rax # for condition\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", Lend);
        gen_stmt(node->then);
        if (node->inc)
        {
            gen_expr(node->inc);
            printf("    pop rax\n");
        }
        printf("    jmp .Lbegin%d\n", Lbegin);
        printf(".Lend%d:\n", Lend);
        Lbegin++;
        Lend++;
        return;
    case ND_TYPE_DEF:
        printf("    # type definition\n");
        return;
    case ND_BLOCK:
        for (Node *n = node->body; n; n = n->next)
        {
            gen_stmt(n);
        }
        return;
    default:
        gen_expr(node);
        printf("    pop rax # pop extra data from stack\n");
        return;
    }
}

void gen_expr(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax # address of variable\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax # value of variable\n");
        return;
    case ND_ASSIGN:
        if (node->lhs->kind == ND_LVAR)
        {
            gen_lval(node->lhs);
        }
        else if (node->lhs->kind == ND_DEREF)
        {
            gen_expr(node->lhs->lhs);
        }
        else
        {
            error("代入可能な左辺値ではありません。");
        }
        gen_expr(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi # assign\n");
        printf("    push rdi\n");
        return;
    case ND_FUNCCALL:
        int nargs = 0;
        for (Node *arg = node->args; arg; arg = arg->next)
        {
            gen_expr(arg);
            nargs++;
        }
        for (int i = nargs - 1; i >= 0; i--)
        {
            printf("    pop %s\n", argreg[i]);
        }
        // mov rax, 0
        // rspを16byte整列にalignしてない
        printf("    call %s\n", node->funcname);
        printf("    push rax\n");
        return;
    case ND_DEREF:
        gen_expr(node->lhs);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
        return;
    default:
        break;
    }

    gen_expr(node->lhs);
    gen_expr(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    default:
        break;
    }

    printf("    push rax\n");
    return;
}