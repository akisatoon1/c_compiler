#include "9cc.h"

// 注意点
// call時に16倍数へのalignmentせず
// 符号付き整数(int)のみ実装中。ただしマイナスの表現は実装せず。
// charはsigned。
// 関数が定義済みかどうかのチェックは無い。(したがって戻り値の型は常にint)
// ポインタの演算や代入は、全て同じ型(ポインタの深さも含め)であると仮定。
// a[expr]は認めるが、num[expr]は認められない(本来は認められるが使わないので認められないことにした)。

// 実装済み
// int 4byte, ptr 8byte

// コンパイルするファイルのpath
static char *user_input;

// ファイルの中身の文字列
static char *compiled_string;

// main.c parse.c tokenize.c token_handler.c で使うグローバル変数
Token *token;

static char *read_file(char *path);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    compiled_string = read_file(user_input);
    token = tokenize(compiled_string);

    // function or global variable vector (main)
    Obj *prog = program();

    printf(".intel_syntax noprefix\n");

    while (prog->name)
    {
        if (prog->is_local)
            error("ローカル変数ではありません。");
        if (prog->is_function)
            gen_function(prog);
        else
            gen_gvar(prog);
        prog = prog->next;
    }

    return 0;
}

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    fprintf(stderr, "token->str: %s\n\n", token->str);
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - compiled_string;
    fprintf(stderr, "%s\n", compiled_string);

    // print space ' ' at fixed number times.
    for (int i = 0; i < pos; i++)
    {
        fprintf(stderr, "%s", " ");
    }

    // point to a place where an error occurs
    fprintf(stderr, "^\n");

    // print error message
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    // print the rest of token
    fprintf(stderr, "token->str rest: '%s'\n\n", token->str);
    exit(1);
}

// 指定されたファイルの内容を返す
char *read_file(char *path)
{
    // ファイルを開く
    FILE *fp = fopen(path, "r");
    if (!fp)
        error("cannot open %s: %s", path, strerror(errno));

    // ファイルの長さを調べる
    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    // ファイル内容を読み込む
    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n')
        buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}
