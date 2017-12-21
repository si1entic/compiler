#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
using namespace std;

const int KEYWORDS_NUM = 14;// 保留字的个数

enum symbol {
    CASE, CHAR, CONST, DEFAULT, ELSE, IF, INT, MAIN, PRINTF, RETURN, SCANF, SWITCH, VOID, WHILE,  // 保留字
    IDENT, NUMBER, CHARA, STRING, END,                   // 标识符,数字,字符,字符串,文件结束符
    PLUS, MINUS, MULT, DIVI, BECOME,                     // + - * / =
    EQL, NEQ, LSS, LEQ, GTR, GEQ,                        // == != < <= > >=
    LPAREN, RPAREN, LBRACK, RBRACK, LBRACE, RBRACE,      // （ ) [ ] { }
    COMMA, COLON, SEMICOLON                              // , : ; 
};
extern string keywords[KEYWORDS_NUM];

/* 符号表 */
enum kinds { CONSTANT, VARIABLE, FUNCTION };
extern string kinds_string[3];
enum types { INTTP, CHARTP, ARRAYTP, VOIDTP };
extern string types_string[4];
struct tab_item {   // tab的项
    string name;    // 标识符名（最多IDENT_LEN个字符长度）
    kinds kind;     // 标识符种类：常量、变量、函数
    types type;     // 标识符类型：整型、字符型、数组、无返回值函数
    int ref;        // 当标识符类型为数组时，记录其在atab中的位置；为函数时，记录其在ftab中的位置
    int block;      // 所属区块，0为全局，其余为各局部编号
    int adr;        // 常量的值 或 变量在S中的相对地址 
};
extern vector<tab_item> tab;
extern size_t pt_block;   // 区块标记指针
/* 数组信息表 */
struct atab_item {
    types eltype;   // 元素的类型
    int length;     // 数组的长度
};
extern vector<atab_item> atab;
/* 函数信息表 */
struct ftab_item {
    int lastpara;   // 函数最后一个参数在符号表中的位置
    int size;       // 参数、局部变量在运行栈中所占的存储单元总数
    int tsize;      // 临时变量($)所占大小
};
extern vector<ftab_item> ftab;
/* 字符串表 */
extern vector<string> stab;

/* 四元式代码 */
struct quad_item {
    string name;
    types type;
};
struct quadruple {
    quad_item a, b, c, d, e, f;
};
extern vector<quadruple> quad_code;
extern size_t pt_r;            // 中间变量指针
extern size_t pt_label;

extern size_t pc;
//extern string t[] = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9" };
//extern int pt_t;

extern size_t offset;         // 函数参数/变量的偏移地址

extern char ch;	    // 最近读出的源代码字符
extern int value;	    // 保存int或char的值
extern string str;     // 保存字符串的值
extern symbol sym;     // 最近识别的符号类型
extern string id;      // 保存标识符名字
extern ifstream fin;
extern ofstream fout;

void error(int no); // 输出错误信息，退出程序
void printTabs();       // 输出符号表信息
void getch();       // 读取源代码的一个字符
void getsymbol();   // 读取一个单词
string genLabel(int i);   // 生成标签
void genQuad(string a, string b, string c, string d, string e, string f);    // 生成四元式
void enter(kinds kind, types type, int block);  // 登录符号表
void enterArray(types eltype, int block);       // 登录数组到符号表
int findTab(string name);      // 返回标识符在符号表中的位置
void constDefi(types ic, int block);    // 处理一个常量定义
void constsDefi(int block);             // 处理一行常量定义
void varDefi(types ic, int block);      // 处理一个变量定义
void varsDefi(types ic, int block);     // 处理一行常量定义
void parameter(int block);      // 处理一个参数
void parameters(int block);     // 处理<参数表>
quad_item call(int position);    // 处理函数调用语句
quad_item factor();      // 处理<因子>
quad_item term();        // 处理<项>
quad_item expression();  // 处理<表达式>
void condition();       // 处理<条件>
void statement();           // 处理<语句>
void comState(int block);   // 处理<复合语句>
void funDefi(types tp); // 处理一个函数定义
void fun_main();        // 处理主函数定义
void program();    // 处理<程序>
void optimize();    // 优化四元式
void transform();   // 四元式转MIPS