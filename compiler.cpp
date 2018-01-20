#include "compiler.h"
using namespace std;
string keywords[KEYWORDS_NUM] = { "case","char","const","default","else","if","int","main","printf","return","scanf","switch","void","while" };
string kinds_string[3] = { "CONSTANT", "VARIABLE", "FUNCTION" };
string types_string[4] = { "INT","CHAR","ARRAY","VOID" };
vector<tab_item> tab;
vector<atab_item> atab;
vector<ftab_item> ftab;
vector<string> stab;
vector<quadruple> quad_code;
size_t pt_T, pt_block, pt_label, pc, offset;
char ch;
int value;
string str;
symbol sym;
string id;
ifstream fin;
ofstream fout;
int prerow = 1; // 上一单词所在行数
int currow = 1; // 当前单词所在行数
int nexrow = 1; // 下一单词所在行数
bool isError = false;
vector<map<string, string>> registers(1);

void error(int id) {
    string msg[100];
    msg[1] = "超过int上限"; msg[2] = "缺少'"; msg[3] = "字符串内容出错"; msg[4] = "!只能与=连用";
    msg[5] = "无法识别符号"; msg[6] = "标识符重复定义"; msg[7] = "缺少(或;"; msg[8] = "void函数不能有返回值或跟()";
    msg[9] = "函数缺少返回值（返回值应在()内）"; msg[10] = "缺少标识符"; msg[11] = "缺少="; msg[12] = "缺少int或char";
    msg[13] = "数组下标越界"; msg[14] = "常量定义类型出错"; msg[15] = "缺少;"; msg[16] = "缺少无符号整数";
    msg[17] = "缺少]"; msg[18] = "无参数的函数调用不能跟()"; msg[19] = "传入参数过多"; msg[20] = "传入参数过少"; 
    msg[21] = "缺少)"; msg[22] = "标识符未定义"; msg[23] = "数组变量缺少下标"; msg[24] = "调用的函数无返回值";
    msg[25] = "因子类型出错"; msg[26] = "缺少("; msg[27] = "缺少else"; msg[28] = "标识符非变量"; 
    msg[29] = "缺少case"; msg[30] = "case后缺少常量"; msg[31] = "case标号重复定义"; msg[32] = "缺少:";
    msg[33] = "缺少["; msg[34] = "不能对常量进行赋值"; msg[35] = "缺少}"; msg[36] = "无法识别语句"; 
    msg[37] = "变量定义出错"; msg[38] = "缺少{"; msg[39] = "函数定义出错"; msg[40] = "main函数类型必须为void";
    msg[41] = "缺少main函数"; msg[42] = "main函数后有多余字符"; msg[43] = "case常量类型错误"; msg[44] = "标识符定义出错";
    msg[45] = "非0整数不能以0开头"; msg[46] = "无参数函数定义时不允许加()"; msg[47] = "<整数>0前不允许有正负号";
    fout << endl << "第" << prerow << "行：" << msg[id];
    isError = true;
    /*fin.close();
    fout.close();
    exit(id);*/
}

void getch() {
    if (fin.peek() != EOF) {
        fin >> ch;
        // fout << ch;
    }
    else
        ch = NULL;
}
void getsymbol() {
    if (ch == NULL) {
        sym = END;
        return;
    }
    if (ch == '_' || isalpha(ch)) { // 对应文法中的<字母>
        id = "";     // 重置标识符名
        do {
            id += ch;
            getch();
        } while (ch == '_' || isalnum(ch));
        transform(id.begin(), id.end(), id.begin(), tolower);   // 将string转为全小写
                                                                /* 检测当前id是否为保留字，二分查找 */
        int l = 0, r = KEYWORDS_NUM - 1, m = 0;
        while (l <= r) {
            m = (l + r) / 2;
            if (id < keywords[m])
                r = m - 1;
            else if (id > keywords[m])
                l = m + 1;
            else
                break;
        }
        if (l > r) // 没找到
            sym = IDENT;        // 当前符号为标识符
        else
            sym = symbol(m);    // 根据int值强制转换为枚举值
    }
    else if (isdigit(ch)) {   // 对应不带+/-的<整数>
        value = 0;      // 重置int值
        sym = NUMBER;   // 符号类型为number
        string s = "";
        do {
            s += ch;
            try {
                value = stoi(s);
            }
            catch (const std::exception&) {
                error(1);
            }
            getch();
        } while (isdigit(ch));
        if (s[0] == '0'&&value != 0)    // 非零数字不得以0开头
            error(45);
    }
    else if (ch == '\'') {  // 对应文法中的＜字符＞  
        getch();
        if (isalnum(ch) || ch == '_' || ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            value = ch;
            getch();
            if (ch != '\'')
                error(2);
            sym = CHARA;
            getch();
        }
    }
    else if (ch == '\"') {  // 对应文法中的<字符串>
        str = "";   // 重置字符串值
        getch();
        while (ch != '\"') {
            if (ch == 32 || ch == 33 || (ch >= 35 && ch <= 126))
                str += ch;
            else
                error(3);
            getch();
        }
        sym = STRING;
        getch();
    }
    else if (ch == '=') {
        getch();
        if (ch == '=') {
            sym = EQL;
            getch();
        }
        else
            sym = BECOME;
    }
    else if (ch == '<') {
        getch();
        if (ch == '=') {
            sym = LEQ;
            getch();
        }
        else
            sym = LSS;
    }
    else if (ch == '>') {
        getch();
        if (ch == '=') {
            sym = GEQ;
            getch();
        }
        else
            sym = GTR;
    }
    else if (ch == '!') {
        getch();
        if (ch == '=') {
            sym = NEQ;
            getch();
        }
        else {
            error(4);
            sym = NEQ;
        }
    }
    else {
        switch (ch) {
        case '+':
            sym = PLUS;
            break;
        case '-':
            sym = MINUS;
            break;
        case '*':
            sym = MULT;
            break;
        case '/':
            sym = DIVI;
            break;
        case '(':
            sym = LPAREN;
            break;
        case ')':
            sym = RPAREN;
            break;
        case '[':
            sym = LBRACK;
            break;
        case ']':
            sym = RBRACK;
            break;
        case '{':
            sym = LBRACE;
            break;
        case '}':
            sym = RBRACE;
            break;
        case ',':
            sym = COMMA;
            break;
        case ':':
            sym = COLON;
            break;
        case ';':
            sym = SEMICOLON;
            break;
        default:
            error(5);
        }
        getch();
    }

    prerow = currow;
    currow = nexrow;
    while (ch<0||isspace(ch)) {   // 判断预读的是否为空白字符
        if (ch < 0) {
            isError = true;
            fout << endl << "第" << nexrow << "行：存在非法字符";
        }
        if (ch == '\n')
            nexrow++;
        getch();
    }
}

string genLabel(int i) {
    stringstream ss;
    ss << ".label" << i;    // 前加.防止与函数重名
    return ss.str();
}

void genQuad(string a = "", string b = "", string c = "", string d = "", string e = "", string f = "") {
    quadruple nq;
    nq.a.name = a;
    nq.b.name = b;
    nq.c.name = c;
    nq.d.name = d;
    nq.e.name = e;
    nq.f.name = f;
    quad_code.push_back(nq);
}

void enter(kinds kind, types type, int block) {   // 登录常量或非数组的变量
    for (int i = tab.size()-1; i >= 0; i--) {
        if (tab[i].block < block)
            break;
        if (tab[i].block == block&&tab[i].name == id) {
            error(6);
            return;
        }
    }
    tab_item nt;
    nt.name = id;
    nt.kind = kind;
    nt.type = type;
    nt.block = block;
    switch (kind) {
    case CONSTANT:  // 对于常量，只需将value赋给adr
        nt.adr = value;
        break;
    case VARIABLE:  // 对于非数组变量，需要登录当前层的相对地址
        nt.adr = ++offset;       
        break;
    case FUNCTION:  // 对于函数，登录其函数信息表位置和入口地址
        nt.ref = ftab.size();  // 记录在ftab中的位置
        ftab_item nf;
        ftab.push_back(nf);
        break;
    }
    tab.push_back(nt);
}
void enterArray(types eltype, int block) {  // 登录数组变量
    tab_item nt;
    nt.adr = ++offset;
    nt.name = id;
    nt.kind = VARIABLE;
    nt.type = ARRAYTP;
    nt.block = block;
    nt.ref = atab.size();  // atab指向新的一项，并记录在tab中
    tab.push_back(nt);
    atab_item na;
    na.length = value;   // 记录数组长度
    na.eltype = eltype;  // 记录元素类型
    atab.push_back(na);
    offset += value - 1;
}
int findTab(string name) { // 返回name在tab中的位置,没有则返-1
    for (int i = tab.size()-1; i >= 0; i--) {
        if (tab[i].block != pt_block)
            if (tab[i].block != 0)
                continue;
        if (tab[i].name == name)
            return i;
    }
    return -1;
}

void constDefi(types ic, int block) {    // 处理一个常量定义    sym进来时为IDENT，出去时为常量后第一个符号
    if (sym != IDENT) {
        error(10);
        return;
    }
    getsymbol();
    if (sym != BECOME){
        error(11);
        return;
    }
    getsymbol();
    if (ic == INTTP && (sym == PLUS || sym == MINUS)) {
        bool n = (sym == MINUS) ? true : false;
        if (ch == '0')
            error(47);
        getsymbol();
        if (sym != NUMBER)
            error(16);
        if (n)
            value = -value;
    }
    if ((ic == INTTP&&sym != NUMBER) || (ic == CHARTP&&sym != CHARA)) {
        error(14);
        return;
    }
    enter(CONSTANT, ic, block);  // 常量进表
    getsymbol();
}
void constsDefi(int block) {      // 处理<常量说明>   sym进时为CONST，出时为分号后第一个符号
    getsymbol();
    types ic;
    if (sym != INT&&sym != CHAR) {
        error(14);
        ic = INTTP;
    }
    else
        ic = (sym == INT) ? INTTP : CHARTP;    // 记录当前常量类型为int还是char
    getsymbol();
    constDefi(ic, block);   // 处理一个常量定义
    while (sym == COMMA) {  // 还有同类常量定义
        getsymbol();
        constDefi(ic, block);
    }
    if (sym != SEMICOLON)
        error(15);
    else
        getsymbol();
    if (sym == CONST)
        constsDefi(block);  // 递归调用，继续处理【const<常量定义>;】
}

