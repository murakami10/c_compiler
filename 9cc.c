#include<ctype.h>
#include<stdarg.h> //可変数の引数にアクセルできるように
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef enum {
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF       // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;    // kindがTK_NUMの際, その数値
    char *str;      // トークン文字列
};

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error_at(char *loc, char *fmt, ...){
    va_list ap;

    va_start(ap, fmt); // apが１つ目の引数を指す
    int pos = loc - user_input; // エラーの場所
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // posの個数空白
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

// 次のトークンが期待している記号の時には,トークンを１つ読み進めて真を返す。
// それ以外のときは偽を返す.
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

// 次のトークンが期待している記号の時には,トークンを１つ読み進める。
// それ以外のときはエラーを返す.
bool expect(char op){
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "'%c'ではありません.", op);
    token = token->next;
}

// 次のトークンが数値の場合,トークンを１つ読み進めてその値を返す.
// そうでない場合エラーを報告する
int expect_number(){
    if(token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val; 
}

bool at_eof(){
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = (Token *)calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 入力文字列をpをトークナイズしてそれを返す.
Token *tokenize(char *p){
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p){
        if(isspace(*p)){
            p++;
            continue;
        }

        if(*p == '+'|| *p == '-'){
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}


int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "引数の数がただしくありません\n");
        return 1;
    }

    // トークナイズする.
    user_input = argv[1];
    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main \n");
    printf("main:\n");
    //数の最初は数でなければならないのでチェックして最初のmov命令を出力
    printf("    mov rax, %d\n", expect_number());
    while(!at_eof()){
        if(consume('+')){
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
}

    printf("    ret\n");
    return 0;
}