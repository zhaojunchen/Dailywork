//
// Created by zhao on 2019/10/8.
//

#ifndef ZHAJINHUA_CARD_H
#define ZHAJINHUA_CARD_H

#include <string>
extern void myerror(std::string msg);
class card {
public:

    card(int number) {
        if (number < 0 || number > 51) {
            myerror("输入的number问题");
        }
        value = number / 4 + 2;
        color = (number % 4) + 1;
    }

    int getValue() {
        return value;
    }

    int getColor() {
        return color;
    }

    ~card() {}

private:
    int value;//牌面值 2 = 2  ...   A = 10 ....
    int color;//花色 0.红桃 1.黑桃 2.方块 3.樱花
};

#endif //ZHAJINHUA_CARD_H
