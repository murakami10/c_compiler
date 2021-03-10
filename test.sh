#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit
    fi
}

# 入力された数字を出力
assert 0 0
assert 42 42

# 文字列での加减の追加
assert 21 "5+20-4"
assert 41 " 12 + 34 - 5"

# 乗除と()を追加
assert 47 "5+6*7"
assert 15 "5*(9-6)"
assert 4  "(3+5)/2"

# 単項演算子
assert 10 "-10+20"
assert 10 "- -10"
assert 10 "- - + 10"

# 比較演算子
assert 0 "0==1"
assert 1 "42==42"
assert 1 "0!=1"
assert 0 "42!=42"

assert 1 "0<1"
assert 0 "1<1"
assert 0 "2<1"
assert 1 "0<=1"
assert 1 "1<=1"
assert 0 "2<=1"

assert 1 "1>0"
assert 0 "1>1"
assert 0 "1>2"
assert 1 "1>=0"
assert 1 "1>=1"
assert 0 "1>=2"


echo OK