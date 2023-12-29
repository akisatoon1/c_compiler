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
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        if (node->_else)
        {
            printf("    je .Lelse%d\n", Lelse);
            gen(node->then);
            printf("    pop rax\n");
            printf("    jmp .Lend%d\n", Lend);
            printf(".Lelse%d:\n", Lelse);
            gen(node->_else);
            printf("    pop rax\n");
            Lelse++;
        }
        else
        {
            printf("    je .Lend%d\n", Lend);
            gen(node->then);
            printf("    pop rax\n");
        }
        printf(".Lend%d:\n", Lend);
        Lend++;
        return;
    case ND_WHILE:
        printf(".Lbegin%d:\n", Lbegin);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", Lend);
        gen(node->then);
        printf("    pop rax\n");
        printf("    jmp .Lbegin%d\n", Lbegin);
        printf(".Lend%d:\n", Lend);
        Lend++;
        Lbegin++;
        return;
    case ND_FOR:
        if (node->init)
        {
            gen(node->init);
            printf("    pop rax\n");
        }
        printf(".Lbegin%d:\n", Lbegin);
        if (node->cond)
        {
            gen(node->cond);
        }
        else
        {
            printf("    push 1\n");
        }
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", Lend);
        gen(node->then);
        printf("    pop rax\n");
        if (node->inc)
        {
            gen(node->inc);
            printf("    pop rax\n");
        }
        printf("    jmp .Lbegin%d\n", Lbegin);
        printf(".Lend%d:\n", Lend);
        Lbegin++;
        Lend++;
        return;
    case ND_BLOCK:
        for (Node *n = node->body; n; n = n->next)
        {
            gen(n);
            printf("    pop rax\n");
        }
        return;
    case ND_FUNCCALL:
        int nargs = 0;
        for (Node *arg = node->args; arg; arg = arg->next)
        {
            gen(arg);
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
    case ND_FUNC:
        printf(".globl %s\n", node->funcname);
        printf("%s:\n", node->funcname);
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, 208\n");
        gen(node->body);
        return;
    default:
        break;
    }

    gen(node->lhs);
    gen(node->rhs);

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