void varDefi(types ic, int block) {     // 处理一个变量定义  sym进时为IDENT后第一个符号，出时为变量名后的第一个符号
    if (sym == LBRACK) {    // 处理数组
        getsymbol();
        if (sym != NUMBER) {
            error(16);
            return;
        }
        getsymbol();
        if (sym != RBRACK) {
            error(17);
            enterArray(ic, block);   // 数组进表
        }
        else {
            enterArray(ic, block);   // 数组进表
            getsymbol();
        }
    }
    else {
        enter(VARIABLE, ic, block);      // 变量进表
    }
}
void varsDefi(types ic, int block) {      // 处理一行变量定义    sym进时为IDENT后第一个符号，出时为分号后第一个符号
    varDefi(ic, block);
    while (sym == COMMA) {
        getsymbol();
        if (sym != IDENT)
            error(10);
        getsymbol();
        varDefi(ic, block);
    }
    if (sym != SEMICOLON)
        error(15);
    else
        getsymbol();
}

void parameter(int block) {  // 处理一个参数   sym进时为(后第一个符号，出时为IDENT后第一个符号
    types tp;
    if (sym == INT) {
        tp = INTTP;
    }
    else if (sym == CHAR) {
        tp = CHARTP;
    }
    else {
        error(12);
        return;
    }
    getsymbol();
    if (sym != IDENT) {
        error(10);
        return;
    }
    enter(VARIABLE, tp, block); // 参数进tab
    getsymbol();
}
void parameters(int block) {  // 处理<参数表>    sym进时为(，出时为)后第一个符号
    getsymbol();
    if (sym == RPAREN) {
        error(46);
    }
    else {
        parameter(block);
        while (sym == COMMA) {
            getsymbol();
            parameter(block);
        }
    }
    ftab[ftab.size() - 1].lastpara = tab.size() - 1;    //  记录该函数最后一个参数在符号表中的位置
    if (sym != RPAREN)
        error(21);
    else
        getsymbol();
}

quad_item call(int position) {   // 处理函数调用语句，参数为函数名在tab表中的位置
    quad_item x;
    stringstream ss;
    int block = tab[position].block;                // 当前函数区块编号
    int last = ftab[tab[position].ref].lastpara;    // 最后一个参数在符号表中的位置
    int index = position;   // 当前已读到的符号表中的项
    if (sym == LPAREN) {    // 有实参传入
        vector<quad_item> paras;    // 保存PUSH 参数的中间代码，读完所有参数后在call 函数前一起输出（避免嵌套PUSH出错）
        if (index == last)
            error(18);
        do {
            getsymbol();
            if (++index > last)
                error(19);
            x = expression();   // 获取表达式值
            paras.push_back(x);     
            // 检测值的类型，并生成加载指令
        } while (sym==COMMA);
        if (index < last)
            error(20);
        for (size_t i = 0; i < paras.size(); i++) {
            genQuad("PUSH", paras[i].name);
        }
        if (sym != RPAREN)
            error(21);
        else
            getsymbol();
    }
    else {  // 无实参传入
        if (index != last)
            error(20);
    }
    // 实参加载完毕，生成函数调用指令
    genQuad("CALL", tab[position].name);
    if (tab[position].type != VOIDTP) {
        ss << "T" << (++pt_T);
        x.name = ss.str();
        x.type = tab[position].type;
        genQuad(x.name, "=", "RET");
    }
    return x;
}

quad_item factor() {	// 处理<因子>
    stringstream ss;
    quad_item x,y;
    if (sym == IDENT) {
        int position = findTab(id);
        if (position == -1) {
            error(22);
            exit(1);
        }
        tab_item item = tab[position];
        getsymbol();
        switch (item.kind) {
        case CONSTANT:
            // 返回常量值item.adr
            ss << item.adr;
            x.name = ss.str();
            x.type = item.type;
            break;
        case VARIABLE:
            // 取item.adr
            if (item.type == ARRAYTP) {    // 该变量为数组
                if (sym != LBRACK) {
                    error(23);
                    exit(1);
                }
                getsymbol();
                y = expression();   // 返回数组下标
                // 判断下标范围
                if (y.name[0] == '-' || isdigit(y.name[0])) {
                    int index = stoi(y.name);
                    if (index < 0 || index >= atab[tab[position].ref].length)
                        error(13);
                }
                if (sym != RBRACK)
                    error(17);
                else
                    getsymbol();
                // 根据数组首地址item.adr和下标值找到该元素的地址，并取值到栈
                ss << "T" << (++pt_T);
                x.name = ss.str();
                x.type = atab[item.ref].eltype;
                genQuad(x.name, "=", item.name, "[", y.name, "]");
            }
            else {
                // 根据变量地址item.adr取值到栈
                x.name = item.name;
                x.type = item.type;
            }
            break;
        case FUNCTION:
            if (item.type == VOIDTP)
                error(24);
            x = call(position); // 函数调用
            break;
        }
    }
    else if (sym == PLUS || sym == MINUS) {
        int k;
        k = (sym == PLUS) ? 1 : -1;
        if (ch == '0')
            error(47);
        getsymbol();
        if (sym != NUMBER)
            error(25);
        ss << k*value;
        x.name = ss.str();
        x.type = INTTP;
        getsymbol();
    }
    else if (sym == NUMBER || sym == CHARA) {
        ss << value;
        x.name = ss.str();
        if (sym == CHARA)
            x.type = CHARTP;
        else
            x.type = INTTP;
        getsymbol();
    }
    else if (sym == LPAREN) {
        getsymbol();
        x = expression();
        if (sym != RPAREN)
            error(21);
        else
            getsymbol();
    }
    else {
        error(25);
        exit(1);
    }
    return x;
}
quad_item term() {	// 处理<项>
    quad_item x, y, z;
    stringstream ss;
    y = factor();
    while (sym == MULT||sym==DIVI) {
        symbol md = sym;    // 记录乘除符号
        getsymbol();
        z = factor();
        if ((y.name[0] == '-' || isdigit(y.name[0])) && (z.name[0] == '-' || isdigit(z.name[0]))) {
            if (md == MULT)
                ss << stoi(y.name) * stoi(z.name);
            else
                ss << stoi(y.name) / stoi(z.name);
            x.name = ss.str();
        }
        else {
            ss << "T" << (++pt_T);
            x.name = ss.str();
            if (md == MULT)
                genQuad(x.name, "=", y.name, "*", z.name);
            else
                genQuad(x.name, "=", y.name, "/", z.name);
        }
        y = x;
        y.type = INTTP;
        ss.str("");     // 重置 相当于.clear

       
    }
    return y;
}
quad_item expression() { // 处理<表达式>
    quad_item x, y, z;
    stringstream ss;
    symbol pm = PLUS;  // 记录加减符号
    if (sym == PLUS || sym == MINUS) {
        pm = sym;
        getsymbol();
    }
    y = term();
    if (pm == MINUS) {
        if (isdigit(y.name[0]))
            y.name = "-" + y.name;
        else if (y.name[0] == '-')
            y.name = y.name.substr(1);
        else {
            // 生成取相反数指令
            ss << "T" << (++pt_T);
            x.name = ss.str();
            ss.str("");     // 重置 相当于.clear
            genQuad(x.name, "=", "0", "-", y.name);
            y = x;
            y.type = INTTP;
        }
    }
    while (sym == PLUS || sym == MINUS) {
        pm = sym; 
        getsymbol();
        z = term();
        if ((y.name[0] == '-' || isdigit(y.name[0])) && (z.name[0] == '-' || isdigit(z.name[0]))) {
            if (pm == PLUS)
                ss << stoi(y.name) + stoi(z.name);
            else
                ss << stoi(y.name) - stoi(z.name);
            x.name = ss.str();
        }
        else {
            ss << "T" << (++pt_T);
            x.name = ss.str();
            if (pm == PLUS)
                genQuad(x.name, "=", y.name, "+", z.name);
            else
                genQuad(x.name, "=", y.name, "-", z.name);
        }
        y = x;
        y.type = INTTP;
        ss.str("");     // 重置 相当于.clear
    }
    return y;
}

void condition() {  //  处理<条件>
    quad_item y, z;
    y = expression();
    if (sym == EQL || sym == NEQ || sym == LSS || sym == LEQ || sym == GTR || sym == GEQ) {
        symbol op = sym;
        getsymbol();
        z = expression();
        switch (op) {
            /*根据关系运算符生成各种指令*/
        case EQL:
            genQuad(y.name, "==", z.name);
            break;
        case NEQ:
            genQuad(y.name, "!=", z.name);
            break;
        case LSS:
            genQuad(y.name, "<", z.name);
            break;
        case LEQ:
            genQuad(y.name, "<=", z.name);
            break;
        case GTR:
            genQuad(y.name, ">", z.name);
            break;
        case GEQ:
            genQuad(y.name, ">=", z.name);
            break;
        }
    }
    else {
        genQuad(y.name, "!=", "0");
    }
}

