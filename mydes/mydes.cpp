//
// Created by zhao on 2019/10/17.
//

#include "mydes.h"


void mydes::setKey(const b64 &key) {
    b56 temp;
    b48 subkey;
    // 初始化 C0 D0
    for (int i = 0; i < 28; i++) {
        currentC[i] = key[pc1_table[i]];
    }
    for (int i = 0; i < 28; i++) {
        currentD[i] = key[pc2_table[i]];
    }

    for (int pass_enc = 0; pass_enc < 16; pass_enc++) {
        int n = loop_table[pass_enc];
        left_shift(currentC, n);
        left_shift(currentD, n);
        for (int i = 0; i < 28; i++) {
            temp[i] = currentC[i];
        }
        for (int i = 0; i < 28; ++i) {
            temp[i + 28] = currentD[i];
        }
        for (int j = 0; j < 48; ++j) {
            subkey[j] = temp[comp_perm[j]];
        }
        subkey_array[pass_enc] = subkey;
    }
}


b64 mydes::encode(const b64 &plainText) {
    initialIpReplacement(plainText);
    b48 internalResult;// 输入拓展
    b48 subkey;
    std::bitset<4> col;//S盒的列
    std::bitset<2> row;//S盒的行
    int box_value;
    b32 result;
    b32 temp;
    for (int pass = 0; pass < 16; ++pass) {
        for (int i = 0; i < 48; ++i)
            internalResult[i] = right_part[expa_perm[i]];
        subkey = subkey_array[pass];
        internalResult ^= subkey;
        for (size_t i = 0; i < 8; ++i) {//8个盒子
            row[0] = internalResult[6 * i];//获取行标
            row[1] = internalResult[6 * i + 5];
            col[0] = internalResult[6 * i + 1];//获取列标
            col[1] = internalResult[6 * i + 2];
            col[2] = internalResult[6 * i + 3];
            col[3] = internalResult[6 * i + 4];
            box_value = s_box[i][tolonglong(row)][tolonglong(col)];
            bitset<4> sbox_output(box_value);

            swap(sbox_output);
            for (size_t j = 0; j < 4; ++j)
                result[4 * i + j] = sbox_output[j];//将32位暂存于48位中
        }

        for (size_t i = 0; i < result.size(); ++i)
            temp[i] = result[p_table[i]];//P盒置换
        temp ^= left_part;
        left_part = right_part;
        right_part = temp;
    }
    return initialIpReverseReplacement();

}

b64 mydes::decode(const b64 &ciperText) {
    initialIpReplacement(ciperText);
    b48 internalResult;// 输入拓展
    b48 subkey;
    std::bitset<4> col;//S盒的列
    std::bitset<2> row;//S盒的行
    int box_value;
    b32 result;
    b32 temp;
    for (int pass = 15; pass >= 0; --pass) {
        for (int i = 0; i < 48; ++i)
            internalResult[i] = right_part[expa_perm[i]];
        subkey = subkey_array[pass];
        internalResult ^= subkey;
        for (size_t i = 0; i < 8; ++i) {//8个盒子
            row[0] = internalResult[6 * i];//获取行标
            row[1] = internalResult[6 * i + 5];
            col[0] = internalResult[6 * i + 1];//获取列标
            col[1] = internalResult[6 * i + 2];
            col[2] = internalResult[6 * i + 3];
            col[3] = internalResult[6 * i + 4];
            box_value = s_box[i][tolonglong(row)][tolonglong(col)];
            bitset<4> sbox_output(box_value);

            swap(sbox_output);
            for (size_t j = 0; j < 4; ++j)
                result[4 * i + j] = sbox_output[j];//将32位暂存于48位中
        }

        for (size_t i = 0; i < result.size(); ++i)
            temp[i] = result[p_table[i]];//P盒置换
        temp ^= left_part;
        left_part = right_part;
        right_part = temp;
    }
    return initialIpReverseReplacement();

}


//ip置换 并且分割
void mydes::initialIpReplacement(const b64 &plaintext) {
    for (int i = 0; i < left_part.size(); i++) {
        left_part[i] = plaintext[ip_table[i]];
    }
    for (int i = 32; i < 64; i++) {
        right_part[i - 32] = plaintext[ip_table[i]];
    }
}

// ip 逆置换
b64 mydes::initialIpReverseReplacement() {
    b64 result;
    for (int i = 0; i < 32; ++i) {
        result[i] = right_part[i];
    }
    for (int j = 0; j < 32; ++j) {
        result[32 + j] = left_part[j];
    }
    b64 temp(result);
    for (int i = 0; i < 64; i++) {
        result[i] = temp[ipr_table[i]];
    }
    return result;
}

