#include "9cc.h"

Type *ty_int = &(Type){TY_INT, NULL, 4, 0};
Type *ty_char = &(Type){TY_CHAR, NULL, 1, 0};

// トークンを知らんとこで読み込んでるのでわかりづらい。
// when type defenition, return type.
Type *new_type(Type *type)
{
    while (consume_reserved("*"))
    {
        Type *type_ptr = calloc(1, sizeof(Type));
        type_ptr->kind = TY_PTR;
        type_ptr->size = 8;
        type_ptr->ptr_to = type;
        type = type_ptr;
    }
    return type;
}