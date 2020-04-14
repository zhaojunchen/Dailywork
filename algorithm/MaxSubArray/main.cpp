#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cassert>

using namespace std;

// 时间复杂度为n  空间复杂度为n
int MaxSubArray1(int *arr, int l, int r) {
    int m;
    int s[r - l + 1];
    m = s[0] = arr[l];
    for (int i = l + 1; i <= r; i++) {
        if (s[i - 1] < 0) {
            s[i] = arr[i];//舍弃以前的结果
        } else {
            s[i] = arr[i] + s[i - 1];
        }
        m = max(s[i], m);
    }
    return m;
}

/**
 * 设s[i]为 arr[l,i-1]的最大子序列的和+arr[i]
 * 鉴于s[i-1] 累计为一个负数、 他对最优解的贡献完全是一个负数！
 * 即使可以舍弃一个解 将其设置为0 从新计算
 * 鉴于arr[i]可能一个负数 显然的s[i-1]局部上相对于s[i]更优
 * s[i] = s[i-1]>=0?s[i-1]+arr[i]:arr[i]
 * 使用max记录这s[i]过程中能出现的最大值！
 * 由于s[r-l+1] 不断地迭代、完全不需要这个数组、只需要s_old和s_new并且即使更新！
 * 即可优化了空间复杂度！ 时间复杂度n 空间复杂度为常数
 * */

/**
 * 将之前更为优化
 * */
int MaxSubArray2(int *arr, int l, int r) {
    //
    int m;
    int s_old, s_new;
    m = s_old = arr[l];
    for (int i = l + 1; i <= r; i++) {
        if (s_old < 0) {
            s_new = arr[i];//舍弃以前的结果
        } else {
            s_new = arr[i] + s_old;
        }
        m = max(s_new, m);
        s_old = s_new;
    }
    return m;
}


// 三个数求和
template<typename T>
T MaxThree(const T &a, const T &b, const T &c) {
    T max = a;
    if (a < b) {
        max = b;
    }
    if (max < c) {
        max = c;
    }
    return max;
}

// 分治（递归）求最大子序列的和 复杂度 nlogn
int MaxSubArray0(int *arr, int l, int r) {
    // 设置递归出口 只有一个元素时候退出
    if (r == l) {
        return arr[l];
    }
    // 递归部分
    int c = (l + r) / 2; //自动向下取整数 二分
    int MaxLeft = MaxSubArray0(arr, l, c);
    int MaxRight = MaxSubArray0(arr, c + 1, r);
    /* 求横跨左半部分和右半部分的最大子序列和 */
    /* 首先是左半部分子序列中包含最后一个元素的最大子序列和 */

    // borderL左序列之和  borderLMAX左边子序列和的最大值！
    int borderL = arr[c], borderLMAX = arr[c];
    for (int i = c - 1; i >= l; i--) {
        borderL += arr[i];
        if (borderL > borderLMAX) {
            borderLMAX = borderL;
        }
    }
    // borderR右序列之和  borderRMAX右子序列和的最大值！
    int borderR = arr[c + 1], borderRMAX = arr[c + 1];
    for (int i = c + 2; i <= r; i++) {
        borderR += arr[i];
        if (borderR > borderRMAX) {
            borderRMAX = borderR;
        }
    }
    return MaxThree<int>(MaxLeft, MaxRight, borderLMAX + borderRMAX);
}


int main(int argc, char const *argv[]) {
    setbuf(stdout, NULL);
    srand((int) time(0));  // 产生随机种子  把0换成NULL也行
    /* code */
    int max0, max1, max2, arrlen;
    int MAXLEN = 100;
    int arr[MAXLEN];
    for (int i = 0; i < 1000; i++) {
        arrlen = rand() % MAXLEN;
        if (arrlen < 2) {
            continue;
        }
        for (int j = 0; j < arrlen; j++) {
            arr[j] = rand() % 200;
        }
        max0 = MaxSubArray0(arr, 0, arrlen - 1);
        max1 = MaxSubArray0(arr, 0, arrlen - 1);
        max2 = MaxSubArray0(arr, 0, arrlen - 1);
        assert(max0 == max1 && max1 == max2);
    }
    cout << "end\n";
    return 0;
}

