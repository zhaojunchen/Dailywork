#define _CRT_SECURE_NO_WARNINGS


#include <fstream>
#include "cmdline.h"
#include <cstdint>
#include<iostream>
#include<vector>
#include <algorithm>
#include <regex>
#include <cassert>
#include <io.h>
#include <windows.h>
#include <string>
#include "fat32.h"


typedef unsigned char us;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
using namespace std;


// ���� ��С !
u16 Bytes_per_Sector = 512;
u16 sectorSize = Bytes_per_Sector;
// ÿһ�ص�����
u8 Sectors_per_Cluster = 8;
u64 clusterSize = Bytes_per_Sector * Sectors_per_Cluster;
// ��������
u16 Reserved_Sector = 8238;
// FAT�����Ŀ
u8 FATs = 2;
// FAT��ռ�õ�������
u32 Sectors_per_FAT_FAT32 = 8169;
// �״ر��
u32 Root_Cluster_Number = 2;
//FAT����ʼ��ַ
u64 baseFat = Reserved_Sector * Bytes_per_Sector;
// �״���ʼ��ַ
u64 baseCluster = Bytes_per_Sector * (Reserved_Sector + Sectors_per_FAT_FAT32 * FATs);
// ��Ŀ¼���ڵ�λ��
u64 baseDir = baseCluster;

// ����غ� ���ض�Ӧ�Ĵص�ƫ��
inline UINT64 offsetCluster(UINT64 in) {
    return baseCluster + (in - Root_Cluster_Number) * clusterSize;
}

inline u64 _offsetCluster(u64 offset) {
    return (offset - baseCluster) / clusterSize + 2;
}

/// ���������� ���� ��Ӧ��ƫ��
inline UINT64 offsetSector(UINT64 in) {
    return sectorSize * in;
}

inline UINT64 _offsetSector(UINT64 offset) {
    return offset / sectorSize;
}


inline u64 FatAddress(u32 clusterIndex) {
    return clusterIndex * 4 + baseFat;
}

static UINT16 uint8to16(UINT8 twouint8[2]) {
    return *(UINT16 *) twouint8;
}

static UINT32 uint8to32(UINT8 fouruint8[4]) {
    return *(UINT32 *) fouruint8;
}

static UINT64 uint8to64(UINT8 eightuint8[8]) {
    return *(UINT64 *) eightuint8;
}

static wstring UTF8ToUnicode(const string &s) {
    wstring result;
    // ��û������Ŀ��ַ�����
    int length = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    wchar_t *buffer = new wchar_t[length];
    ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buffer, length);
    result = buffer;
    delete[] buffer;
    return result;
}

static void printByte(string str) {
    int i = 0;
    for (i = 0; i < str.length(); i++) {
        printf("%X ", (unsigned char) str.at(i));
    }
    printf("\n");
}

static void wprintByte(wstring str) {
    int i = 0;
    for (i = 0; i < str.length() * sizeof(wchar_t); i++) {
        printf("%X ", *((unsigned char *) str.c_str() + i));
    }
    printf("\n");
}

// �ַ����ָ����
static vector<string> split(const string &str, const string &pattern) {
    vector<string> res;
    if (str == "")
        return res;
    //���ַ���ĩβҲ����ָ����������ȡ���һ��
    string strs = str + pattern;
    size_t pos = strs.find(pattern);
    while (pos != strs.npos) {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        //ȥ���ѷָ���ַ���,��ʣ�µ��ַ����н��зָ�
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }
    return res;
}

static LPCWSTR stringToLPCWSTR(std::string orig) {
    wchar_t *wcstring = 0;
    try {
        size_t origsize = orig.length() + 1;
        const size_t newsize = 100;
        size_t convertedChars = 0;
        if (orig == "") {
            wcstring = (wchar_t *) malloc(0);
            mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
        } else {
            wcstring = (wchar_t *) malloc(sizeof(wchar_t) * (orig.length() - 1));
            mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
        }
    }
    catch (std::exception e) {
    }
    return wcstring;
}

static void show(unsigned char *arr, int length) {
    if (length) {
        printf("%s ", "\n");
        for (int i = 0; i < length; i++) {
            printf("%02X ", arr[i]);
            if ((i + 1) % 16 == 0) {
                printf("%s ", "\n");
            }
        }
        printf("%s ", "\n");
    }
}

