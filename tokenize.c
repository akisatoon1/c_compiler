#include "9cc.h"

// keyword
char *keywords[] = {"return", "if", "else", "while", "for", "int", "char", "sizeof", "struct"};

static bool is_keyword(char *p, char *s);
static bool is_alnum(char c);
static char *return_keyword(char *p);

bool is_keyword(char *p, char *s)
{
    return !strncmp(p, s, strlen(s)) && !is_alnum(p[strlen(s)]);
}

bool is_alnum(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}

char *return_keyword(char *p)
{
    int num = sizeof(keywords) / 8;
    for (int i = 0; i < num; i++)
    {
        if (is_keyword(p, keywords[i]))
            return keywords[i];
    }
    return NULL;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p)
    {
        // 空白文字と改行文字をスキップ
        if (isspace(*p) || *p == '\n')
        {
            p++;
            continue;
        }

        // #includeの文を書かないとvscode上でエラー表示されるので、
        // #include <stdio.h>のような文は読み込まない。
        if (*p == '#')
        {
            p++;
            while (*p != '\n')
                p++;
            continue;
        }

        // 行コメントをスキップ
        if (!strncmp(p, "//", 2))
        {
            p = p + 2;
            while (*p != '\n')
                p++;
            continue;
        }

        // ブロックコメントをスキップ
        if (!strncmp(p, "/*", 2))
        {
            p = p + 2;
            char *q = strstr(p + 2, "*/");
            if (!q)
                error_at(p, "コメントが閉じられていません。");
            p = q + 2;
            continue;
        }

        // 長い記号を先にトークナイズする。
        if (strncmp(p, ">=", 2) == 0 || strncmp(p, "<=", 2) == 0 || strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0)
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*&/()<>;={}[],.", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if (strchr("\"", *p))
        {
            char *q = ++p;
            while (!strchr("\"", *p))
                p++;
            cur = new_token(TK_STRING, cur, q, p++ - q);
            continue;
        }

        char *keyword = return_keyword(p);
        if (keyword)
        {
            int len = strlen(keyword);
            cur = new_token(TK_KEYWORD, cur, p, len);
            p += len;
            continue;
        }

        if (is_ident1(*p))
        {
            char *q = p;
            cur = new_token(TK_IDENT, cur, p++, 0);
            while (is_ident2(*p))
            {
                p++;
            }
            cur->len = p - q;
            continue;
        }
        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        error("トークナイズできません。*p: %c", *p);
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
