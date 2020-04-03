//
// Created by zhao on 2019/10/4.
//

#ifndef TEST_VARTABLE_H
#define TEST_VARTABLE_H

#include "Token.h"

class Vartable {
public:
    Token *token;
    bool isinit;

    Vartable(Token *_token, bool _isinit = false) : token(_token), isinit(_isinit) {
        if (token == nullptr) {
            cout << "堆指针分配错误";
            exit(-1);
        }
    }
    ~Vartable() {
        delete token;
    }
};

#endif //TEST_VARTABLE_H