void volumeTravel();

HANDLE OpenDisk(char partition, bool onlyRead = false) {
    string path = "\\\\.\\f:";
    path[4] = partition;
    DWORD dwDesiredAccess;
    if (onlyRead) {
        dwDesiredAccess = FILE_SHARE_READ;
    } else {
        dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
    }
    // TODO visual ��clionò���е�����
    HANDLE hDevice = CreateFileW(stringToLPCWSTR(path),
                                 dwDesiredAccess,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);
    if (hDevice == INVALID_HANDLE_VALUE) { // cannot open the drive
        cout << "May be no permission!Or no such partition!\n";
        exit(-1);
    }
    return hDevice;
}

//����HANDLE�ĵ�ǰָ�� Ĭ��Ϊ��ͷ��ʼ�ƶ�
void seekDisk(HANDLE handle, UINT64 offset, DWORD dwMoveMode = FILE_BEGIN) {
    assert(offset % 512 == 0);
    LARGE_INTEGER currentPointer;
    LARGE_INTEGER distance;
    distance.QuadPart = offset;
    currentPointer.QuadPart = 0;
    SetFilePointerEx(handle, distance, &currentPointer, dwMoveMode);
    if (dwMoveMode == FILE_BEGIN && currentPointer.QuadPart != distance.QuadPart) {
        std::cout << "line:" << __LINE__ << "error  error code:" << GetLastError() << std::endl;
        exit(-1);
    }
}

static DWORD ReadDisk(HANDLE hDevice, us *&out, u64 start, u64 size) {
    DWORD readSize = 0;
    out = new us[size + 1];// �ڴ�й¶��
    memset(out, 0, sizeof(us) * size + 1);
    //��ǰ�ļ�ƫ��
    seekDisk(hDevice, start);
    bool ok = ReadFile(hDevice, out, size, &readSize, 0);
    if (size != readSize || ok == false) {
        cout << "LINE:" << __LINE__ << " error Read File Error\n";
        cout << GetLastError();
        exit(-2);
    }
    return readSize;
}

static DWORD WriteDisk(HANDLE hDevice, us *&in, u64 start, u64 size) {
    assert(size % 512 == 0);
    DWORD dwBytesReturned = 0;
    //������
# if 1
    if (!DeviceIoControl(hDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwBytesReturned, NULL)) {
        printf("\n������ʧ�ܣ�ʧ��ԭ��:%d", GetLastError());
        CloseHandle(hDevice);
        exit(-1);
    }
#endif
    seekDisk(hDevice, start);
    DWORD writeSize = 0;
    bool ok = WriteFile(hDevice, in, size, &writeSize, 0);
    if (!ok || writeSize != size) {
        cout << "LINE��" << __LINE__ << "   ERROR " << GetLastError();
        CloseHandle(hDevice);
        exit(-1);
    }
# if 1
    if (!DeviceIoControl(hDevice, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dwBytesReturned, NULL)) {
        printf("\n\n���̽���ʧ��!������:%d", GetLastError());
    }
# endif
}

DWORD WriteCluster(HANDLE handle, us *in, DWORD startCluster, DWORD numberCluster = 1) {
    WriteDisk(handle, in, offsetCluster(startCluster), numberCluster * clusterSize);
}

// ��ȡ��������
DWORD ReadSector(HANDLE handle, us *&out, DWORD startSector, DWORD numberSector = 1) {
    DWORD readSize = 0;
    DWORD size = numberSector * Bytes_per_Sector;// �����ֽ���
    return ReadDisk(handle, out, offsetSector(startSector), size);
}

// ��ȡ���ɴ�
DWORD ReadCluster(HANDLE handle, us *&out, DWORD startCluster, DWORD numberCluster = 1) {
    DWORD readSize = 0;
    DWORD size = numberCluster * clusterSize;// �����ֽ���
    return ReadDisk(handle, out, offsetCluster(startCluster), size);
}

bool FileExist(const std::string &name);

