#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>
#include <set>
#include <map>
#include <cstdlib>
#include <regex>
#include "Token.h"
#include "Vartable.h"

using namespace std;

// https://devdocs.io/cpp/container/set/find
const static set<string> keywords = {"float", "int", "write"};
map<string, Vartable *> vartable;

int lines = 0;//行
void error(const string &msg = "", int line = __LINE__);

//解析全文的token序列 输入有待解析的序列
vector<Token *> parseToken(const string &context);

void deleteHeap(vector<Token *> &tokens);

/**输入文件名称 返回文件内容 */
string readFileIntoString(const char *filename);

string stmt(const vector<Token *> &subtokes, int &offset, bool return_type = true);//0 int 1 bool
string exp_to_postexp(string &exp);

template<typename T>
void calculate_postexp(string &postexp, T &result);

//解析每一行token
int paser(const vector<Token *> &tokens);

string fushuproblem(const string &exp);

bool checkyou(const string &exp) {
    string pattern = "([\\d.]+)\\s*([*/])\\s*([\\d.]+)";
    regex re(pattern);
    if (regex_match(exp, re)) {
        return true;
    } else {
        return false;
    }
}

int main() {

    auto var = &vartable;
    string testexample = "((12-56-20)/(4+2)*2.2*12)";
    bool isOk = checkyou(testexample);
    //sourcecode带解析的原文本
    string sourcecode = readFileIntoString("../test.txt");
    cout << "start \tparse...\n";
    vector<Token *> tokens = parseToken(sourcecode);
    paser(tokens);
    deleteHeap(tokens);
    cout << "end \tparse\n";
    return 0;
}


void error(const string &msg, int line) {
    cout << "error at " << line << " line\t " << msg << endl;
    exit(-1);
}

//解析全文的token序列 输入有待解析的序列
vector<Token *> parseToken(const string &context) {
    vector<Token *> tokens;
    lines = 1;//初始化全局变量lines记录行号
    int len = context.length();
    int offset = 0;
    char ch = ' ';
    int intvalue = 0;
//    float times, floatvalue = 0;
    string str_value;
    //识别单词
    string word = "";//
    while (offset < len) {
        ch = context[offset];
        if (ch == ' ' || ch == '\t' || ch == '\n') {
            ++offset;//偏移一个单位
            if (ch == '\n')
                lines++;
            continue;
        }
        if (ch == ';') {//分号 添加一个结束符来标记
            tokens.push_back(new Token(token_eof, lines));
            ++offset;
            continue;
        }
        //符号 token
        if (ch == LHS || ch == RHS || ch == MUL || ch == PLUS || ch == DIV || ch == SUB || ch == EQ) {
            tokens.push_back(new Token_Operator(ch, lines));
            offset++;
            continue;
        }
        //判断为数字或者浮点数类型 假设为正数
        if (isdigit(ch)) {
            str_value = "";// 初始化token的string
            do {
                str_value += ch;
                ++offset;
                if (offset < len) {
                    ch = context[offset];// 更新ch
                } else { //输入完毕 解析完成（当然实际上不可能出现这个状况）
                    return tokens;
                }
            } while (isdigit(ch));
            //判断是否为浮点数
            if (ch != '.') {
                // offset不更新
                //添加tokens
                tokens.push_back(new Token_Int(str_value, lines));
                continue;
            }
            if (offset == len - 1) {//解决123. 结束的问题
                tokens.push_back(new Token(token_eof, lines));
                return tokens;//返回token序列
            }
            //小数部分
            str_value += '.';// 加上小数点
            ++offset;
            if (offset >= len) {
                return tokens;//判断长度
            } else {
                ch = context[offset];
            }
            do {
                str_value += ch;
                ++offset;
                if (offset < len) {
                    ch = context[offset];
                } else { //输入完毕 解析完成（当然实际上不可能出现这个状况）
                    error("you need end symbols");//
                }
            } while (isdigit(ch));
            //不更新offset
            //添加token
            tokens.push_back(new Token_Float(str_value, lines));// 转为为 string类型传入地址
            continue;
        }
        //判断为标识符(关键字和标识符)
        if (isalpha(ch) || ch == '_') {
            word.clear();//初始化
            do {
                word += ch;
                ++offset;
                if (offset < len) {
                    ch = context[offset];
                } else {
                    error("error", lines);//
                }
            } while (isalpha(ch) || ch == '_' || isdigit(ch));
            //不更新offset  添加token
            if (keywords.find(word) != keywords.end()) {//识别为关键字
                tokens.push_back(new Token_Keyword(word, lines));
            } else {//识别为变量名
                tokens.push_back(new Token_Identifier(word, lines));
            }
            continue;
        }
        //TODO 当结束字符为 123. (在浮点数解析解决了这个问题)
        if (ch == '.') {
            tokens.push_back(new Token(token_eof, lines));
            return tokens;
        } else {//识别其他的字符 防止死循环
            error("offset error", lines);
        }
    }
    if (tokens.at(tokens.size() - 1)->getTag() != token_eof) {
        error("parse error", tokens.at(tokens.size() - 1)->getLines());
    }
    return tokens;
}