void statement() {  // 处理<语句>
    quad_item x, y, z;
    vector<int> case_value;   // 各case的常量值
    vector<int> case_label;   // 各case的开始位置
    stringstream ss;
    switch (sym)
    {
    case IF:    // 识别到<条件语句>
        int else_begin,if_end;
        getsymbol();
        if (sym != LPAREN)
            error(26);
        else
            getsymbol();
        condition();
        else_begin = ++pt_label;
        genQuad("BZ", genLabel(else_begin));  // 条件不满足，则跳转至else_begin处
        if (sym != RPAREN)
            error(21);
        else
            getsymbol();
        statement();
        if_end = ++pt_label;
        genQuad("GOTO", genLabel(if_end));    // then执行完毕，跳转至if_end处
        if (sym != ELSE) {
            error(27);
            genQuad(genLabel(else_begin), ":");   // else_begin:
        }
        else {
            getsymbol();
            genQuad(genLabel(else_begin), ":");   // else_begin:
            statement();
        }
        genQuad(genLabel(if_end), ":");       // if_end:
        break;
    case PRINTF:
        getsymbol();
        if (sym != LPAREN)
            error(26);
        else
            getsymbol();
        if (sym == STRING) {
            genQuad("PRINTF", "\""+str+ "\"");
            stab.push_back(str);
            getsymbol();
            if (sym == RPAREN) {
                getsymbol();
            }
            else if (sym == COMMA) {
                getsymbol();
                y = expression();
                genQuad("PRINTF", y.name);
                quad_code[quad_code.size()-1].b.type = y.type;  // 设置四元式项的type，用以区分char和int
                if (sym != RPAREN)
                    error(21);
                else
                    getsymbol();
            }
            else
                error(21);
        }
        else {
            y = expression();
            genQuad("PRINTF", y.name);
            quad_code[quad_code.size() - 1].b.type = y.type;
            if (sym != RPAREN)
                error(21);
            else
                getsymbol();
        }
        if (sym != SEMICOLON)
            error(15);
        else
            getsymbol();
        break;
    case RETURN:
        getsymbol();
        int i;
        for (i = tab.size()-1; tab[i].kind != FUNCTION; i--);
        if (sym == LPAREN) {
            if (tab[i].type == VOIDTP)
                error(8);
            getsymbol();
            y = expression();
            genQuad("RET", y.name);
            if (sym != RPAREN)
                error(21);
            else
                getsymbol();
        }
        else {
            if (tab[i].type != VOIDTP)
                error(9);
            genQuad("RET");
        }
        if (sym != SEMICOLON)
            error(7);
        else
            getsymbol();
        break;
    case SCANF:
        getsymbol();
        if (sym != LPAREN)
            error(26);
        do {
            getsymbol();
            if (sym != IDENT)
                error(10);
            else {
                int position = findTab(id);
                if (position == -1)
                    error(22);
                else {
                    if (tab[position].kind != VARIABLE)
                        error(28);
                    genQuad("SCANF", tab[position].name);
                    quad_code[quad_code.size() - 1].b.type = tab[position].type;  // 设置四元式项的type，用以区分char和int
                }
            }
            getsymbol();
        } while (sym == COMMA);
        if (sym != RPAREN)
            error(21);
        else
            getsymbol();
        if (sym != SEMICOLON)
            error(15);
        else
            getsymbol();
        break;
    case SWITCH:    // 识别到<情况语句>
        int switch_cmp,switch_end,default_begin;   // 进行比较、switch语句结束、default开始的位置
        getsymbol();
        if (sym != LPAREN)
            error(26);
        else
            getsymbol();
        x = expression();
        if (sym != RPAREN)
            error(21);
        else
            getsymbol();
        if (sym != LBRACE)
            error(38);
        else
            getsymbol();
        switch_cmp = ++pt_label;
        switch_end = ++pt_label;
        genQuad("GOTO", genLabel(switch_cmp));       // 中间代码跳转至case_cmp处
        do {
            if (sym != CASE)
                error(29);
            getsymbol();
            if (sym == NUMBER){
                if (x.type != INTTP)
                    error(43);
            }
            else if (sym == PLUS || sym == MINUS) {
                if (ch == '0')
                    error(47); 
                int k = (sym == PLUS) ? 1 : -1;
                getsymbol();
                if (sym != NUMBER || x.type != INTTP)
                    error(43);
                value *= k;
            }
            else if (sym == CHARA) {
                if (x.type != CHARTP)
                    error(43);
            }
            else
                error(30);
            for (size_t i = 0; i < case_value.size(); i++)
                if (value == case_value[i])
                    error(31);
            case_value.push_back(value);       // 记录常量值
            case_label.push_back(++pt_label);  // 记录标签下标
            getsymbol();
            if (sym != COLON)
                error(32);
            else
                getsymbol();
            genQuad(genLabel(case_label[case_label.size()-1]), ":");    // 各case语句起始位置
            statement();
            genQuad("GOTO", genLabel(switch_end));
        } while (sym == CASE);
        if (sym == DEFAULT) {
            getsymbol();
            if (sym != COLON)
                error(32);
            getsymbol();
            default_begin = ++pt_label;
            genQuad(genLabel(default_begin), ":");
            statement();
            genQuad("GOTO", genLabel(switch_end));
        }
        else
            default_begin = switch_end;     // 若源代码无default，则设为switch结束位置
        if (sym != RBRACE)
            error(35);
        else
            getsymbol();
        genQuad(genLabel(switch_cmp), ":");
        for (size_t i = 0; i < case_value.size(); i++) {
            stringstream ss;
            ss << case_value[i];
            genQuad(x.name, "==", ss.str());    
            genQuad("BNZ", genLabel(case_label[i]));   // 满足条件，则跳转至相应位置
        } 
        genQuad("GOTO", genLabel(default_begin));     // 不满足任一条件，则跳转至default处
        genQuad(genLabel(switch_end), ":");           // switch语句结束位置
        break;
    case WHILE:     // 识别到<循环语句>
        int while_head, while_end;
        getsymbol();
        if (sym != LPAREN)
            error(26);
        else
            getsymbol();
        while_head = ++pt_label;
        while_end = ++pt_label;
        genQuad(genLabel(while_head), ":");
        condition();
        if (sym != RPAREN)
            error(21);
        else
            getsymbol();
        genQuad("BZ", genLabel(while_end));
        statement();
        genQuad("GOTO", genLabel(while_head));
        genQuad(genLabel(while_end), ":");
        break;
    case IDENT:   
        int position;
        position = findTab(id);
        if (position == -1) {
            error(22);
            break;
        }
        if (tab[position].kind == FUNCTION) {     // 识别到<有/无返回值函数调用>
            getsymbol();
            call(position);
        }
        else if (tab[position].kind == VARIABLE) {  // 识别到<赋值语句>
            if (tab[position].type == ARRAYTP) {    // 对数组某元素赋值
                getsymbol();
                if (sym != LBRACK) {
                    error(33);
                    y.name = "0";
                }
                else {
                    getsymbol();
                    y = expression();   // 获取数组下标的值
                                        // 判断下标范围
                    if (y.name[0] == '-' || isdigit(y.name[0])) {
                        int index = stoi(y.name);
                        if (index < 0 || index >= atab[tab[position].ref].length)
                            error(13);
                    }
                    if (sym != RBRACK)
                        error(17);
                    getsymbol();
                }
                if (sym != BECOME)
                    error(11);
                getsymbol();
                z = expression();   // 获取赋值号右边的值
                // 生成赋值代码
                genQuad(tab[position].name, "[", y.name, "]", "=", z.name);
            }
            else {      // 对非数组变量赋值
                getsymbol();
                if (sym != BECOME)
                    error(11);
                getsymbol();
                y = expression();    
                // 生成赋值代码
                genQuad(tab[position].name, "=", y.name);
            }
        }
        else {
            error(34); 
            getsymbol();
            if (sym != BECOME)
                error(11);
            getsymbol();
            y = expression();
        }
        if (sym != SEMICOLON)
            error(15);
        else
            getsymbol();
        break;
    case LBRACE:    // 识别到<语句列>
        getsymbol();
        if (sym == RBRACE) {  // 空语句 {}
            getsymbol();
            return;
        }
        while (sym == LBRACE || sym == IF || sym == WHILE || sym == SWITCH || sym == RETURN ||
            sym == SCANF || sym == PRINTF || sym == IDENT || sym == SEMICOLON)
            statement();
        if (sym != RBRACE)
            error(35);
        else
            getsymbol();
        break;
    case SEMICOLON: // 识别到空语句 ;
        getsymbol();
        break;
    default:
        error(36);
        break;
    }
}
void comState(int block) {  // 处理<复合语句>     sym进时为{后第一个符号，出时应为}
    if (sym == CONST)   // 识别到局部常量定义
        constsDefi(block);
    while (sym == INT || sym == CHAR) {
        types ic = (sym == INT) ? INTTP : CHARTP;
        getsymbol();
        if (sym != IDENT) {
            error(10);
        }
        getsymbol();
        if (sym == LBRACK || sym == COMMA || sym == SEMICOLON)  // 识别到[ , ; 说明是局部变量定义
            varsDefi(ic, block);
        else
            error(37);
    }
    ftab[ftab.size()-1].size = 1 + offset;  // 记录该函数的局部数据区(返回地址+函数和局部变量)大小  
    while (sym == LBRACE || sym == IF || sym == WHILE || sym == SWITCH || sym == RETURN ||
        sym == SCANF || sym == PRINTF || sym == IDENT || sym == SEMICOLON)
        statement();
}

void funDefi(types icv) {
    pt_block++;     // 该函数区块编号
    pt_T = -1;      // 重置临时变量名
    string x;
    switch (icv) {
    case INTTP:
        x = "int";
        break;
    case CHARTP:
        x = "char";
        break;
    case VOIDTP:
        x = "void";
        break;
    }
    genQuad(x, id,"(",")");
    if (sym == LPAREN) {    // 有参数
        enter(FUNCTION, icv, 0);
        parameters(pt_block);   // 处理参数
        if (sym != LBRACE)
            error(38);
        else
            getsymbol();
    }
    else if (sym == LBRACE) {  //  无参数
        enter(FUNCTION, icv, 0);
        ftab[ftab.size() - 1].lastpara = tab.size() - 1;
        getsymbol();
    }
    else
        error(38);
    comState(pt_block);   // 处理复合语句
    if (quad_code[quad_code.size() - 1].a.name != "RET")  // 函数最后一句若不是return，则加上
        genQuad("RET");
    if (sym != RBRACE)
        error(35);
    else
        getsymbol();
    ftab[ftab.size() - 1].tsize = pt_T + 1;
}
void fun_main() {
    pt_block++;
    pt_T = -1;      // 重置临时变量名
    genQuad("void", "main", "(", ")");
    enter(FUNCTION, VOIDTP, 0);
    ftab[ftab.size() - 1].lastpara = tab.size() - 1;
    getsymbol();
    if (sym != LPAREN)
        error(26);
    else
        getsymbol();
    if (sym != RPAREN)
        error(21);
    else
        getsymbol();
    if (sym != LBRACE)
        error(38);
    else
        getsymbol();
    comState(pt_block);   // 处理复合语句
    if (sym != RBRACE)
        error(35);
    else
        getsymbol();
    ftab[ftab.size() - 1].tsize = pt_T + 1;
}
void program() {    // 处理<程序>
    if (sym == CONST)   // 识别到全局常量定义
        constsDefi(0);
    offset = 0;
    ftab_item nf;
    ftab.push_back(nf);
    ftab[0].size = 0;
    while (sym == INT || sym == CHAR) {
        types ic = (sym == INT) ? INTTP : CHARTP;
        getsymbol();
        if (sym != IDENT)
            error(10);
        getsymbol();
        if (sym == LBRACK || sym == COMMA || sym == SEMICOLON) {  // 识别到[ , ; 说明是全局变量定义
            varsDefi(ic, 0);
            ftab[0].size = 1 + offset;      // 更新全局数据区(返回地址+全局变量)大小
        }
        else if (sym == LPAREN || sym == LBRACE) {  // 识别到( { 说明是有返回值函数定义
            offset = 0;
            funDefi(ic);
            break;
        }
        else
            error(44);
    }
    do {
        if (sym != INT && sym != CHAR && sym != VOID)
            error(39);
        else {
            types icv = (sym == INT) ? INTTP : (sym == CHAR) ? CHARTP : VOIDTP;
            getsymbol();
            if (sym == MAIN) {
                if (icv != VOIDTP)
                    error(40);
                offset = 0;     // 重置函数数据区偏移量
                fun_main();
                break;
            }
            if (sym != IDENT)
                error(10);
            getsymbol();
            offset = 0;
            funDefi(icv);
            if (sym != INT && sym != CHAR && sym != VOID)
                error(41);
        }
    } while (sym == INT || sym == CHAR || sym == VOID);
    if (sym != END)
        error(42);
}

