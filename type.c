#include "9cc.h"

Type *ty_int = &(Type){TY_INT, NULL, 4};
Type *ty_char = &(Type){TY_CHAR, NULL, 1};

static Type *copy_type(Type *ty);

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
        if (node->lhs->ty->ptr_to && node->rhs->ty->ptr_to)
        {
            node->ty = ty_int;
            return;
        }
        else if (node->lhs->ty->ptr_to && !node->rhs->ty->ptr_to)
        {
            if (node->lhs->ty->kind == TY_ARRAY)
            {
                node->lhs->ty = copy_type(node->lhs->ty);
                node->lhs->ty->size = 8;
                node->lhs->ty->array_len = 0;
            }

            node->ty = node->lhs->ty;
            return;
        }
        else if (!node->lhs->ty->ptr_to && node->rhs->ty->ptr_to)
        {
            if (node->rhs->ty->kind == TY_ARRAY)
            {
                node->rhs->ty = copy_type(node->rhs->ty);
                node->rhs->ty->size = 8;
                node->rhs->ty->array_len = 0;
            }

            node->ty = node->rhs->ty;
            return;
        }
        else if (!node->lhs->ty->ptr_to && !node->rhs->ty->ptr_to)
        {
            node->ty = node->lhs->ty;
            return;
        }
        else
            error("型が存在しません in type.c add_type()");

    case ND_MUL:
    case ND_DIV:
        node->ty = node->lhs->ty;
        return;

    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_NUM:
    case ND_FUNCCALL: // All func's return type is int.
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

    default:
        error_at(token->str, "%d is not expected NodeKind in type.c add_type()");
        return;
    }
}
