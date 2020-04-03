#include <iostream>
#include "Play.h"
using namespace std;

void myerror(string msg);

int main() {
    srand((int)time(NULL));
    //初始化种子
    clock_t start, finish;
    double totaltime;
    start = clock();
    int Ayes = 0;
    int Ano = 0;
    int Aeq = 0;
    int myvalue;
    cout << "start...\n";
    int total = 10000000;
    Play play;
    for (int i = 0; i < total; i++) {
        myvalue = play.flush();
        if (myvalue > 0) {
            Ayes += 1;
        }
        else if (myvalue < 0) {
            Ano += 1;
        }
        else {
            Aeq += 1;
        }
        /*cout << "--------\n";*/
    }
    cout << total << endl;
    cout << "yes\t" << Ayes << "\nno\t" << Ano << "\neq\t" << Aeq << endl;
    cout << "\nyes\t" << (double)Ayes / total << "\nno\t" << (double)Ano / total << "\neq\t" << (double)Aeq / total << endl;
    finish = clock();
    totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
    cout << "\nrunning " << totaltime << "mils" << endl;

    //    cout << endl << pokerlist.size() << endl;
    return 0;
}

void myerror(string msg) {
    cout << __LINE__ << "error " << msg << endl;
}