bool cmp_by_value(const pair<string, int>& lhs, const pair<string, int>& rhs) {
    return lhs.second > rhs.second;
}
void var_count(map<string, int> &local_count_map, map<string, int> &global_count_map,string name) {
    if (isdigit(name[0]) || name[0] == '-')
        return;
    if (name == "RET")
        return;
    if (name[0] == 'T') {
        local_count_map[name]++;
        return;
    }
    if (tab[findTab(name)].block != 0)
        local_count_map[name]++;
    else
        global_count_map[name]++;
}
void optimize_count() {
    pt_block = 0;
    int last_index = 0;
    map<string, int> local_count_map;
    map<string, int> global_count_map;
    map<string, string> name_register_map;
    for (size_t index = 0; index < quad_code.size(); index++) {
        quadruple code = quad_code[index];
        if (code.a.name == "int" || code.a.name == "char" || code.a.name == "void" ||
            index == quad_code.size() - 1) { // 读完一个函数体
            vector<pair<string, int>> local_count_vec(local_count_map.begin(), local_count_map.end());
            sort(local_count_vec.begin(), local_count_vec.end(), cmp_by_value);   // 排序
            int i = 0;
            for (vector<pair<string, int>>::iterator it = local_count_vec.begin(); it!= local_count_vec.end(); it++) {   
                if (i < 8) {    // 给次数最多的8个局部变量分配寄存器
                    stringstream ss;
                    ss << "$t" << i;
                    name_register_map[it->first] = ss.str();
                    i++;
                }
            }
            if (index != 0)
                registers.push_back(name_register_map);
            local_count_map.clear();
            name_register_map.clear();
            last_index = index;
            pt_block++;
            continue;
        }
 
        if (code.b.name == "=") {
            if (code.d.name == "") {    // 格式: a = c
                var_count(local_count_map, global_count_map, code.a.name);
                var_count(local_count_map, global_count_map, code.c.name);
            }
            else if (code.f.name == "") {    // 格式: T = c op e
                var_count(local_count_map, global_count_map, code.a.name);
                var_count(local_count_map, global_count_map, code.c.name);
                var_count(local_count_map, global_count_map, code.e.name);
            }
            else {  // T = c [ e ]
                var_count(local_count_map, global_count_map, code.a.name);
                var_count(local_count_map, global_count_map, code.c.name);
                var_count(local_count_map, global_count_map, code.e.name);
            }
        }
        else if (code.e.name == "=") {   // T [ c ] = f
            var_count(local_count_map, global_count_map, code.a.name);
            var_count(local_count_map, global_count_map, code.c.name);
            var_count(local_count_map, global_count_map, code.f.name);
        }
        else if (code.b.name == "<" || code.b.name == "<=" || code.b.name == ">" ||
            code.b.name == ">=" || code.b.name == "!=" || code.b.name == "==") {    // a op c
            var_count(local_count_map, global_count_map, code.a.name);
            var_count(local_count_map, global_count_map, code.c.name);
        }
        else if (code.a.name == "PUSH") {
            var_count(local_count_map, global_count_map, code.b.name);
        }
        else if (code.a.name == "PRINTF") {
            if (code.b.name[0] != '\"') {
                var_count(local_count_map, global_count_map, code.b.name);
            }
        }
        else if (code.a.name == "SCANF")
            var_count(local_count_map, global_count_map, code.b.name);
        else if (code.a.name == "RET") {
            if (code.b.name != "") {
                var_count(local_count_map, global_count_map, code.b.name);
            }
        }
    }

    // 替换全局变量
    vector<pair<string, int>> global_count_vec(global_count_map.begin(), global_count_map.end());
    sort(global_count_vec.begin(), global_count_vec.end(), cmp_by_value);   // 排序
    int i = 0;
    for (vector<pair<string, int>>::iterator it = global_count_vec.begin(); it != global_count_vec.end(); it++) {
        if (i < 7) {    // 给次数最多的7个全局变量分配寄存器
            stringstream ss;
            ss << "$s" << i;
            name_register_map[it->first] = ss.str();
            i++;
        }
    }
    registers[0] = name_register_map;
    /*pt_block = 0;
    for (size_t index = 0; index < quad_code.size(); index++) {
        quadruple &old = quad_code[index];
        if (old.a.name == "int" || old.a.name == "char" || old.a.name == "void")
            pt_block++;
        else if (old.b.name == "=") {
            if (name_register_map[old.a.name] != "")
                if (tab[findTab(old.a.name)].block == 0)
                    old.a.name = name_register_map[old.a.name];
            if (name_register_map[old.c.name] != "")
                if (tab[findTab(old.c.name)].block == 0)
                    old.c.name = name_register_map[old.c.name];
            if (name_register_map[old.e.name] != "")
                if (tab[findTab(old.e.name)].block == 0)
                    old.e.name = name_register_map[old.e.name];
        }
        else if (old.e.name == "=") {   // a [ c ] = f
            if (name_register_map[old.a.name] != "")
                if (tab[findTab(old.a.name)].block == 0)
                    old.a.name = name_register_map[old.a.name];
            if (name_register_map[old.c.name] != "")
                if (tab[findTab(old.c.name)].block == 0)
                    old.c.name = name_register_map[old.c.name];
            if (name_register_map[old.f.name] != "")
                if (tab[findTab(old.f.name)].block == 0)
                    old.f.name = name_register_map[old.f.name];
        }
        else if (old.b.name == "<" || old.b.name == "<=" || old.b.name == ">" ||
            old.b.name == ">=" || old.b.name == "!=" || old.b.name == "==") {    // a op c
            if (name_register_map[old.a.name] != "")
                if (tab[findTab(old.a.name)].block == 0)
                    old.a.name = name_register_map[old.a.name];
            if (name_register_map[old.c.name] != "")
                if (tab[findTab(old.c.name)].block == 0)
                    old.c.name = name_register_map[old.c.name];
        }
        else if (old.a.name == "PUSH" || old.a.name == "PRINTF" || old.a.name == "SCANF" || old.a.name == "RET") {
            if (name_register_map[old.b.name] != "")
                if (tab[findTab(old.b.name)].block == 0)
                    old.b.name = name_register_map[old.b.name];
        }
    }*/
}