//解析每一行token
int paser(const vector<Token *> &tokens) {
    int offset = 0, len = tokens.size();
    string stmt_value;
    bool stmt_value_isInt;
    Token *t = nullptr, *t1 = nullptr, *t2 = nullptr, *t3 = nullptr;
    map<string, Vartable *>::iterator search;
    while (offset < len) {
        if (offset >= len - 1) {
            cout << "end ......" << endl;
            return 0;
        }
        t = tokens.at(offset);
        if (t->getTag() == token_eof) {
            offset++;
            continue;
        }
        if (t->getTag() == token_keyword) {//关键字
            if (t->getValue() == "write") {// write(stmt);
                offset++;// 指向write的(  防止出现  最右侧） 干扰匹配
                if (offset >= len - 1 || tokens.at(offset)->getTag() == token_eof) {
                    error("Function write lack '(', you need add it ", t->getLines());
                }
                cout << stmt(tokens, offset) << endl;
            } else if (t->getValue() == "int" || t->getValue() == "float") { //int id = (stmt);
                offset++;// 变量名
                if (offset >= len - 1 || tokens.at(offset)->getTag() == token_eof) {
                    error("Affirmative statement lack a identifier, like int a or float;", t->getLines());
                }
                t1 = tokens[offset];//变量token (a)
                if (t1->getTag() != token_identifier) {
                    error("The Token must be a identifier", t1->getLines());
                }
                search = vartable.find(t1->getValue());
                if (search != vartable.end()) {
                    error("Variable " + t1->getValue() + " twices", t1->getLines());
                }
                offset++;// =
                if (offset >= len - 1) {
                    //此处就不插入这个变量了
                    cout
                            << "Parsing is complete, but it is not recommended to declare the initialized variable at the last of the line.";
                    continue;
                }
                t2 = tokens.at(offset);
                if (t2->getTag() == token_eof) { //int id; 形如
                    Token *temp;
                    if (t->getValue() == "int") {
                        temp = new Token_Int("0", t1->getLines());
                    } else {
                        temp = new Token_Float("0.0", t1->getLines());
                    }

                    vartable.insert(pair<string, Vartable *>(t1->getValue(), new Vartable(temp, false)));
                } else if (stoi(t2->getValue()) != EQ) { // 形如 int id = stmt()
                    if (offset == len - 1) { error("May be you need a '='", t2->getLines()); }
                } else {//TODO
                    offset++;
                    if (offset >= len - 1 || tokens.at(offset)->getTag() == token_eof) {
                        error("error such as: int a =  ; you need a statement expression to complete it , like int c = a+b;",
                              t->getLines());
                    }
                    // 判断返回值的类型
                    stmt_value_isInt = (t->getValue() == "float");
                    stmt_value = stmt(tokens, offset, stmt_value_isInt);

                    if (stmt_value_isInt) {
                        vartable.insert(
                                pair<string, Vartable *>(t1->getValue(),
                                                         new Vartable(new Token_Float(stmt_value), true)));
                    } else {
                        vartable.insert(
                                pair<string, Vartable *>(t1->getValue(),
                                                         new Vartable(new Token_Int(stmt_value), true)));
                    }
                }
            }
        } else if (t->getTag() == token_identifier) {//关键字   id = stmt;
            search = vartable.find(t->getValue());
            if (search == vartable.end()) {
                error("Variable " + t->getValue() + " not defined", t->getLines());
            }
            offset++;
            if (offset == len - 1 || tokens.at(offset)->getTag() == token_eof) {
                error("May be you need a '='", t->getLines());
            }
            t1 = tokens.at(offset);// =
            string my = t1->getValue();
            int tem = stoi(my);
            if (stoi(t1->getValue()) != EQ) {// EQ = 61
                error("The character must be a '='", t1->getLines());
            }
            offset++;// stmt()
            if (offset == len - 1 || tokens.at(offset)->getTag() == token_eof) {
                error("error such as: a =  ; you need a statement expression to complete it , like c = a+b;",
                      t->getLines());
            }
            stmt_value_isInt = (search->second->token->getTag() == token_float);
            stmt_value = stmt(tokens, offset, stmt_value_isInt);
            if (stmt_value_isInt) {
                search->second->token = new Token_Float(stmt_value);
            } else {
                search->second->token = new Token_Int(stmt_value);
            }
            search->second->isinit = true;
        }
    }
    return 1;
}

