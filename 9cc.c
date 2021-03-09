#include<ctype.h>
#include<stdarg.h> //可変数の引数にアクセルできるように
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

        if(strchr("+-*/()", *p)){
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

//
// Parser
//

// 抽象構文木のノードの種類
typedef enum{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node{
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺 (left-hand side)
    Node *rhs;      // 右辺
    int val;        // kindがND_NUMのときのみ使う
};



Node *new_node(NodeKind kind){
    Node *node = (Node *) calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_num(int val){
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

Node *expr();
Node *mul();
Node *unary();
Node *primary();

// expr = mul ("+" mul | "-" mul)*
Node *expr(){
    Node *node = mul();

    for(;;){
        if(consume('+'))
            node = new_binary(ND_ADD, node, mul());
        else if(consume('-'))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(){
    Node *node = unary();

    for(;;){
        if(consume('*'))
            node = new_binary(ND_MUL, node, unary());
        else if(consume('/'))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ( "+" | "-" )? unary | primary
Node *unary(){

    if (consume('+'))
        return unary();

    if (consume('-'))
        return new_binary(ND_SUB, new_num(0), unary());

    return primary();

}

// primary = "(" expr ")" | num
Node *primary(){

    // 次のトークンが"("なら、"(" expr ")"のはず
    if(consume('(')){
        Node *node = expr();
        expect(')');
        return node;
    }

    // そうでなければ数値
    return new_num(expect_number());
}

//
// Code generator
//


// nodeのkindに応じてアセンブリの操作を書く
void gen(Node *node){
    if(node->kind == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch(node->kind){
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
    }

    printf("    push rax\n");

}




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