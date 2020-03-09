# mydes 实现64bits的数据加密解密算法

## 1. 关于代码中使用的c++ STL bitset说明

- 由于bitset内部数据结构采用小端的对齐方式，所以二进制序列和预期的完全相反。
举个例子 bitset<8> test(0b11010110) test内部的 test[i] 分为别为0 1 1 0 1 0 1 1。
为了按位操作实现的方便性我们会将bitset的bit序列反过来表示。
举个例子: 待加密的64bits明文的二进制表示： unsigned long long plaintText_littleend = 0b0011000000110001001100100011001100110100001101010011011000110111
为了接口的需求 需要将其转化为bitset<64> 于是 bitset<64> plainText(plaintText_littleend);  plainText 的0..63为分别为倒序的plaintText_littleend
为了方便我们按照c++常规的处理模式（按照数组的0.. length），这里我们写了末班函数来将bitset的bit序列反转。

  ```
  // 倒叙置换保持数据结构
  template<typename T>
  static void swap(T &t) {
      T temp(t);
      int size = t.size() - 1;
      for (int i = 0; i < t.size(); i++) {
          t[i] = temp[size - i];
      }
  }
  swap(plainText) ; 
  ```
  至此 plainText的bit为（0..63）   0b0011000000110001001100100011001100110100001101010011011000110111  与初始化之前的明文结构一直，方便了我们的操作。


- 对bitset的移位操作是相反的 ， 举个例子 上述的plainText (经过swap交换) 为0b0011000000110001001100100011001100110100001101010011011000110111
palinText= palinText<< 1  ----> plainText （0..63）变为 0b0001100000011000100110010001100110011010000110101001101100011011  由此只需要注意一下我们相对bitset的每一位
移位时，反的方向来就是了。


## 算法的流程分析

重点介绍加密算法， 


