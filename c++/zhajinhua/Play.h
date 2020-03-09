#pragma once

//
// Created by zhao on 2019/10/8.
//


#include <iostream>
#include <algorithm>
#include <set>
#include <ctime>
#include "card.h"

#define random(x) (rand()%x)
using namespace std;

class Play {
public:

    Play() {
        cout << "start our game!\n";
    }

    int flush() {
        number_return = getrandomcardsvalue();
        Acards[0] = new card(number_return[1]);
        Acards[1] = new card(number_return[3]);
        Acards[2] = new card(number_return[5]);

        Bcards[0] = new card(number_return[0]);
        Bcards[1] = new card(number_return[2]);
        Bcards[2] = new card(number_return[4]);
        //cout << "\n开始发牌\nA方的牌\t" << endl;
        /*for (int i = 0; i < 3; i++) {
            cout <<Acards[i]->getColor() << "->" << Acards[i]->getValue() << "\t";
        }
        cout << endl<<"B方的牌\t";
        for (int i = 0; i < 3; i++) {
            cout << Bcards[i]->getColor() << "->" << Bcards[i]->getValue() << "\t";
        }*/
        judge_result = judge();
        /*if (judge_result > 0) {
            cout << "\nA yes" << endl;
        }
        else if (judge_result < 0) {
            cout << "\nA no" << endl;
        }
        else {
            cout << "\nA == B" << endl;
        }*/
        return judge_result;
    }

    ~Play() {
        for (int i = 0; i < length; ++i) {
            delete Acards[i];
        }
        for (int i = 0; i < length; ++i) {
            delete Bcards[i];
        }
        delete number_return;
    }

private:
    const static int length = 3;
    card* Acards[length];
    card* Bcards[length];
    int judge_result;
    int* number_return;

    //判断同花
    bool istonghua(const int color[], const int value[], int& tonghua, int length = Play::length) {
        int last = color[0];
        if (length == 1) {
            return true;
        }
        for (int i = 1; i < 4; ++i) {
            if ((last xor color[i]) != 0) {
                return false;
            }
        }
        tonghua = color[0];
        return true;
    }

    //判断顺子 输入的数组为有序数组
    bool isshunzi(const int cards[], int& shunzi, int length = Play::length) {
        int last = cards[0];
        for (int i = 1; i < length; ++i) {
            if (((last + i) xor cards[i]) != 0) {
                return false;
            }
        }
        shunzi = cards[0];
        return true;
    }

    //判断对子  输入为有序数字
    bool isduizi(const int cards[], int& duizi, int length = Play::length) {
        duizi = 0;
        for (int i = 0; i < length - 1; i++) {
            if (cards[i] == cards[i + 1]) {
                duizi = cards[i];
                return true;
            }
        }
        return false;
    }

    bool istongdian(const int cards[], int& tongdian, int length = Play::length) {
        if (cards[0] == cards[length - 1]) {
            tongdian = cards[0];
            return true;
        }
        return false;
    }


    int cp(const int& a, const int& b) {
        if (a > b) {
            return 1;
        }
        else if (a < b) {
            return -1;
        }
        else {
            return 0;
        }
    }

    int judge() {
        int maxA, midA, minA, shunziA, duiziA, tongdianA, tonghuaA;
        int levelA = level(Acards, maxA, midA, minA, shunziA, duiziA, tongdianA, tonghuaA);
        int maxB, midB, minB, shunziB, duiziB, tongdianB, tonghuaB;
        int levelB = level(Bcards, maxB, midB, minB, shunziB, duiziB, tongdianB, tonghuaB);
        if (levelA != levelB) {
            return cp(levelA, levelB);
        }
        if (levelA == 7) {//炸弹
            return cp(tongdianA, tongdianA);
        }
        if (levelA == 6) {//同花顺
            return (midA, midB);
        }
        if (levelA == 5 || levelA == 2) {//同花对
            if (midA != midB) {
                return cp(midA, midB);
            }
            else {
                int temp1 = (midA == minA ? maxA : minA);
                int temp2 = (midB == minB ? maxB : minB);
                return cp(temp1, temp2);
            }
        }
        if (levelA == 4 || levelA == 1) {//单同花
            if (maxA != maxB) {
                return cp(maxA, maxB);
            }
            else if (midA != maxB) {
                return cp(midA, midB);
            }
            else {
                return cp(minA, minB);
            }
        }

        if (levelA == 3) {//单顺子
            return cp(midA, midB);
        }
        return -1;
    }

    int
    level(card* mycard[], int& max, int& mid, int& min, int& shunzi, int& duizi, int& tongdian, int& tonghua) {
        max = 0, mid = 0, min = 0, shunzi = 0, duizi = 0, tongdian = 0, tonghua = 0;
        int value[length];
        int color[length];
        for (int i = 0; i < length; i++) {
            value[i] = mycard[i]->getValue();
            color[i] = mycard[i]->getColor();
        }
        sort(value, value + length);
        sort(color, color + length);
        min = value[0];
        mid = value[1];
        max = value[2];//最大值
        //同花大于顺子
        if (istongdian(value, tongdian)) {
            //            cout << "炸弹" << endl;
        }

        if (istonghua(color, value, tonghua)) {
            //            cout << "同花" << endl;
        }
        if (isshunzi(value, shunzi)) {
            //            cout << "顺子" << endl;
        }
        if (isduizi(value, duizi)) {
            //            cout << "对子" << endl;
        }

        if (tongdian) {
            //炸弹  比tongdian
//            cout << "炸弹" << endl;
            return 7;
        }

        if (tonghua && shunzi) {
            //同花顺  比shunzi（3,4,5 -->min 3）
//            cout << "同花顺" << endl;
            return 6;
        }

        if (tonghua && duizi) {
            return 5;
        }

        if (tonghua) {
            //            cout << "同花" << endl;
            return 4;
        }

        if (shunzi) {
            //            cout << "顺子" << endl;
            return 3;
        }

        if (duizi) {
            //            cout << "对子" << endl;
            return 2;
        }
        return 1;//比putong
    }

    int* getrandomcardsvalue() {
        number_return = new int[6];
        int num = -1;
        int value, i, j;
        bool flag = false;
        for (i = 0; i < 6;) {
            flag = true;
            value = random(52);
            for (j = 0; j <= i; j++) {
                if (value == number_return[j]) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                number_return[i] = value;
                i++;
                continue;
            }
            else {
                continue;
            }
        }
        return number_return;
    }
};


