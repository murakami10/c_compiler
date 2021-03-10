#include "9cc.h"

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

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
bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

// 次のトークンが期待している記号の時には,トークンを１つ読み進める。
// それ以外のときはエラーを返す.
bool expect(char *op){
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
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

// トークンが終端であるか
bool at_eof(){
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
    Token *tok = (Token *)calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

// pとqがstrlen(q)番目一致しているか
bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q))  == 0;
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

        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
                cur = new_token(TK_RESERVED, cur, p, 2);
                p += 2;
                continue;
            }

        if(strchr("+-*/()<>", *p)){
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = q - p;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}


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


// expr = equality
Node *expr(){
    return equality();
}

// equality = relational ( "==" relational | "!=" relational)*
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_binary(ND_EQ, node, relational());
        
        else if (consume("!="))
            node = new_binary(ND_NE, node, relational());

        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(){
    Node *node = add();

    for (;;) {
        if(consume("<"))
            node = new_binary(ND_LT, node, add());
        else if (consume("<="))
            node = new_binary(ND_LE, node, add());
        else if (consume(">"))
            node = new_binary(ND_LT, add(), node);
        else if (consume(">="))
            node = new_binary(ND_LE, add(), node);
        else
            return node;
    }
}


// add = mul ("+" mul | "-" mul)*
Node *add() {
    Node *node = mul();

    for(;;) {
        if(consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }

}

// mul = unary ("*" unary | "/" unary)*
Node *mul(){
    Node *node = unary();

    for(;;){
        if(consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if(consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}


// unary = ( "+" | "-" )? unary | primary
Node *unary(){

    if (consume("+"))
        return unary();

    if (consume("-"))
        return new_binary(ND_SUB, new_num(0), unary());

    return primary();

}

// primary = "(" expr ")" | num
Node *primary(){

    // 次のトークンが"("なら、"(" expr ")"のはず
    if(consume("(")){
        Node *node = expr();
        expect(")");
        return node;
    }

    // そうでなければ数値
    return new_num(expect_number());
}

