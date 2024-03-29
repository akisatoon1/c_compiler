#include "9cc.h"

Type *ty_int = &(Type){TY_INT, NULL, 4};
Type *ty_char = &(Type){TY_CHAR, NULL, 1};
Type *ty_void = &(Type){TY_VOID};

bool is_integer(Type *ty)
{
    return ty->kind == TY_INT || ty->kind == TY_CHAR;
}

Type *copy_type(Type *ty)
{
    Type *copied = calloc(1, sizeof(Type));
    *copied = *ty;
    return copied;
}

Type *pointer_to(Type *ptr_to)
{
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_PTR;
    ty->ptr_to = ptr_to;
    ty->size = 8;
    return ty;
}

Type *array_of(Type *ptr_to, int len)
{
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_ARRAY;
    ty->name = ptr_to->name;
    ty->array_len = len;
    ty->size = (ptr_to->size) * (ty->array_len);
    ty->ptr_to = ptr_to;
    return ty;
}

void add_type(Node *node)
{
    if (!node || node->ty)
        return;

    add_type(node->lhs);
    add_type(node->rhs);

    switch (node->kind)
    {
    case ND_ADD:
    case ND_SUB:
        if (node->lhs->ty->kind == TY_ARRAY)
        {
            node->ty = pointer_to(node->lhs->ty->ptr_to);
            return;
        }
        node->ty = node->lhs->ty;
        return;

    case ND_MUL:
    case ND_DIV:
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_AND:
    case ND_OR:
    case ND_NUM:
        node->ty = ty_int;
        return;

    case ND_ASSIGN:
        if (node->lhs->ty->kind == TY_ARRAY)
            error_at(token->str, "配列型に代入できません。 in type.c add_type()");
        node->ty = node->lhs->ty;
        return;

    case ND_LVAR:
    case ND_GVAR:
        node->ty = node->var->ty;
        return;

    case ND_DEREF:
        node->ty = node->lhs->ty->ptr_to;
        return;

    case ND_ADDR:
        node->ty = pointer_to(node->lhs->ty);
        return;

    case ND_MEMBER:
        node->ty = node->member->ty;
        return;

    case ND_STRING:
        node->ty = pointer_to(ty_char);
        return;

    case ND_FUNCCALL:
        node->ty = find_func(node->funcname);
        return;

    case ND_STMT_EXPR:
        if (node->body)
        {
            Node *stmt = node->body;
            while (stmt->next)
                stmt = stmt->next;
            node->ty = stmt->ty;
            return;
        }
        error_at(token->str, "body is not found. in type.c add_type()");

    default:
        error_at(token->str, "%d is not expected NodeKind in type.c add_type()");
        return;
    }
}
