#include "9cc.h"

// ラベルid
int Lbegin = 0;
int Lend = 0;
int Lelse = 0;

// registers
char *argreg_64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *argreg_32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char *argreg_8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

// generate
static void gen_lval_address(Node *node);
static void gen_gvar_address(Node *node);
static void gen_stmt(Node *node);
static void gen_expr(Node *node);

void gen_lval_address(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error("代入の左辺値が変数ではありません。(lvar)");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->var->offset);
    printf("    push rax # address of local variable\n");
    return;
}

void gen_gvar_address(Node *node)
{
    if (node->kind != ND_GVAR)
    {
        error("代入の左辺値が変数ではありません。(gvar)");
    }
    printf("    lea rax, %s[rip]\n", node->var->name);
    printf("    push rax # address of global variable\n");
    return;
}

void gen_gvar(Obj *gvar)
{
    printf(".data\n");
    printf(".globl %s\n", gvar->name);
    printf("%s:\n", gvar->name);
    printf("    .zero %d\n", gvar->ty->size);
    return;
}

void gen_function(Obj *func)
{
    printf(".text\n");
    printf(".globl %s\n", func->name);
    printf("%s:\n", func->name);
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d # allocate function stack for local variables\n", func->stack_size);

    int nargs = 0;
    int sizes[6];
    for (Obj *param = func->params; param->offset; param = param->next)
    {
        sizes[nargs] = param->ty->size;

        printf("    mov rax, rbp\n");
        printf("    sub rax, %d\n", param->offset);
        printf("    push rax\n");
        nargs++;
    }
    for (int i = 0; i < nargs; i++)
    {
        printf("    pop rax\n");
        if (sizes[nargs - i - 1] == 1)
            printf("    mov BYTE PTR [rax], %s\n", argreg_8[i]);
        else if (sizes[nargs - i - 1] == 4)
            printf("    mov DWORD PTR [rax], %s\n", argreg_32[i]);
        else if (sizes[nargs - i - 1] == 8)
            printf("    mov  QWORD PTR [rax], %s\n", argreg_64[i]);
        else
            error("(args)存在しないサイズです。size: %d", sizes[nargs - i]);
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
        gen_lval_address(node);
        printf("    pop rax # address of variable\n");

        if (node->ty->kind == TY_ARRAY)
        {
            printf("    push rax\n");
            return;
        }

        if (node->ty->size == 1)
        {
            printf("    mov al, BYTE PTR [rax]\n");
            printf("    movsx rax, al\n");
        }
        else if (node->ty->size == 4)
        {
            printf("    mov eax, DWORD PTR [rax]\n");
            printf("    movsxd rax, eax\n");
        }
        else if (node->ty->size == 8)
        {
            printf("    mov rax, QWORD PTR [rax]\n");
        }
        else
            error("(ND_LVAR)存在しないサイズです。size: %d", node->ty->size);
        printf("    push rax # value of variable\n");

        return;
    case ND_GVAR:
        gen_gvar_address(node);
        if (node->ty->kind == TY_ARRAY)
        {
            // printf("    pop rax\n");
            // printf("    push rax\n");
            return;
        }

        if (node->ty->size == 1)
        {
            printf("    mov al, BYTE PTR [rax]\n");
            printf("    movsx rax, al\n");
        }
        else if (node->ty->size == 4)
        {
            printf("    pop rax\n");
            printf("    mov eax, DWORD PTR [rax]\n");
            printf("    movsxd rax, eax\n");
        }
        else if (node->ty->size == 8)
        {
            printf("    mov rax, QWORD PTR [rax]\n");
        }
        else
            error("(ND_LVAR)存在しないサイズです。size: %d", node->ty->size);
        printf("    push rax # value of variable\n");
        return;
    case ND_ASSIGN:
        if (node->lhs->kind == ND_LVAR)
            gen_lval_address(node->lhs);
        else if (node->lhs->kind == ND_GVAR)
            gen_gvar_address(node->lhs);
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

        if (node->ty->size == 1)
        {
            printf("    mov BYTE PTR [rax], dil # assign\n");
        }
        else if (node->lhs->ty->size == 4)
        {
            printf("    mov DWORD PTR [rax], edi # assign\n");
        }
        else if (node->lhs->ty->size == 8)
        {
            printf("    mov QWORd PTR [rax], rdi # assign\n");
        }
        else
            error("(ND_ASSIGN)存在しないサイズです。size: %d", node->lhs->ty->size);

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
            printf("    pop %s\n", argreg_64[i]);
        }
        // mov rax, 0
        // rspを16byte整列にalignしてない
        printf("    call %s\n", node->funcname);
        printf("    push rax\n");
        return;
    case ND_DEREF:
        gen_expr(node->lhs);
        printf("    pop rax\n");

        if (node->ty->size == 1)
        {
            printf("    mov al, BYTE PTR [rax]\n");
            printf("    movsx rax, al\n");
        }
        else if (node->lhs->ty->size == 4)
        {
            printf("    mov eax, DWORD PTR [rax]\n");
            printf("    movsxd rax, eax\n");
        }
        else if (node->lhs->ty->size == 8)
        {
            printf("    mov rax, QWORD PTR [rax]\n");
        }
        else
            error("(ND_DEREF)存在しないサイズです。size: %d", node->lhs->ty->size);

        printf("    push rax\n");

        return;
    case ND_ADDR:
        if (node->lhs->kind == ND_LVAR)
            gen_lval_address(node->lhs);
        else if (node->lhs->kind == ND_GVAR)
            gen_gvar_address(node->lhs);
        else
            error("単項&にふさわしくないノードです。codegen.c: gen_expr()");
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