// �����ļ�·�� ����û�����ĳ���
vector<string> ParseFilePath(const string &path = "f:/cmd/main.cpp") {
    // �ļ����� �����·���ǣ� C:/test/test.c ��ʽ��
    // �ж��ļ��Ƿ����
    // ���ж��ļ��Ƿ����
    if (!FileExist(path)) {
        cerr << "LINE " << __LINE__ << "ERROR  FILE NOT EXIST!" << endl;
        exit(-1);
    }
    vector<string> res = split(path, "/");
    string partition(path, 0, 3);
    std::regex reg("^[A-Za-z]:/");
    smatch result;
    if (!regex_match(partition, result, reg)) {
        cout << "���������·����";
        exit(-1);
    }
    // ��ʼ������
    return res;
}

// DBR����ȫ�ֱ����ĳ�ʼ����
pFAT32_DBR INIT_FAT32_DBR(HANDLE handle) {
    pFAT32_DBR dbr = new FAT32_DBR();
    ReadDisk(handle, (us *&) dbr, 0, 512);
    // ������С 512
    Bytes_per_Sector = dbr->BPB.Bytes_per_Sector;
    sectorSize = Bytes_per_Sector;
    // ÿһ�ص����� 8
    Sectors_per_Cluster = dbr->BPB.Sectors_per_Cluster;
    clusterSize = Bytes_per_Sector * Sectors_per_Cluster;
    // �������� 8238
    Reserved_Sector = dbr->BPB.Reserved_Sector;
    // FAT�����Ŀ 2
    FATs = dbr->BPB.FATs;
    // FAT��ռ�õ������� 8169
    Sectors_per_FAT_FAT32 = dbr->BPB.Fat32_Sector.Sectors_per_FAT_FAT32;
    //  ��ʼ�غ�
    Root_Cluster_Number = dbr->BPB.Fat32_Sector.Root_Cluster_Number;

    //FAT����ʼ��ַ
    baseFat = Reserved_Sector * Bytes_per_Sector;
    // �״���ʼ��ַ��2�ţ�
    baseCluster = Bytes_per_Sector * (Reserved_Sector + Sectors_per_FAT_FAT32 * FATs);
    // ��Ŀ¼���ڵ�λ��
    baseDir = baseCluster;
    return dbr;
}

inline u64 offsetBySector(u64 sectorNumber) {
    return Bytes_per_Sector * sectorNumber;
}

// ���ݴغš�Ѱ�ҵ��ôص�ƫ�ƣ�
inline u64 offsetByCluster(u64 clusterNumber) {
    assert(clusterNumber >= Root_Cluster_Number);
    return Bytes_per_Sector * (Reserved_Sector + FATs * Sectors_per_FAT_FAT32) +
           (clusterNumber - Root_Cluster_Number) * Bytes_per_Sector * Sectors_per_Cluster;
}

// ����UINT32�Ĳ�ֵ��(������)
static u32 diff(u32 a, u32 b) {
    if (a > b) {
        return a - b;
    } else {
        return b - a;
    }
}

// �����״غ�Ѱ�Ҵ��������ش���������
vector<u32> findClusterList(HANDLE handle, const u32 &clusterIndex) {
    vector<u32> clusterList;
    clusterList.push_back(clusterIndex);
    u32 endFlag = 0x0FFFFFFF;
    us *out = nullptr;// һ�������� TODO �ͷſռ�
    // TODO ����һ�ζ�ȡһ���� (8*��) �ļ���СУ�飡 ע������û��ʹ�ôغ������� FAT�ڿ�ʼ�غ�֮ǰ
    // ��ǰFAT��������������
    u8 numberSector = 64;// ÿ�ζ�ȡ����������
    u32 currentSector = _offsetSector(FatAddress(clusterIndex));
    ReadSector(handle, out, currentSector, numberSector);
    us *start = out + ((FatAddress(clusterIndex) - offsetSector(currentSector)) % (sectorSize * numberSector));
    u32 nextClusterIndex = uint8to32(start);
    u32 nextSector;// ��һ�ض�ӦFAT��ƫ�ƣ�
    while (nextClusterIndex != endFlag) {// ��ǰ��Ŀ��
        clusterList.push_back(nextClusterIndex);
        nextSector = _offsetSector(FatAddress(nextClusterIndex));
        if (diff(nextSector, currentSector) < numberSector) {
            // �����ٶ�ȡ��һ���̵�Ԫ  ��λ��һ�شغ����ڵĵ�ַ��
            start = out +
                    (((FatAddress(nextClusterIndex) - offsetSector(currentSector)) % (sectorSize * numberSector)));
            nextClusterIndex = uint8to32(start);
        } else {
            delete[]out;
            ReadSector(handle, out, nextSector, numberSector);
            start = out + ((FatAddress(nextClusterIndex) - offsetSector(nextSector)) % (sectorSize * numberSector));
            nextClusterIndex = uint8to32(start);
            currentSector = nextSector;
        }
    }
    delete[]out;
    return clusterList;
}

