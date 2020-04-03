#include "mydes.h"

int main() {
    ll key_little = 0b0011000100110010001100110011010000110101001101100011011100111000;
    b64 key(key_little);
    swap(key);
    print(key, "key is:");

    ll plaintText_little = 0b0011000000110001001100100011001100110100001101010011011000110111;
    b64 plainText(plaintText_little);
    swap(plainText);
    print(plainText, "plainText is:");

    //standard data model
    //plainText : 0011000000110001001100100011001100110100001101010011011000110111
    //ciperText : 1000101110110100011110100000110011110000101010010110001001101101
    //key is    : 0011000100110010001100110011010000110101001101100011011100111000

    mydes zjc(key);
    b64 ciperText = zjc.encode(plainText);
    print(ciperText, "ciperText is:");
    print(zjc.decode(ciperText), "jiemi is:");
    return 0;
}