void optimize_DAG(){
    struct dag_node {
        int id;
        string op;
        int x, y;       // 左右子结点号
    }node;
    struct dag_item {
        string name;
        int id;
    }item;
    vector<dag_node> dag;       // DAG图
    vector<dag_item> dag_tab;   // 结点表
    int pt_node = 0;
    for (vector<quadruple>::iterator iter_quad = quad_code.begin(); iter_quad != quad_code.end(); iter_quad++) {
        quadruple& code = *iter_quad;   // 必须是引用！否则修改无效
        bool del = false;   // 标记是否需要删除当前四元式
        if (code.b.name == ":"||code.c.name=="(") {   // 标签划分基本块
            pt_node = 0;
            dag.clear();
            dag_tab.clear();
            continue;
        }
        int x, y, z;    // 记录a、c、e的结点号
        int i, tab_len, dag_len;
        if (code.b.name == "=") {   // a = c 或 a = c op e 或 a = c [ e ]
            if (code.c.name == "RET")
                continue;
            /* 查找并修改c */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.c.name) {   // 结点表中已有该变量
                    y = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // 结点表未找到
                y = node.id = item.id = ++pt_node;
                node.op = item.name = code.c.name;      // 叶结点op为变量名，子节点为0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // 添加到DAG图
                dag_tab.push_back(item);    // 添加到结点表
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == y && dag_tab[i].name[0] == 'T') {
                    if (code.d.name == ""&&code.a.name[0] == 'T')  // 重复的T = array，删去
                        del = true;
                    else
                        code.c.name = dag_tab[i].name;  // 将四元式的c修改为第一个与其节点号相同的临时变量
                    break;
                }
            }
            if (code.d.name == "") {    // 语句格式为：a = c 
                x = y; 
            }
            else {   // a = c op e 或 a = c [ e ]
                /* 查找并修改e */
                for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                    if (dag_tab[i].name == code.e.name) {   // 结点表中已有该变量
                        z = dag_tab[i].id;
                        break;
                    }
                } 
                if (i == tab_len) {    // 结点表未找到
                    z = node.id = item.id = ++pt_node;
                    node.op = item.name = code.e.name;      // 叶结点op为变量名，子节点为0
                    node.x = 0;
                    node.y = 0;
                    dag.push_back(node);        // 添加到DAG图
                    dag_tab.push_back(item);    // 添加到结点表
                }
                for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                    if (dag_tab[i].id == z && dag_tab[i].name[0] == 'T') {
                        code.e.name = dag_tab[i].name;  // 将四元式的e修改为第一个与其节点号相同的临时变量
                        break;
                    }
                }
                /* 查找并修改cde */
                for (i = 0, dag_len = dag.size(); i < dag_len; i++) {
                    if (dag[i].op == code.d.name&&dag[i].x == y&&dag[i].y == z) {   // DAG图中存在子结点和操作符都相同的结点
                        x = dag[i].id;
                        if (code.a.name[0] == 'T') { // 重复的临时变量计算，删去
                            del = true;
                        }
                        else {  // 将四元式右边重复的表达式替换为第一个结点相同的变量
                            for (int j = 0, tab_len = dag_tab.size(); j < tab_len; j++) {
                                if (dag_tab[j].id == x) {
                                    code.c.name = dag_tab[j].name;  // 将四元式的c修改为第一个与其节点号相同的变量名
                                    code.d.name = code.e.name = code.f.name = "";
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                if (i==dag_len) {   // DAG图中无该结点
                    x = node.id = ++pt_node;
                    node.op = code.d.name;
                    node.x = y;
                    node.y = z;
                    dag.push_back(node);
                }
            }
            // 更新结点表中a对应的结点号
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.a.name) {   // 结点表中存在该变量名，则更新结点号
                    dag_tab[i].id = x;
                    break;
                }
            }
            if (i == tab_len) { // 不存在则新增一项
                item.name = code.a.name;
                item.id = x;
                dag_tab.push_back(item);
            }
            if (del) {
                iter_quad = quad_code.erase(iter_quad); // 删除该四元式
                iter_quad--;    // erase()会将当前指针指向被删除元素的后一元素,再执行for里的++相对于跳过了一个元素，故需--
            }
        }
        else if (code.e.name == "=") {  // T [ c ] = f
            /* 查找并修改f */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.f.name) {   // 结点表中已有该变量
                    z = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // 结点表未找到
                z = node.id = item.id = ++pt_node;
                node.op = item.name = code.f.name;      // 叶结点op为变量名，子节点为0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // 添加到DAG图
                dag_tab.push_back(item);    // 添加到结点表
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == z && dag_tab[i].name[0] == 'T') {
                    code.f.name = dag_tab[i].name;  // 将四元式的f修改为第一个与其节点号相同的临时变量
                    break;
                }
            }
            /* 查找并修改a */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.a.name) {   // a为数组名临时变量T，之前必出现过
                    x = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // 结点表未找到
                x = node.id = item.id = ++pt_node;
                node.op = item.name = code.a.name;      // 叶结点op为变量名，子节点为0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // 添加到DAG图
                dag_tab.push_back(item);    // 添加到结点表
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == x && dag_tab[i].name[0] == 'T') {
                    code.a.name = dag_tab[i].name;  // 将四元式的a修改为第一个与其节点号相同的临时变量
                    break;
                }
            }
            /* 给数组名新分配结点(因为数组中的元素已被修改) */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == x && dag_tab[i].name[0] != 'T') {
                    dag_tab[i].id = node.id = ++pt_node;
                    node.op = dag_tab[i].name[0];      // 叶结点op为变量名，子节点为0
                    node.x = 0;
                    node.y = 0;
                    dag.push_back(node);        // 添加到DAG图
                    break;
                }
            }
            /* 查找并修改c */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.c.name) {   // 结点表中已有该变量
                    y = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // 结点表未找到
                y = node.id = item.id = ++pt_node;
                node.op = item.name = code.c.name;      // 叶结点op为变量名，子节点为0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // 添加到DAG图
                dag_tab.push_back(item);    // 添加到结点表
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == y && dag_tab[i].name[0] == 'T') {
                    code.c.name = dag_tab[i].name;  // 将四元式的f修改为第一个与其节点号相同的临时变量
                    break;
                }
            }
        }
        else if (code.b.name == "<" || code.b.name == "<=" || code.b.name == ">" ||
            code.b.name == ">=" || code.b.name == "!=" || code.b.name == "==") {    // a op c
            /* 查找并修改a */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.a.name) {   // 结点表中已有该变量
                    x = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // 结点表未找到
                x = node.id = item.id = ++pt_node;
                node.op = item.name = code.a.name;      // 叶结点op为变量名，子节点为0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // 添加到DAG图
                dag_tab.push_back(item);    // 添加到结点表
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == x && dag_tab[i].name[0] == 'T') {
                    code.a.name = dag_tab[i].name;  // 将四元式的f修改为第一个与其节点号相同的临时变量
                    break;
                }
            }
            /* 查找并修改c */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.c.name) {   // 结点表中已有该变量
                    y = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // 结点表未找到
                y = node.id = item.id = ++pt_node;
                node.op = item.name = code.c.name;      // 叶结点op为变量名，子节点为0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // 添加到DAG图
                dag_tab.push_back(item);    // 添加到结点表
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == y && dag_tab[i].name[0] == 'T') {
                    code.c.name = dag_tab[i].name;  // 将四元式的f修改为第一个与其节点号相同的临时变量
                    break;
                }
            }
        }
        else if (code.a.name == "PUSH"|| code.a.name == "PRINTF"|| code.a.name == "RET") {
            if (code.b.name == "")
                continue;
            /* 查找并修改b */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.b.name) {   // 结点表中已有该变量
                    x = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // 结点表未找到
                x = node.id = item.id = ++pt_node;
                node.op = item.name = code.b.name;      // 叶结点op为变量名，子节点为0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // 添加到DAG图
                dag_tab.push_back(item);    // 添加到结点表
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == x && dag_tab[i].name[0] == 'T') {
                    code.b.name = dag_tab[i].name;  // 将四元式的b修改为第一个与其节点号相同的临时变量
                    break;
                }
            }
        }
    }
}

