#include "9cc.h"

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "引数の数がただしくありません\n");
        return 1;
    }

    // トークナイズしてパースする.
    user_input = argv[1];
    token = tokenize(argv[1]);
    Node *node = expr();

    // アセンプリの前半部分
    printf(".intel_syntax noprefix\n");
    printf(".global main \n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする.
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}