// 还用控制着偏移量   stmt结束时 指向本轮的token的下标
string stmt(const vector<Token *> &subtokes, int &offset, bool return_type) {//赋值的类型
    Token *token = subtokes.at(offset);
    string clac = "";
    char ch = 0;
    int len = subtokes.size();
    bool isInt = true;// 确保类型转换 int和double
    map<string, Vartable *>::iterator search;
    // 参与运算
    while (true) {
        if (token->getTag() == token_eof || offset == len - 1) {
            if (clac == "") {
                error("检察原代码");
            }
            clac = fushuproblem(clac);
            string exp = exp_to_postexp(clac);
            if (isInt) {
//                if (return_type) {
//                    error("int to float! can not convert", token->getLines());
//                }
                int value;
                calculate_postexp(exp, value); //TODO
                return to_string(value);
            } else {
                double value;
                calculate_postexp(exp, value); //TODO
                if (!return_type)//int
                    return (to_string((int) value));
                return to_string(value);
            }

        }
        if (token->getTag() == token_identifier) {
            search = vartable.find(token->getValue());
            if (search == vartable.end()) {
                error("Variable " + token->getValue() + " is not defined", token->getLines());
            }
            if (search->second->isinit == false) {
                error("Variable " + token->getValue() + " is not initial", token->getLines());
            }
            if (search->second->token->getTag() == token_float) {
                isInt = false;
            }
            clac += search->second->token->getValue();
        } else if (token->getTag() == token_operator) {// 括号等操作符
            ch = stoi(token->getValue());
            switch (ch) {
                case PLUS:
                    clac += '+';
                    break;
                case SUB:
                    clac += '-';
                    break;
                case MUL:
                    clac += '*';
                    break;
                case DIV:
                    clac += '/';
                    break;
                case LHS:
                    clac += '(';
                    break;
                case RHS:
                    clac += ')';
                    break;
                default:
                    error("出现莫名其妙的符号", token->getLines());
                    break;
            }
        } else if (token->getTag() == token_int) {
            clac += token->getValue();
        } else if (token->getTag() == token_float) {
            clac += token->getValue();
            isInt = false;
        } else {
            error("解析出现严重错误", token->getLines());
        }

        offset++;
        if (offset == len - 1 && subtokes.at(offset)->getTag() != token_eof) {
            error("缺少结束符", token->getLines());
            continue;
        }
        token = subtokes.at(offset);
    }
}


template<typename T>
// 模板动态控制类型
void calculate_postexp(string &postexp, T &result) {//方便指定模板的类型
    string::iterator it = postexp.begin();
    stack<T> s;
    T a, b, sum = 0;
    int i = 0;
    while (it != postexp.end()) {
        switch (*it) {
            case '+':
                a = s.top();
                s.pop();
                b = s.top();
                s.pop();
                s.push(b + a);
                break;
            case '-':
                a = s.top();
                s.pop();
                b = s.top();
                s.pop();
                s.push(b - a);
                break;
            case '*':
                a = s.top();
                s.pop();
                b = s.top();
                s.pop();
                s.push(b * a);
                break;
            case '/':
                a = s.top();
                s.pop();
                b = s.top();
                s.pop();
                if (a == 0) {
                    result = 0;// div 0  0作为除数
                    return;
                }
                s.push(b / a);
                break;
            default:
                sum = 0;
                string value = "";
                bool isInt = true;
                while ((*it >= '0' && *it <= '9') || *it == '.') {
//                    sum = 10 * sum + int(*it - '0');
                    value += *it;
                    if (*it == '.') { isInt = false; }// 动态确定double还是int
                    it++;
                }
                sum = isInt ? atoi(value.c_str()) : atof(value.c_str());
                s.push(sum);
        }
        it++;
    }
    result = s.top();
}

