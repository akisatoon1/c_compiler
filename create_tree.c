#include "9cc.h"

void print_space(FILE *fp, int n);
char *node_kind_name(Node *node);
void display_tree(FILE *fp, Node *node, int space);
void display_child(FILE *fp, Node *node, char *name, int space);

char *nodekind[] = {"ND_ADD",
                    "ND_SUB",
                    "ND_MUL",
                    "ND_DIV",
                    "ND_EQ",
                    "ND_NE",
                    "ND_LT",
                    "ND_LE",
                    "ND_ASSIGN",
                    "ND_LVAR",
                    "ND_GVAR",
                    "ND_RETURN",
                    "ND_IF",
                    "ND_WHILE",
                    "ND_FOR",
                    "ND_BLOCK",
                    "ND_FUNCCALL",
                    "ND_NUM",
                    "ND_DEREF",
                    "ND_ADDR",
                    "ND_TYPE_DEF",
                    "ND_STRING",
                    "ND_MEMBER"};

void print_space(FILE *fp, int n)
{
    for (int i = 0; i < n; i++)
    {
        fprintf(fp, " ");
    }
    return;
}

char *node_kind_name(Node *node)
{
    return nodekind[node->kind];
}

void display_tree(FILE *fp, Node *node, int space)
{
    if (!node)
    {
        fprintf(fp, "NULL\n");
        return;
    }

    char *name = node_kind_name(node);
    int len = strlen(name);

    fprintf(fp, "%s", name);
    display_child(fp, node->lhs, "lhs", space + len);
    print_space(fp, len + space);
    display_child(fp, node->rhs, "rhs", space + len);
    print_space(fp, len + space);
    display_child(fp, node->cond, "cond", space + len);
    print_space(fp, len + space);
    display_child(fp, node->then, "then", space + len);
    print_space(fp, len + space);
    display_child(fp, node->_else, "_else", space + len);
    print_space(fp, len + space);
    display_child(fp, node->init, "init", space + len);
    print_space(fp, len + space);
    display_child(fp, node->inc, "inc", space + len);
    Node *n;
    for (n = node->body; n; n = n->next)
    {
        print_space(fp, len + space);
        display_child(fp, n, "body", space + len);
    }
    print_space(fp, len + space);
    display_child(fp, n, "body", space + len);
    return;
}

void display_child(FILE *fp, Node *node, char *name, int space)
{
    int len = strlen(name);
    fprintf(fp, "--%s-->", name);
    display_tree(fp, node, space + len + 5);
    return;
}