bool compareLDT(const wstring &target, us *&p, const int &skip) {
    wchar_t *copy = new wchar_t[skip * 13];
    memset(copy, 0, skip * 26);
    int i, j, k = 0;
    us *start, *p0;
    p0 = p;
    for (i = 0; i < skip; i++) {
        start = p0 + 0x1;
        for (j = 0; j < 5; j++) {
            copy[k] = *(wchar_t *) start;
            start += 2;
            k++;
        }
        start = p0 + 0xE;
        for (j = 0; j < 6; j++) {
            copy[k] = *(wchar_t *) start;
            start += 2;
            k++;
        }
        start = p0 + 0x1c;
        for (j = 0; j < 2; j++) {
            copy[k] = *(wchar_t *) start;
            start += 2;
            k++;
        }
        p0 -= 32;
    }
    wstring copy_wstring(copy);
    if (copy_wstring == target) {
        delete[]copy;
        return true;
    } else {
        delete[]copy;
        return false;
    }
}

u32 returnNextCluster(us *&start) {
    return (uint8to16(start + 0x14) << 16) + uint8to16(start + 0x1A);
}

// ����Ŀ¼������Ŀ¼� ���������� �ϼ�Ŀ¼����ʼ��ַ��
u32 Parse_FDT_Dir(HANDLE handle, string directory, u32 parentCluster = 2) {
    us *out, *start;
    u32 numberCluster = 20;// ���������start��ֵĴ�
    ReadCluster(handle, out, parentCluster, numberCluster);
    wstring target = UTF8ToUnicode(directory);
    start = out;
    u32 skip;
    pFDT fdt = new FDT();
    pLDT ldt;
    pSDT sdt;
    bool isLDT = true;
    string upperString = directory;//��дĿ¼��
    transform(upperString.begin(), upperString.end(), upperString.begin(), ::toupper);
    int i;
    vector<string> list;
    string end = " ";
    bool match = false;
    if (upperString.find('.') != string::npos) {
        // �ļ�
        list = split(upperString, ".");
        if (list[0].length() <= 8 && list[1].length() <= 3) {
            isLDT = false;
            for (i = list[0].length(); i < 8; i++) {
                list[0].append(end);
            }
            for (i = list[1].length(); i < 3; i++) {
                list[1].append(end);
            }
        } else {
            isLDT = true;
        }
    } else {
        // Ŀ¼
        if (upperString.length() <= 8) {
            isLDT = false;
            for (i = upperString.length(); i < 11; i++) {
                upperString.append(end);
            }
        } else {
            isLDT = true;
        }
    }
    while (*start != 0x0) {
        // ��Ŀ�Ѿ���ɾ����
        if (*start == 0xE5) {
            start += 32;
        } else if (*(start + 0xB) == 0xF) {
            skip = ((*start) ^ 0x40);
            if (isLDT) {
                start += (skip - 1) * 32;
                if (compareLDT(target, start, skip)) {
                    // �ҵ���Ŀ���ͷ�ļ� ���Ķ�Ŀ¼�����Ŀ�� start+32!
                    start += 32;
                    u32 nextCluster = returnNextCluster(start);
                    delete[]out;
                    delete fdt;
                    return nextCluster;
                } else {// ����ƥ�䣡
                    start += 32 * 2;
                }
            } else {
                start += skip * 32 + 32;
            }
        } else {
            // ��Ŀ¼��ƥ�䣡
            if (isLDT) {
                start += 32;
                continue;
            }
            // ȷ���Ƕ�Ŀ¼�
            memcpy(fdt, start, 32);
            sdt = (pSDT) fdt;
            if (list.size() == 2) {
                match = true;
                // �ж�Ϊ�� �ļ�
                for (i = 0; i < 8; i++) {

                    if ((us) list[0][i] != sdt->filename[i]) {
                        // ��ƥ��
                        start += 32;
                        match = false;
                        break;
                    }
                }
                if (match) {
                    for (i = 0; i < 3; i++) {
                        if ((us) list[1][i] != sdt->filename[i + 8]) {
                            // ��ƥ��
                            start += 32;
                            break;
                        } else {
                            // ƥ����ȷ
                            u32 nextCluster = returnNextCluster(start);
                            delete[]out;
                            delete fdt;
                            return nextCluster;
                        }
                    }
                }

            } else {//�ж�Ϊ�� Ŀ¼
                match = true;
                for (i = 0; i < 11; i++) {
                    if ((us) upperString[i] != sdt->filename[i]) {
                        // ��ƥ��
                        start += 32;
                        match = false;
                        break;
                    }
                }
                if (match) {
                    u32 nextCluster = returnNextCluster(start);
                    delete[]out;
                    delete fdt;
                    return nextCluster;
                }
            }

        }

    }

}