// 中缀表达式转化为后缀表达式
string exp_to_postexp(string &exp) {
    stack<char> mystack;//存运算符包括()的栈 不会有数字出现
    string postexp = "";//存放数字和符号及运算符
    char ch = '\n';
    string::iterator it = exp.begin();
    while (it != exp.end()) {
        switch (*it) {
            //判断为'('则将其入栈
            case '(':
                mystack.push(*it);
                it++;
                break;
                //将'('之前的所有的字符添加到postexp里面 将'('出栈
            case ')':
                while ((ch = mystack.top()) != '(') {
                    postexp += ch;
                    mystack.pop();
                }
                mystack.pop();
                it++;
                break;
                //+ 出栈运算符到postexp 直到(或栈空
            case '+':
                while (!mystack.empty() && (ch = mystack.top()) != '(') {
                    postexp += ch;
                    mystack.pop();
                }
                mystack.push(*it);
                it++;
                break;
                //将符号入栈
            case '-':
                while (!mystack.empty() && (ch = mystack.top()) != '(') {
                    postexp += ch;
                    mystack.pop();
                }
                mystack.push(*it);
                it++;
                break;
            case '*':
                while (!mystack.empty() && mystack.top() != '(' && !mystack.top() != '+' && mystack.top() != '-') {
                    postexp += mystack.top();
                    mystack.pop();
                }
                mystack.push(*it);
                it++;
                break;
            case '/':
                //此时栈可能为空
                //ch = mystack.top();
                while (!mystack.empty() && mystack.top() != '(' && !mystack.top() != '+' && mystack.top() != '-') {
                    postexp += mystack.top();
                    mystack.pop();

                }
                mystack.push(*it);
                it++;
                break;
                //判断为数字
                //空格直接忽略
            case ' ':
                it++;
                break;
            default:
                //数字的边界导致字符溢出  包含字符类型
                while ((*it <= '9' && *it >= '0') || *it == '.') {
                    postexp += *it;
                    it++;
                    if (it == exp.end()) {
                        break;
                    }
                }//当这个*it为最后一个的时候 再次加就会出错
                postexp += '#';
                break;
        }
    }
    while (!mystack.empty()) {
        postexp += mystack.top();
        mystack.pop();
    }
    return postexp;
}

string fushuproblem(const string &exp) {
    vector<int> flag;
    vector<int> LHS;//左括号
    vector<int> RHS;//右括号
    string result = exp;
    for (int i = 0; i < exp.length(); i++) {
        if (exp[i] == '-') {
            flag.push_back(i);
        }
    }
    int current, pre, last;
    bool isjudge;
    for (int i = 0; i < flag.size(); i++) {
        current = flag[i];
        isjudge = false;
        if (current == 0) {
            LHS.push_back(0);
            last = 1;
            while (last < exp.length()) {
                if (isdigit(exp[last]) || exp[last] == '.') {
                    last++;
                } else {
                    break;
                }
            }
            RHS.push_back(last);
        } else if (current == flag.size() - 1) {
            cout << "- is in the last\n";
        } else {
            if (!isdigit(exp[current + 1]))//判断为减号  绝不可能为负号
                continue;
            pre = current - 1;
            while (pre >= 0) {
                char ch = exp[pre];
                if (ch == ' ' || ch == '\t') {
                    pre--;
                    continue;
                } else if (ch == '*' || ch == '/' || ch == '+' || ch == '-' || ch == '(') {//判断为负号
                    LHS.push_back(pre + 1);
                    last = current + 1;
                    while (last < exp.length()) {
                        if (isdigit(exp[last]) || exp[last] == '.') {
                            last++;
                        } else {
                            break;
                        }
                    }
                    RHS.push_back(last);
                    break;
                } else {
                    break;
                }
            }
        }
    }
    for (int i = 0; i < LHS.size(); i++) {
        result.insert(LHS[i] + 3 * i, "(0");
        result.insert(RHS[i] + 3 * i + 2, ")");
    }
    return result;
}

string readFileIntoString(const char *filename) {
    ifstream ifile(filename);
    ostringstream buf;
    char ch;
    while (buf && ifile.get(ch)) {
        buf.put(ch);
    }
    return buf.str();
}

void deleteHeap(vector<Token *> &tokens) {
    for (int i = 0; i < tokens.size(); ++i) {
        delete tokens[i];
    }
    for (auto iter = vartable.begin(); iter != vartable.end(); iter++) {
        delete iter->second;
    }
    tokens.clear();
    vartable.clear();
}