void transform() {
    fout << ".data" << endl;
    for (size_t i = 0; i < stab.size(); i++) {
        fout << ".string" << i << ": .asciiz \"" << stab[i] << "\"" << endl;
    }
    fout << ".align 2" << endl; // 内存按word对齐
    fout << ".begin:" << endl;
    fout << ".text" << endl;
    fout << "la\t$gp, .begin" << endl;                          // $gp存全局数据区基址
    fout << "addi\t$s7, $gp, " << (ftab[0].size*4) << endl;     // $s7存当前数据区基址
                                                                // $sp存当前临时数据区基址
    fout << "j main" << endl;
    pc = 0;
    pt_block = 0;
    int pt_stab = -1;
    offset = 0;
    quadruple code;
    string base[] = { "$s7","$gp" };
    while (pc < quad_code.size()) {
        code = quad_code[pc];
        if (code.a.name == "int" || code.a.name == "char" || code.a.name == "void") {
            pt_block++;
            fout << code.b.name << ":" << endl;  // 生成函数标签
            pc++;
            continue;
        }
        /* 四元式为赋值语句 */
        if (code.b.name == "=") {
            if (code.d.name == "") {
                if (code.c.name == "RET") {     // 格式: T = RET
                    fout << "sw\t$v0, " << stoi(code.a.name.substr(1))*4 << "($sp)" << endl;
                    pc++;
                    continue;
                }
                tab_item a = tab[findTab(code.a.name)];
                int a_global = (a.block == 0) ? 1 : 0;
                if (code.c.name[0] == 'T') {         // 格式: x = T
                    fout << "lw\t$t8, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0]=='-') {  // 格式: x = 1
                    fout << "addi\t$t8, $0, " << code.c.name << endl;
                }
                else {                          // 格式: x = y
                    tab_item c = tab[findTab(code.c.name)];
                    int c_global = (c.block == 0) ? 1 : 0;
                    fout << "lw\t$t8, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
                }
                fout << "sw\t$t8, " << (a.adr * 4) << "(" << base[a_global] << ")" << endl;  // 保存临时变量的值
            }
            else if (code.f.name == "") {    // 格式: T = c op e
                if (code.c.name[0] == 'T') {         // c为T
                    fout << "lw\t$t8, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // c为数字
                    fout << "addi\t$t8, $0, " << code.c.name << endl;
                }
                else {                          // c为变量
                    tab_item c = tab[findTab(code.c.name)];
                    int c_global = (c.block == 0) ? 1 : 0;
                    fout << "lw\t$t8, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
                }
                if (code.e.name[0] == 'T') {         // e为T
                    fout << "lw\t$t9, " << stoi(code.e.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // e为数字
                    fout << "addi\t$t9, $0, " << code.e.name << endl;
                }
                else {                          // e为变量
                    tab_item e = tab[findTab(code.e.name)];
                    int e_global = (e.block == 0) ? 1 : 0;
                    fout << "lw\t$t9, " << (e.adr * 4) << "(" << base[e_global] << ")" << endl;
                }
                if (code.d.name == "+")
                    fout << "add\t$t8, $t8, $t9" << endl;
                else if (code.d.name == "-")
                    fout << "sub\t$t8, $t8, $t9" << endl;
                else if (code.d.name == "*") {
                    fout << "mult\t" << "$t8, $t9" << endl;
                    fout << "mflo\t$t8" << endl;
                }
                else if (code.d.name == "/") {
                    fout << "div\t$t8, $t8, $t9" << endl;
                    fout << "mflo\t" << "$t8" << endl;
                }
                fout << "sw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // 保存临时变量的值
            }
            else {  // T = c [ e ]
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "addi\t$t8, " << base[c_global] << ", " << (c.adr * 4) << endl;  // 计算该数组的基址
                if (code.e.name[0] == 'T') {         // 因子e为T
                    fout << "lw\t$t9, " << stoi(code.e.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                    fout << "sll\t$t9, $t9, 2" << endl;     // 数组下标乘4
                    fout << "add\t$t8, $t8, $t9" << endl;   // 计算该元素的地址
                    fout << "lw\t$t8, ($t8)" << endl;
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // e为数字
                    fout << "lw\t$t8, " << (stoi(code.e.name) * 4) << "($t8)" << endl;
                }
                else {  // e为变量
                    tab_item e = tab[findTab(code.e.name)];
                    int e_global = (e.block == 0) ? 1 : 0;
                    fout << "lw\t$t9, " << (e.adr * 4) << "(" << base[e_global] << ")" << endl; // 获取数组下标
                    fout << "sll\t$t9, $t9, 2" << endl;     // 数组下标乘4
                    fout << "add\t$t8, $t8, $t9" << endl;   // 计算该元素的地址
                    fout << "lw\t$t8, ($t8)" << endl;
                }
                fout << "sw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // 保存临时变量的值
            }
        }
        else if (code.e.name == "=") {   // a [ c ] = f
            tab_item a = tab[findTab(code.a.name)];
            int a_global = (a.block == 0) ? 1 : 0;
            fout << "addi\t$t8, " << base[a_global] << ", " << (a.adr * 4) << endl;  // 计算该数组的基址
            if (code.c.name[0] == 'T') {         // c为T
                fout << "lw\t$t9, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                fout << "sll\t$t9, $t9, 2" << endl;
                fout << "add\t$t8, $t8, $t9" << endl;   // 计算该元素的地址
            }
            else if (isdigit(code.c.name[0])) {  // c为数字(数组下标非负)
                fout << "addi\t$t8, $t8, " << (stoi(code.c.name) * 4) << endl;
            }
            else {      // c为变量
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "lw\t$t9, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl; // 获取数组下标
                fout << "sll\t$t9, $t9, 2" << endl;     // 数组下标乘4
                fout << "add\t$t8, $t8, $t9" << endl;   // 计算该元素的地址
            }
            if (code.f.name[0] == 'T') {         // 因子f为T
                fout << "lw\t$t9, " << stoi(code.f.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
            }
            else if (isdigit(code.f.name[0]) || code.f.name[0] == '-') {  // f为数
                fout << "addi\t$t9, $0, " << code.f.name << endl;
            }
            else {                          // f为变量
                tab_item f = tab[findTab(code.f.name)];
                int f_global = (f.block == 0) ? 1 : 0;
                fout << "lw\t$t9, " << (f.adr * 4) << "(" << base[f_global] << ")" << endl;
            }
            fout << "sw\t$t9, ($t8)" << endl;
        }
        else if(code.b.name ==":")    // 标签
            fout << code.a.name << code.b.name << endl;
        else if (code.b.name == "<" || code.b.name == "<=" || code.b.name == ">" || 
            code.b.name == ">=" || code.b.name == "!=" || code.b.name == "==") {    // a op c
            string instr;
            if (code.b.name == "<")
                instr = "slt";
            else if (code.b.name == "<=")
                instr = "sle";
            else if (code.b.name == ">")
                instr = "sgt";
            else if (code.b.name == ">=")
                instr = "sge";
            else if (code.b.name == "!=")
                instr = "sne";
            else 
                instr = "seq";
            if (code.a.name[0] == 'T')
                fout << "lw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
            else if (isdigit(code.a.name[0]) || code.a.name[0] == '-')
                fout << "addi\t$t8, $0, " << code.a.name << endl;
            else {
                tab_item a = tab[findTab(code.a.name)];
                int a_global = (a.block == 0) ? 1 : 0;
                fout << "lw\t$t8, " << (a.adr * 4) << "(" << base[a_global] << ")" << endl;
            }
            if (code.c.name[0] == 'T')
                fout << "lw\t$t9, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
            else if (isdigit(code.c.name[0])||code.c.name[0]=='-')
                fout << "addi\t$t9, $0, " << code.c.name << endl;
            else {
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "lw\t$t9, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
            }
            fout << instr << "\t$t8, $t8, $t9" << endl;
            code = quad_code[++pc];
            if (code.a.name == "BZ") {
                fout << "beq\t$t8, $0, " << code.b.name << endl;
            }
            else if (code.a.name == "BNZ") {
                fout << "bne\t$t8, $0, " << code.b.name << endl;
            }
        }
        else if (code.a.name == "GOTO") {
            fout << "j\t" << code.b.name << endl;
        }
        else if (code.a.name == "PUSH") {
            fout << "addi\t$t8, $s7, " << (ftab[pt_block].size * 4) << endl;    // 计算下个函数体的基址
            if (code.b.name[0] == 'T') {   // PUSH T
                fout << "lw\t$t9, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                fout << "sw\t$t9, " << (++offset * 4) << "($t8)" << endl;  // 参数只能为int或char，故大小每次加4即可
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {  // PUSH 数字
                fout << "addi\t$t9, $0, " << code.b.name << endl;
                fout << "sw\t$t9, " << (++offset * 4) << "($t8)" << endl;
            }
            else {  // PUSH 变量  
                tab_item b = tab[findTab(code.b.name)];
                int b_global = (b.block == 0) ? 1 : 0;
                fout << "lw\t$t9, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
                fout << "sw\t$t9, " << (++offset * 4) << "($t8)" << endl;
            }
        }
        else if (code.a.name == "PRINTF") {
            if (code.b.name[0] == '\"') {    // 输出字符串
                fout << "la\t$a0, .string" << ++pt_stab << endl;    // 加载字符串地址到$a0
                fout << "addi\t$v0, $0, 4" << endl;         // print string
            }
            else if (code.b.name[0] == 'T') {
                if (code.b.type == CHARTP) {    
                    fout << "lw\t$a0, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // 将char值取到$a0
                    fout << "addi\t$v0, $0, 11" << endl;    // print char
                }
                else {
                    fout << "lw\t$t8, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                    fout << "add\t$a0, $0, $t8" << endl;
                    fout << "addi\t$v0, $0, 1" << endl;     // print int
                }
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {
                fout << "addi\t$a0, $0, " << code.b.name << endl;
                fout << "addi\t$v0, $0, 1" << endl;
            }
            else {
                tab_item b = tab[findTab(code.b.name)];
                int b_global = (b.block == 0) ? 1 : 0;
                if (code.b.type == CHARTP) {     // char变量需要输出字符
                    fout << "lw\t$a0, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
                    fout << "addi\t$v0, $0, 11" << endl;
                }
                else {
                    fout << "lw\t$a0, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
                    fout << "addi\t$v0, $0, 1" << endl;
                }
            }
            fout << "syscall" << endl;
        }
        else if (code.a.name == "SCANF") {
            tab_item b = tab[findTab(code.b.name)];
            int b_global = (b.block == 0) ? 1 : 0;
            if (code.b.type == CHARTP) {     // 读入char
                fout << "addi\t$v0, $0, 12" << endl;    // 读入char到$v0
                fout << "syscall" << endl;
                fout << "sw\t$v0, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
            }
            else {
                fout << "addi\t$v0, $0, 5" << endl;     // 读入int到$v0
                fout << "syscall" << endl;
                fout << "sw\t$v0, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl; // 将值存到变量地址上
            }
        }
        else if (code.a.name == "CALL") {
            offset = 0;
            /* 跳转进其他函数前 */
            fout << "sw\t$ra, ($s7)" << endl;                                   // 保存当前函数的返回地址
            fout << "addi\t$s7, $s7, " << (ftab[pt_block].size * 4) << endl;    // 更新$s7基址位置(加当前函数数据区大小)
            fout << "addi\t$sp, $sp, " << (ftab[pt_block].tsize * 4) << endl;   // 更新$sp基址位置(加当前函数临时数据区大小)
            fout << "jal\t" << code.b.name << endl;
            /* 从其他函数跳回后 */
            fout << "addi\t$s7, $s7, " << -(ftab[pt_block].size * 4) << endl;   // 更新$s7基址位置(减当前函数数据区大小)
            fout << "addi\t$sp, $sp, " << -(ftab[pt_block].tsize * 4) << endl;  // 更新$sp基址位置(减当前函数临时数据区大小)
            fout << "lw\t$ra, ($s7)" << endl;                                   // 将保存的该函数返回地址赋给$ra
        }
        else if (code.a.name == "RET") {
            if (code.b.name == "") {
                if (pt_block == ftab.size() - 1)   // main函数里的return
                    fout << "j\t.end" << endl;
                else
                    fout << "jr\t$ra" << endl;
            }
            else if (code.b.name[0] == 'T') {
                fout << "lw\t$t8, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                fout << "add\t$v0, $0, $t8" << endl;
                fout << "jr\t$ra" << endl;
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {
                fout << "addi\t$v0, $0, " << code.b.name << endl;
                fout << "jr\t$ra" << endl;
            }
            else {  // 返回变量
                tab_item b = tab[findTab(code.b.name)];
                int b_global = (b.block == 0) ? 1 : 0;
                fout << "lw\t$v0, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
                fout << "jr\t$ra" << endl;
            }
        }
        pc++;
    }
    fout << ".end :" << endl;
    fout << "addi\t$v0, $0, 10" << endl;
    fout << "syscall" << endl;
}

void transform_opt() {
    fout << ".data" << endl;
    for (size_t i = 0; i < stab.size(); i++) {
        fout << ".string" << i << ": .asciiz \"" << stab[i] << "\"" << endl;
    }
    fout << ".align 2" << endl; // 内存按word对齐
    fout << ".begin:" << endl;
    fout << ".text" << endl;
    fout << "la\t$gp, .begin" << endl;                          // $gp存全局数据区基址
    fout << "addi\t$s7, $gp, " << (ftab[0].size * 4) << endl;   // $s7存当前数据区基址
                                                                // $sp存当前临时数据区基址
    // 保存数组型全局变量的基址到寄存器
    for (int i = 0; tab[i].block==0; i++) {
        if (tab[i].type == ARRAYTP) {
            if (registers[0][tab[i].name] != "")    // 若该变量分配了寄存器
                fout << "addi\t" << registers[0][tab[i].name] << ", $gp, " << (tab[i].adr * 4) << endl;  
        }
    }
    fout << "j main" << endl;
    pc = 0;
    pt_block = 0;
    int pt_stab = -1;
    offset = 0;
    quadruple code;
    while (pc < quad_code.size()) {
        code = quad_code[pc];
        if (code.a.name == "int" || code.a.name == "char" || code.a.name == "void") {
            pt_block++;
            fout << code.b.name << ":" << endl;  // 生成函数标签
            // 将已分配$的参数的值从地址加载到寄存器
            int index;
            for (index = tab.size() - 1; index >= 0; index--) { // 查找函数名在tab中的位置
                if (tab[index].block != 0)
                    continue;
                if (tab[index].name == code.b.name)
                    break;
            }
            for (int i = index+1; i <= ftab[pt_block].lastpara; i++) {  // 遍历该函数参数
                if (registers[pt_block][tab[i].name] != "") {   //如果该参数分配了寄存器
                    fout << "lw\t" << registers[pt_block][tab[i].name] << ", " << (tab[i].adr * 4) << "($s7)" << endl;
                }
            }
            // 将数组型局部变量的地址加载到寄存器
            for (size_t i = index + 1; i < tab.size() && tab[i].block == pt_block; i++) {
                if (tab[i].type == ARRAYTP) {
                    if (registers[pt_block][tab[i].name] != "")    // 若该变量分配了寄存器
                        fout << "addi\t" << registers[pt_block][tab[i].name] << ", $sp, " << (tab[i].adr * 4) << endl;
                }
            }
            pc++;
            continue;
        }
        /* 四元式为赋值语句 */
        if (code.b.name == "=") {
            if (code.d.name == "") {
                if (code.c.name == "RET") {     // 格式: T = RET
                    if (registers[pt_block][code.a.name] != "")
                        fout << "add\t" << registers[pt_block][code.a.name] << ", $0, $v0" << endl;
                    else
                        fout << "sw\t$v0, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;
                    pc++;
                    continue;
                }

                tab_item a = tab[findTab(code.a.name)];
                int r0 = (a.block == 0) ? 0 : pt_block;
                string t0 = registers[r0][code.a.name];
                if (t0 != "") {     // a分配了寄存器
                    if (code.c.name[0] == 'T') {         // 格式: x = T
                        string t1 = registers[pt_block][code.c.name];
                        if (t1 != "")   // c分配了寄存器
                            fout << "add\t" << t0 << ", $0, " << t1 << endl;
                        else {
                            fout << "lw\t"<< t0 <<", " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                        }
                    }
                    else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // 格式: x = 1
                        fout << "addi\t" << t0 << ", $0, " << code.c.name << endl;
                    }
                    else {                          // 格式: x = y
                        tab_item c = tab[findTab(code.c.name)];
                        int r1 = (c.block == 0) ? 0 : pt_block;
                        string t1 = registers[r1][code.c.name]; 
                        if (t1 != "") { // c分配了寄存器
                            fout << "add\t" << t0 << ", $0, " << t1 << endl;
                        }
                        else {
                            string base_c = (tab[findTab(code.c.name)].block == 0) ? "$gp" : "$s7";
                            fout << "lw\t" << t0 << ", " << (c.adr * 4) << "(" << base_c << ")" << endl;
                        }
                    }
                }
                else {
                    string base_a = (tab[findTab(code.a.name)].block == 0) ? "$gp" : "$s7";
                    if (code.c.name[0] == 'T') {         // 格式: x = T
                        string t1 = registers[pt_block][code.c.name];
                        if (t1 != "")   // c分配了寄存器
                            fout << "sw\t" << t1 << ", " << (a.adr * 4) << "(" << base_a << ")" << endl;
                        else {
                            fout << "lw\t$t8, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl; 
                            fout << "sw\t$t8, " << (a.adr * 4) << "(" << base_a << ")" << endl;
                        }
                    }
                    else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // 格式: x = 1
                        fout << "addi\t$t8, $0, " << code.c.name << endl;
                        fout << "sw\t$t8, " << (a.adr * 4) << "(" << base_a << ")" << endl;
                    }
                    else {                          // 格式: x = y
                        tab_item c = tab[findTab(code.c.name)];
                        int r1 = (c.block == 0) ? 0 : pt_block;
                        string t1 = registers[r1][code.c.name];
                        if (t1 != "") { // c分配了寄存器
                            fout << "sw\t" << t1 << ", " << (a.adr * 4) << "(" << base_a << ")" << endl;
                        }
                        else {
                            string base_c = (c.block == 0) ? "$gp" : "$s7";
                            fout << "lw\t$t8, " << (c.adr * 4) << "(" << base_c << ")" << endl;
                            fout << "sw\t$t8, " << (a.adr * 4) << "(" << base_a << ")" << endl;
                        }
                    }
                }
            }
            else if (code.f.name == "") {    // 格式: T = c op e
                string t0, t1, t2;  // 保存a,c,e的寄存器
                if (code.c.name[0] == 'T') {         // c为T
                    t1 = registers[pt_block][code.c.name];
                    if (t1 == "") {  // c未分配寄存器
                        fout << "lw\t$t8, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;
                        t1 = "$t8";
                    }
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // c为数字
                    fout << "addi\t$t8, $0, " << code.c.name << endl;
                    t1 = "$t8";
                }
                else {                          // c为变量
                    tab_item c = tab[findTab(code.c.name)];
                    int r1 = (c.block == 0) ? 0 : pt_block;
                    string base_c = (c.block == 0) ? "$gp" : "$s7";
                    t1 = registers[r1][code.c.name];
                    if (t1 == "") {  // c未分配寄存器
                        fout << "lw\t$t8, " << (c.adr * 4) << "(" << base_c << ")" << endl;
                        t1 = "$t8";
                    }
                }

                if (code.e.name[0] == 'T') {         // e为T
                    t2 = registers[pt_block][code.e.name];
                    if (t2 == "") {  // e未分配寄存器
                        fout << "lw\t$t9, " << stoi(code.e.name.substr(1)) * 4 << "($sp)" << endl;
                        t2 = "$t9";
                    }
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // e为数字
                    fout << "addi\t$t9, $0, " << code.e.name << endl;
                    t2 = "$t9";
                }
                else {                          // e为变量
                    tab_item e = tab[findTab(code.e.name)];
                    int r2 = (e.block == 0) ? 0 : pt_block;
                    string base_e = (e.block == 0) ? "$gp" : "$s7";
                    t2 = registers[r2][code.e.name];
                    if (t2 == "") {  // c未分配寄存器
                        fout << "lw\t$t9, " << (e.adr * 4) << "(" << base_e << ")" << endl;
                        t2 = "$t9";
                    }
                }

                t0 = registers[pt_block][code.a.name];
                if (t0 == "")   // a未分配寄存器
                    t0 = "$t8";
                if (code.d.name == "+")
                    fout << "add\t" << t0 << ", " << t1 << ", " << t2 << endl;
                else if (code.d.name == "-")
                    fout << "sub\t" << t0 << ", " << t1 << ", " << t2 << endl;
                else if (code.d.name == "*") {
                    fout << "mult\t" << t1 << ", " << t2 << endl;
                    fout << "mflo\t" << t0 << endl;
                }
                else if (code.d.name == "/") {
                    fout << "div\t" << t1 << ", " << t2 << endl;
                    fout << "mflo\t" << t0 << endl;
                }
                if (t0 == "$t8")
                    fout << "sw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // 保存临时变量的值
            }
            else {  // T = c [ e ]
                string t0, t1, t2;  // 保存a,c,e的寄存器
                tab_item c = tab[findTab(code.c.name)];
                int r1 = (c.block == 0) ? 0 : pt_block;
                string base_c = (c.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.c.name];
                if (t1 == "") {  // c未分配寄存器
                    fout << "addi\t$t8, " << base_c << ", " << (c.adr * 4) << endl;  // 计算该数组的基址
                    t1 = "$t8";
                }
                if (code.e.name[0] == 'T') {         // 因子e为T
                    t2 = registers[pt_block][code.e.name];
                    if (t2 == "") {  // e未分配寄存器
                        fout << "lw\t$t9, " << stoi(code.e.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                        t2 = "$t9";
                    }
                    fout << "sll\t$t9, " << t2 << ", 2" << endl;        // 数组下标乘4
                    fout << "add\t$t8, " << t1 << ", $t9" << endl;      // 计算该元素的地址
                    fout << "lw\t$t8, ($t8)" << endl;               // 取值
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // e为数字
                    fout << "lw\t$t8, " << (stoi(code.e.name) * 4) << "(" << t1 << ")" << endl;
                }
                else {  // e为变量
                    tab_item e = tab[findTab(code.e.name)];
                    int r2 = (e.block == 0) ? 0 : pt_block;
                    string base_e = (e.block == 0) ? "$gp" : "$s7";
                    t2 = registers[r2][code.e.name];
                    if (t2 == "") {  // e未分配寄存器
                        fout << "lw\t$t9, " << (e.adr * 4) << "(" << base_e << ")" << endl;
                        t2 = "$t9";
                    }
                    fout << "sll\t$t9, " << t2 << ", 2" << endl;        // 数组下标乘4
                    fout << "add\t$t8, " << t1 << ", $t9" << endl;      // 计算该元素的地址
                    fout << "lw\t$t8, ($t8)" << endl;               // 取值
                }
                t0 = registers[pt_block][code.a.name];
                if (t0 == "") {  // a未分配寄存器
                    fout << "sw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;
                }
                else
                    fout << "add\t" << t0 << ", $0, $t8" << endl;
            }
        }
        else if (code.e.name == "=") {   // a [ c ] = f
            string t0, t1, t2;  // 保存a,c,f的寄存器
            tab_item a = tab[findTab(code.a.name)];
            int r0 = (a.block == 0) ? 0 : pt_block;
            string base_a = (a.block == 0) ? "$gp" : "$s7";
            t0 = registers[r0][code.a.name];
            if (t0 == "") {
                fout << "addi\t$t8, " << base_a << ", " << (a.adr * 4) << endl;  // 计算该数组的基址
                t0 = "$t8";
            }
            if (code.c.name[0] == 'T') {         // c为T
                t1 = registers[pt_block][code.c.name];
                if (t1 == "") {
                    fout << "lw\t$t9, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                    t1 = "$t9";
                }
                fout << "sll\t$t9, " << t1 << ", 2" << endl;
                fout << "add\t$t8, " << t0 << ", $t9" << endl;  // 计算该元素的地址
            }
            else if (isdigit(code.c.name[0])) {  // c为数字(数组下标非负)
                fout << "addi\t$t8, " << t0 << ", " << (stoi(code.c.name) * 4) << endl;
            }
            else {      // c为变量
                tab_item c = tab[findTab(code.c.name)];
                int r1 = (c.block == 0) ? 0 : pt_block;
                string base_c = (c.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.c.name];
                if (t1 == "") {
                    fout << "lw\t$t9, " << (c.adr * 4) << "(" << base_c << ")" << endl; // 获取数组下标
                    t1 = "$t9";
                }
                fout << "sll\t$t9, " << t1 << ", 2" << endl;
                fout << "add\t$t8, " << t0 << ", $t9" << endl;  // 计算该元素的地址
            }
            if (code.f.name[0] == 'T') {         // 因子f为T
                t2 = registers[pt_block][code.f.name];
                if (t2 == "") {
                    fout << "lw\t$t9, " << stoi(code.f.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                    t2 = "$t9";
                }
            }
            else if (isdigit(code.f.name[0]) || code.f.name[0] == '-') {  // f为数
                fout << "addi\t$t9, $0, " << code.f.name << endl;
                t2 = "$t9";
            }
            else {                          // f为变量
                tab_item f = tab[findTab(code.f.name)];
                int r2 = (f.block == 0) ? 0 : pt_block;
                string base_f = (f.block == 0) ? "$gp" : "$s7";
                t2 = registers[r2][code.f.name];
                if (t2 == "") {
                    fout << "lw\t$t9, " << (f.adr * 4) << "(" << base_f << ")" << endl;
                    t2 = "$t9";
                }
            }
            fout << "sw\t" << t2 << ", ($t8)" << endl;
        }
        else if (code.b.name == ":")    // 标签
            fout << code.a.name << code.b.name << endl;
        else if (code.b.name == "<" || code.b.name == "<=" || code.b.name == ">" ||
            code.b.name == ">=" || code.b.name == "!=" || code.b.name == "==") {    // a op c
            string instr, t1, t2;
            if (code.b.name == "<")
                instr = "slt";
            else if (code.b.name == "<=")
                instr = "sle";
            else if (code.b.name == ">")
                instr = "sgt";
            else if (code.b.name == ">=")
                instr = "sge";
            else if (code.b.name == "!=")
                instr = "sne";
            else
                instr = "seq";
            if (code.a.name[0] == 'T') {
                t1 = registers[pt_block][code.a.name];
                if (t1 == "") {
                    fout << "lw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                    t1 = "$t8";
                }
            }
            else if (isdigit(code.a.name[0]) || code.a.name[0] == '-') {
                fout << "addi\t$t8, $0, " << code.a.name << endl;
                t1 = "$t8";
            }
            else {
                tab_item a = tab[findTab(code.a.name)];
                int r1 = (a.block == 0) ? 0 : pt_block;
                string base_a = (a.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.a.name];
                if (t1 == "") {
                    fout << "lw\t$t8, " << (a.adr * 4) << "(" << base_a << ")" << endl;
                    t1 = "$t8";
                }
            }
            if (code.c.name[0] == 'T') {
                t2 = registers[pt_block][code.c.name];
                if (t2 == "") {
                    fout << "lw\t$t9, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                    t2 = "$t9";
                }
            }
            else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {
                fout << "addi\t$t9, $0, " << code.c.name << endl;
                t2 = "$t9";
            }
            else {
                tab_item c = tab[findTab(code.c.name)];
                int r2 = (c.block == 0) ? 0 : pt_block;
                string base_c = (c.block == 0) ? "$gp" : "$s7";
                t2 = registers[r2][code.c.name];
                if (t2 == "") {
                    fout << "lw\t$t9, " << (c.adr * 4) << "(" << base_c << ")" << endl;
                    t2 = "$t9";
                }
            }
            fout << instr << "\t$t8, " << t1 << ", " << t2 << endl;
            code = quad_code[++pc];
            if (code.a.name == "BZ") {
                fout << "beq\t$t8, $0, " << code.b.name << endl;
            }
            else if (code.a.name == "BNZ") {
                fout << "bne\t$t8, $0, " << code.b.name << endl;
            }
        }
        else if (code.a.name == "GOTO") {
            fout << "j\t" << code.b.name << endl;
        }
        else if (code.a.name == "PUSH") {
            string t1;
            fout << "addi\t$t8, $s7, " << (ftab[pt_block].size * 4) << endl;    // 计算下个函数体的基址
            if (code.b.name[0] == 'T') {   // PUSH T
                t1 = registers[pt_block][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$t9, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                    t1 = "$t9";
                }
                fout << "sw\t" << t1 << ", " << (++offset * 4) << "($t8)" << endl;  // 参数只能为int或char，故大小每次加4即可
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {  // PUSH 数字
                fout << "addi\t$t9, $0, " << code.b.name << endl;
                fout << "sw\t$t9, " << (++offset * 4) << "($t8)" << endl;
            }
            else {  // PUSH 变量  
                tab_item b = tab[findTab(code.b.name)];
                int r1 = (b.block == 0) ? 0 : pt_block;
                string base_b = (b.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$t9, " << (b.adr * 4) << "(" << base_b << ")" << endl;
                    t1 = "$t9";
                }
                fout << "sw\t" << t1 << ", " << (++offset * 4) << "($t8)" << endl;  // 参数只能为int或char，故大小每次加4即可
            }
        }
        else if (code.a.name == "PRINTF") {
            string t1;
            if (code.b.name[0] == '\"') {    // 输出字符串
                fout << "la\t$a0, .string" << ++pt_stab << endl;    // 加载字符串地址到$a0
                fout << "addi\t$v0, $0, 4" << endl;         // print string
            }
            else if (code.b.name[0] == 'T') {
                t1 = registers[pt_block][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$a0, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // 将char值取到$a0
                }
                else
                    fout << "add\t$a0, $0, " << t1 << endl;
                if (code.b.type == CHARTP) {
                    fout << "addi\t$v0, $0, 11" << endl;    // print char
                }
                else {
                    fout << "addi\t$v0, $0, 1" << endl;     // print int
                }
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {
                fout << "addi\t$a0, $0, " << code.b.name << endl;
                fout << "addi\t$v0, $0, 1" << endl;
            }
            else {
                tab_item b = tab[findTab(code.b.name)];
                int r1 = (b.block == 0) ? 0 : pt_block;
                string base_b = (b.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$a0, " << (b.adr * 4) << "(" << base_b << ")" << endl;
                }
                else
                    fout << "add\t$a0, $0, " << t1 << endl;
                if (code.b.type == CHARTP) {     // char变量需要输出字符
                    fout << "addi\t$v0, $0, 11" << endl;
                }
                else {
                    fout << "addi\t$v0, $0, 1" << endl;
                }
            }
            fout << "syscall" << endl;
        }
        else if (code.a.name == "SCANF") {
            string t1;
            tab_item b = tab[findTab(code.b.name)];
            int r1 = (b.block == 0) ? 0 : pt_block;
            string base_b = (b.block == 0) ? "$gp" : "$s7";
            t1 = registers[r1][code.b.name];
            if (code.b.type == CHARTP) {     // 读入char
                fout << "addi\t$v0, $0, 12" << endl;    // 读入char到$v0
            }
            else {
                fout << "addi\t$v0, $0, 5" << endl;     // 读入int到$v0
            }
            fout << "syscall" << endl;
            if (t1 == "") {
                fout << "sw\t$v0, " << (b.adr * 4) << "(" << base_b << ")" << endl; // 将值存到变量地址上
            }
            else
                fout << "add\t" << t1 << ", $v0, $0, " << endl;
        }
        else if (code.a.name == "CALL") {
            offset = 0;
            /* 跳转进其他函数前 */
            fout << "sw\t$ra, ($s7)" << endl;                                   // 保存当前函数的返回地址
            fout << "addi\t$t8, $sp, " << (ftab[pt_block].tsize * 4) << endl;   // 保存局部寄存器的值
            int i = 0;
            for (map<string, string>::iterator it = registers[pt_block].begin(); it != registers[pt_block].end(); it++) {
                if (it->second != "") {
                    fout << "sw\t$t" << i << ", " << i * 4 << "($t8)" << endl;
                    i++;
                }
            }
            fout << "addi\t$s7, $s7, " << (ftab[pt_block].size * 4) << endl;    // 更新$s7基址位置(加当前函数数据区大小)
            fout << "addi\t$sp, $sp, " << ((ftab[pt_block].tsize + 8) * 4) << endl;   // 更新$sp基址位置(加当前函数临时数据区+局部寄存器大小)
            fout << "jal\t" << code.b.name << endl;
            /* 从其他函数跳回后 */
            fout << "addi\t$sp, $sp, " << -((ftab[pt_block].tsize + 8) * 4) << endl;  // 更新$sp基址位置(减当前函数临时数据区+局部寄存器大小)
            fout << "addi\t$s7, $s7, " << -(ftab[pt_block].size * 4) << endl;   // 更新$s7基址位置(减当前函数数据区大小)
            fout << "addi\t$t8, $sp, " << (ftab[pt_block].tsize * 4) << endl;    // 将保存的局部寄存器值赋回
            i = 0;
            for (map<string, string>::iterator it = registers[pt_block].begin(); it != registers[pt_block].end(); it++) {
                if (it->second != "") {
                    fout << "lw\t$t" << i << ", " << i * 4 << "($t8)" << endl;
                    i++;
                }
            }
            fout << "lw\t$ra, ($s7)" << endl;                                   // 将保存的该函数返回地址赋给$ra
        }
        else if (code.a.name == "RET") {
            string t1;
            if (code.b.name == "") {
                if (pt_block == ftab.size() - 1)   // main函数里的return
                    fout << "j\t.end" << endl;
                else
                    fout << "jr\t$ra" << endl;
            }
            else if (code.b.name[0] == 'T') {
                t1 = registers[pt_block][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$t8, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // 取出临时变量的值
                    t1 = "$t8";
                }
                fout << "add\t$v0, $0, " << t1 << endl;
                fout << "jr\t$ra" << endl;
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {
                fout << "addi\t$v0, $0, " << code.b.name << endl;
                fout << "jr\t$ra" << endl;
            }
            else {   
                tab_item b = tab[findTab(code.b.name)];
                int r1 = (b.block == 0) ? 0 : pt_block;
                string base_b = (b.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$v0, " << (b.adr * 4) << "(" << base_b << ")" << endl;
                }
                else
                    fout << "add\t$v0, $0, " << t1 << endl;
                fout << "jr\t$ra" << endl;
            }
        }
        pc++;
    }
    fout << ".end :" << endl;
    fout << "addi\t$v0, $0, 10" << endl;
    fout << "syscall" << endl;
}

void printTabs() {
    fout << endl << endl << "符号表：" << endl;
    fout << "[index] [name] [kind] [type] [ref] [adr] [block]" << endl;
    for (size_t i = 0; i < tab.size(); i++)    // tab表内容
       fout << i << "\t" << tab[i].name << "\t" << kinds_string[tab[i].kind] << "\t" << types_string[tab[i].type]
        << "\t" << tab[i].ref << "\t" << tab[i].adr << "\t" << tab[i].block << endl;
    fout << endl << endl << "数组信息表：" << endl;
    fout << "[index] [eltype] [length]" << endl;
    for (size_t i = 0; i < atab.size(); i++)    // atab表内容
        fout << i << "\t" << ((atab[i].eltype == INTTP) ? "INT\t" : "CHAR") << "\t" << atab[i].length << endl;
    fout << endl << endl << "函数信息表：" << endl;
    fout << "[index] [lastpara] [size] [tsize]" << endl; 
    for (size_t i = 0; i < ftab.size(); i++)    // ftab表内容
        fout << i << "\t" << ftab[i].lastpara << "\t" << ftab[i].size << "\t" << ftab[i].tsize << endl;
    fout << endl << endl << "字符串表：" << endl;
    fout << "[index] [string]" << endl;
    for (size_t i = 0; i < stab.size(); i++)    // stab表内容
        fout << i << "\t" << stab[i] << endl;
    fout << endl << endl << "四元式代码：" << endl;
    for (size_t i = 0; i < quad_code.size(); i++)
        fout << i << "\t" << quad_code[i].a.name + " " + quad_code[i].b.name + " " + quad_code[i].c.name + 
        " " + quad_code[i].d.name + " " + quad_code[i].e.name + " " + quad_code[i].f.name << endl;
}

int main()
{
    pt_label = -1;
    pt_block = 0;
    
    string file;
    cout << "请输入需要编译的源程序绝对路径:" << endl;
    getline(cin, file);
    fin.open(file);
    while (!fin) {
        cout << "无法打开该文件，请确保文件可读并重新输入路径：" << endl;
        getline(cin, file);
        fin.open(file);
    }
    fin >> noskipws;    // 不跳过空格回车
    cout << "请输入编译输出文件的绝对路径:" << endl;
    getline(cin, file);
    fout.open(file);
    while (!fout) {
        cout << "无法打开该文件，请确保文件可写并重新输入路径：" << endl;
        getline(cin, file);
        fout.open(file);
    }

    ch = ' ';
    while (ch < 0 || isspace(ch)) {   // 判断预读的是否为空白字符
        if (ch < 0) {
            isError = true;
            fout << endl << "第" << nexrow << "行：存在非法字符";
        }
        if (ch == '\n')
            nexrow++;
        getch();
    }
    getsymbol();
    try
    {
        program();
    }
    catch (const std::exception&)
    {
        fin.close();
        fout.close();
        cout << "源程序有错误!" << endl;
        exit(1);
    }
    if (isError) {
        fin.close();
        fout.close();
        cout << "源程序有错误!" << endl;
        exit(1);
    }

    optimize_DAG();
    printTabs();
    optimize_count();
    transform_opt();

  //  transform();

    fin.close();
    fout.close();
}