/*��ʼ������*/
void rc4_init(unsigned char *s, unsigned char *key, unsigned long Len) {
    int i = 0, j = 0;
    char k[256] = {0};
    unsigned char tmp = 0;
    for (i = 0; i < 256; i++) {
        s[i] = i;
        k[i] = key[i % Len];
    }
    for (i = 0; i < 256; i++) {
        j = (j + s[i] + k[i]) % 256;
        tmp = s[i];
        s[i] = s[j]; //����s[i]��s[j]
        s[j] = tmp;
    }
}

/*�ӽ���*/
void rc4_crypt(unsigned char *s, unsigned char *Data, unsigned long Len) {
    int i = 0, j = 0, t = 0;
    unsigned long k = 0;
    unsigned char tmp;
    for (k = 0; k < Len; k++) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        tmp = s[i];
        s[i] = s[j]; //����s[x]��s[y]
        s[j] = tmp;
        t = (s[i] + s[j]) % 256;
        Data[k] ^= s[t];
    }
}

int isSequence(const vector<u32> &list) {
    assert(list.size() > 0);
    if (list.size() < 2) {
        return 1;
    }
    for (int i = 0; i < list.size() - 1; i++) {
        if (list[i] + 1 != list[i + 1]) {
            return 1;
        }
    }
    int once = 1000;
    return list.size() < once ? list.size() : once;
}

void work(cmdline::parser &cmd) {

    // ·������
    //string path = "f:/cmdline-master/main.cpp";
    string path = cmd.rest()[0];
    vector<string> res = ParseFilePath(path);
    // �򿪴��̾��
    HANDLE handle = OpenDisk(res[0][0]);
    // ��ʼ��DBR��Ϣ
    pFAT32_DBR dbr = INIT_FAT32_DBR(handle);
    // �ͷ�dbr�ռ�
    delete dbr;
    // �״صĴغ�
    u32 nextCluster = Root_Cluster_Number;
    // ����Ѱ��Ŀ���ļ����״غţ�
    for (int i = 1; i < res.size(); i++) {
        nextCluster = Parse_FDT_Dir(handle, res[i], nextCluster);
    }
    // �����״غ�Ѱ�Ҵ���
    vector<u32> clusterList = findClusterList(handle, nextCluster);
    if (cmd.exist("show")) {
        cout << "\n�ļ�:" << path << "�Ĵغ�����: ��������(" << clusterList.size() << ")\n";
        int i = 0;
        for (const auto it : clusterList) {
            if (++i % 8 == 0) {
                cout << "\n";
            }
            cout << it << "\t";
        }
    }
    if (cmd.exist("encode") || cmd.exist("decode")) {
        // ����RC4��ʽ����!
        unsigned char s[256] = {0}; //S-box
        string key;
        if (cmd.exist("encode") && cmd.exist("decode")) {
            exit(-1);
        } else if (cmd.exist("encode")) {
            key = cmd.get<string>("encode");
        } else {
            key = cmd.get<string>("decode");
        }
        rc4_init(s, (unsigned char *) key.c_str(), key.length()); //�Ѿ�����˳�ʼ��
        // ��ȡ��������Ϣ�� ���������˳��Ļ���һ���Զ�ȡ���еĴ�����
        us *out = nullptr;
        // ÿһ�ζ���ȡ����Ŀ��
        int onceCluster = isSequence(clusterList), all = clusterList.size();// �����˳��Ļ���һ�ζ�ȡ1000��

        int i = 0, j;
        while (all >= onceCluster) {
            // �ռ��ԣ��
            ReadCluster(handle, out, clusterList[i * onceCluster], onceCluster);
            for (j = 0; j < onceCluster; j++) {
                rc4_crypt(s, (us *) (out + j * clusterSize), clusterSize);
            }
            //�ӽ���
            WriteCluster(handle, out, clusterList[i * onceCluster], onceCluster);
            all -= onceCluster;
            i++;
        }
        if (all <= 0) {
            cout << "\n����/������ɣ�\n";
            delete[]out;
            return;
        } else {// ִ�е����˵��onceCluster��=1 ���Ǵ����������ģ�
            ReadCluster(handle, out, clusterList[i * onceCluster], all);
            for (j = 0; j < all; j++) {
                rc4_crypt(s, (us *) (out + j * clusterSize), clusterSize);
            }
            WriteCluster(handle, out, clusterList[i * onceCluster], all);
            delete[]out;
            cout << "\n����/������ɣ�\n";
            return;
        }

    }
}


