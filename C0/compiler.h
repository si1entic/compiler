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

const int KEYWORDS_NUM = 14;// �����ֵĸ���

enum symbol {
    CASE, CHAR, CONST, DEFAULT, ELSE, IF, INT, MAIN, PRINTF, RETURN, SCANF, SWITCH, VOID, WHILE,  // ������
    IDENT, NUMBER, CHARA, STRING, END,                   // ��ʶ��,����,�ַ�,�ַ���,�ļ�������
    PLUS, MINUS, MULT, DIVI, BECOME,                     // + - * / =
    EQL, NEQ, LSS, LEQ, GTR, GEQ,                        // == != < <= > >=
    LPAREN, RPAREN, LBRACK, RBRACK, LBRACE, RBRACE,      // �� ) [ ] { }
    COMMA, COLON, SEMICOLON                              // , : ; 
};
extern string keywords[KEYWORDS_NUM];

/* ���ű� */
enum kinds { CONSTANT, VARIABLE, FUNCTION };
extern string kinds_string[3];
enum types { INTTP, CHARTP, ARRAYTP, VOIDTP };
extern string types_string[4];
struct tab_item {   // tab����
    string name;    // ��ʶ���������IDENT_LEN���ַ����ȣ�
    kinds kind;     // ��ʶ�����ࣺ����������������
    types type;     // ��ʶ�����ͣ����͡��ַ��͡����顢�޷���ֵ����
    int ref;        // ����ʶ������Ϊ����ʱ����¼����atab�е�λ�ã�Ϊ����ʱ����¼����ftab�е�λ��
    int block;      // �������飬0Ϊȫ�֣�����Ϊ���ֲ����
    int adr;        // ������ֵ �� ������S�е���Ե�ַ 
};
extern vector<tab_item> tab;
extern size_t pt_block;   // ������ָ��
/* ������Ϣ�� */
struct atab_item {
    types eltype;   // Ԫ�ص�����
    int length;     // ����ĳ���
};
extern vector<atab_item> atab;
/* ������Ϣ�� */
struct ftab_item {
    int lastpara;   // �������һ�������ڷ��ű��е�λ��
    int size;       // �������ֲ�����������ջ����ռ�Ĵ洢��Ԫ����
    int tsize;      // ��ʱ����($)��ռ��С
};
extern vector<ftab_item> ftab;
/* �ַ����� */
extern vector<string> stab;

/* ��Ԫʽ���� */
struct quad_item {
    string name;
    types type;
};
struct quadruple {
    quad_item a, b, c, d, e, f;
};
extern vector<quadruple> quad_code;
extern size_t pt_r;            // �м����ָ��
extern size_t pt_label;

extern size_t pc;
//extern string t[] = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9" };
//extern int pt_t;

extern size_t offset;         // ��������/������ƫ�Ƶ�ַ

extern char ch;	    // ���������Դ�����ַ�
extern int value;	    // ����int��char��ֵ
extern string str;     // �����ַ�����ֵ
extern symbol sym;     // ���ʶ��ķ�������
extern string id;      // �����ʶ������
extern ifstream fin;
extern ofstream fout;

void error(int no); // ���������Ϣ���˳�����
void printTabs();       // ������ű���Ϣ
void getch();       // ��ȡԴ�����һ���ַ�
void getsymbol();   // ��ȡһ������
string genLabel(int i);   // ���ɱ�ǩ
void genQuad(string a, string b, string c, string d, string e, string f);    // ������Ԫʽ
void enter(kinds kind, types type, int block);  // ��¼���ű�
void enterArray(types eltype, int block);       // ��¼���鵽���ű�
int findTab(string name);      // ���ر�ʶ���ڷ��ű��е�λ��
void constDefi(types ic, int block);    // ����һ����������
void constsDefi(int block);             // ����һ�г�������
void varDefi(types ic, int block);      // ����һ����������
void varsDefi(types ic, int block);     // ����һ�г�������
void parameter(int block);      // ����һ������
void parameters(int block);     // ����<������>
quad_item call(int position);    // �������������
quad_item factor();      // ����<����>
quad_item term();        // ����<��>
quad_item expression();  // ����<���ʽ>
void condition();       // ����<����>
void statement();           // ����<���>
void comState(int block);   // ����<�������>
void funDefi(types tp); // ����һ����������
void fun_main();        // ��������������
void program();    // ����<����>
void optimize();    // �Ż���Ԫʽ
void transform();   // ��ԪʽתMIPS