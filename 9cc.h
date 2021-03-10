#include<ctype.h>
#include<stdarg.h> //可変数の引数にアクセスできるように
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//
// Tokenizer
//

typedef enum {
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF       // 入力の終わりを表すトークン
} TokenKind;

// tokenのタイプ
typedef struct Token Token;
struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの際, その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};


//
// Parser
//

// 抽象構文木のノードの種類
typedef enum{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // =
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_NUM, // 整数
} NodeKind;


// 抽象構文木のノードの型
typedef struct Node Node;
struct Node{
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺 (left-hand side)
    Node *rhs;      // 右辺
    int val;        // kindがND_NUMのときのみ使う
};


Token *tokenize(char *p);
Node *expr();
void gen(Node *node);

// 現在着目しているトークン
extern Token *token;

// 入力プログラム
extern char *user_input;

