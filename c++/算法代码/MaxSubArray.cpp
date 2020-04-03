#include <iostream>
using namespace std;
/**
    最大子序列算法
*/
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

// 递归求最大子序列的和
int MaxSubArray(int arr[], int l, int r) {
    // 设置递归出口 只有一个元素时候退出
    if (r == l) {
        return arr[l];
    }
    // 递归部分
    int c = (l + r) / 2; //自动向下取整数 二分
    int MaxLeft = MaxSubArray(arr, l, c);
    //printf("left %d\n", MaxLeft);
    int MaxRight = MaxSubArray(arr, c + 1, r);
    //printf("right %d\n", MaxRight);
    /* 求横跨左半部分和右半部分的最大子序列和 */
    /* 首先是左半部分子序列中包含最后一个元素的最大子序列和 */

    // borderL左序列之和  borderLMAX左边子序列和的最大值！
    int borderL = arr[c], borderLMAX = arr[c];
    for (int i = c - 1; i >= l; --i) {
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
    //printf("merge %d\n", borderLMAX + borderRMAX);
    // 对左边最大子序列
    int max = MaxThree<int>(MaxLeft, MaxRight, borderLMAX + borderRMAX);
    //printf("return %d\n", max);
    return max;
}


int main() {
    setbuf(stdout, NULL);
    //int a[8] = {4, -3, 5, -2, -1, 2, 9, 2};
    int a[8] = {-4, -3, -5, -2, -1, -2, -9, -2};
    printf("%d", MaxSubArray(a, 0, 7));
    return 0;
}