int main(int argc, char *argv[]) {
    cmdline::parser cmd;
    cmd.add("show", 'c', "show file node information");
    cmd.add<string>("encode", 'e', "encode file node by key", false, "");
    cmd.add<string>("decode", 'd', "decode file node by key", false, "");
    cmd.add("help", '?', "print this message");
    cmd.footer("filename ...");
    cmd.set_program_name("Cluster system!");

    bool ok = cmd.parse(argc, argv);
    if (argc == 1 || cmd.exist("help")) {
        cerr << cmd.usage();
        return 0;
    }
    if (!ok) {
        cerr << cmd.error() << endl << cmd.usage();
        return 0;
    }
    string curFileName;

    vector<string> fileCollection;
    for (size_t i = 0; i < cmd.rest().size(); i++) {
        curFileName = cmd.rest()[i];
        fileCollection.push_back(curFileName);
        // �ж��ļ��Ƿ���ڣ�
        if (!FileExist(curFileName)) {
            cerr << curFileName << " Not exist!";
            exit(-1);
        }
    }
    work(cmd);

    // ����·���ļ� �õ�Ŀ¼�����ʼ�غţ�

    //delete[]out;
    //CloseHandle(handle);
    system("pause");
    return 0;
}

bool FileExist(const std::string &name) {
    ifstream f(name.c_str());
    return f.good();
}

void volumeTravel() {
    char VolumeDeviceString[0x500] = {0};
    // ǰһ���ֽ�Ϊ��Ϣ���ͣ������52�ֽ�Ϊ���������������
    BYTE BufferData[0x1000] = {0};
    char FileSystem[MAX_PATH] = {0};
    char *Travel = NULL;
    GetLogicalDriveStringsA(sizeof(VolumeDeviceString), VolumeDeviceString);
    //�����������Ϣ
    /*
    0x001FF228  43 3a 5c 00 45 3a 5c 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  C:\.E:\.............
    */

    //0018F460  43 3A 5C 00 44 3A 5C 00 45 3A 5C 00 46 3A  C:\.D:\.E:\.F:
    //0018F46E  5C 00 47 3A 5C 00 48 3A 5C 00 4A 3A 5C 00  \.G:\.H:\.J:\.
    Travel = VolumeDeviceString;

    DWORD Offset = 0;
    for (Offset = 1; *Travel != '\0'; Travel += lstrlenA(Travel) + 1)   //�����+1Ϊ�˹�\0
    {
        memset(FileSystem, 0, sizeof(FileSystem));  //�ļ�ϵͳ NTFS
        // �õ��ļ�ϵͳ��Ϣ����С
        GetVolumeInformationA(Travel, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);
        ULONG FileSystemLength = lstrlenA(FileSystem) + 1;

        int a = 0;
    }
}



