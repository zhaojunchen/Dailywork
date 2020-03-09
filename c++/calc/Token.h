//
// Created by zhao on 2019/10/9.
//

#ifndef TEST_TOKEN_H
#define TEST_TOKEN_H

#include <string>

using namespace std;
enum token_type {
    token_null = 0, token_eof = 1, token_int, token_float, token_identifier, token_operator, token_keyword
};

enum operator_type {
    PLUS = '+', SUB = '-', MUL = '*', DIV = '/', LHS = '(', RHS = ')', EQ = '=', UNKNOWN_OP = 0
};

class Token {
public:
    Token() : lines(0), tag(token_null) {}

    Token(int _tag, int _lines = 0) : lines(_lines), tag(_tag) {}

    virtual string getValue() { return ""; }

    int getTag() const { return tag; }

    int getLines() const { return lines; }

    void setLines(int _lines) { lines = _lines; }

    virtual void setValue(string _value) {}


    virtual ~Token() {};
protected:
    int tag;
    int lines = 0;//所在行位置
};

class Token_Int : public Token {
public:
    string value;

    Token_Int() : Token(token_int), value(0) {};

    Token_Int(string _value, int _lines = 0) : Token(token_int, _lines), value(_value) {}

    string getValue() override { return value; }

    void setValue(string _value) override { value = _value; }

    ~Token_Int() override {}
};

class Token_Float : public Token {
public:
    //将类型转化为string便于处理 float value;
    string value;

    Token_Float() : Token(token_float), value(0) {};

    Token_Float(string _value, int _lines = 0) : Token(token_float, _lines), value(_value) {}

    string getValue() override { return value; }

    void setValue(string _value) override { value = _value; }

    ~Token_Float() override {}
};

class Token_Identifier : public Token {
public:
    string value;

    Token_Identifier() : Token(token_identifier), value("") {};

    Token_Identifier(string _value, int _lines = 0) : Token(token_identifier, _lines), value(_value) {}

    string getValue() override { return value; }

    ~Token_Identifier() override {}
};

class Token_Keyword : public Token {
public:
    string value;

    Token_Keyword() : Token(token_keyword), value("") {};

    Token_Keyword(string _value, int _lines = 0) : Token(token_keyword, _lines), value(_value) {}

    string getValue() override { return value; }

    ~Token_Keyword() override {}
};

class Token_Operator : public Token {
public:
    int value;

    Token_Operator() : Token(token_operator), value(UNKNOWN_OP) {};

    Token_Operator(int _value, int _lines = 0) : Token(token_operator, _lines), value(_value) {}

    string getValue() override { return to_string(value); }

    ~Token_Operator() override {}
};

#endif //TEST_TOKEN_H
