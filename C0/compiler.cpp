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
size_t pt_r, pt_block, pt_label, pc, offset;
char ch;
int value;
string str;
symbol sym;
string id;
ifstream fin;
ofstream fout;

void error(int id) {
    string msg[100];
    msg[1] = "超过int上限"; msg[2] = "缺少'"; msg[3] = "字符串内容出错"; msg[4] = "!只能与=连用";
    msg[5] = "无法识别符号"; msg[6] = "标识符重复定义"; msg[7] = "缺少(或;"; msg[8] = "void函数不能有返回值或跟()";
    msg[9] = "函数缺少返回值（返回值应在()内）"; msg[10] = "缺少标识符"; msg[11] = "缺少="; msg[12] = "缺少int或char";
    msg[13] = "缺少const"; msg[14] = "常量定义类型出错"; msg[15] = "缺少;"; msg[16] = "缺少无符号整数";
    msg[17] = "缺少]"; msg[18] = "无参数的函数调用不能跟()"; msg[19] = "传入参数过多"; msg[20] = "传入参数过少"; 
    msg[21] = "缺少)"; msg[22] = "标识符未定义"; msg[23] = "[使用出错"; msg[24] = "调用的函数无返回值";
    msg[25] = "因子类型出错"; msg[26] = "缺少("; msg[27] = "缺少else"; msg[28] = "标识符非变量"; 
    msg[29] = "缺少case"; msg[30] = "case后缺少常量"; msg[31] = "case标号重复定义"; msg[32] = "缺少:";
    msg[33] = "缺少["; msg[34] = "不能对常量进行赋值"; msg[35] = "缺少}"; msg[36] = "无法识别语句"; 
    msg[37] = "变量定义出错"; msg[38] = "缺少{"; msg[39] = "函数定义出错"; msg[40] = "main函数类型必须为void";
    msg[41] = "缺少main函数"; msg[42] = "main函数后有多余字符"; msg[43] = "case常量类型错误"; msg[44] = "标识符定义出错";
    msg[45] = "非0整数不能以0开头";
    fout << endl << msg[id];
    fin.close();
    fout.close();
    exit(id);
}

void getch() {
    if (fin.peek() != EOF) {
        fin >> ch;
        fout << ch;
    }
    else
        ch = NULL;
}
void getsymbol() {
    while (isspace(ch)) {   // 判断是否为空白字符' '、'\t'、'\r'、'\n'
        getch();
    }
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
        if (ch == '0') {    
            getch();
            if (isdigit(ch))    // 不能以0开头
                error(45);
            return;
        }
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
        else
            error(4);
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
        if (tab[i].block == block&&tab[i].name == id)
            error(6);
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
    if (sym != IDENT)
        error(10);
    getsymbol();
    if (sym != BECOME)
        error(11);
    getsymbol();
    if (ic == INTTP && (sym == PLUS || sym == MINUS)) {
        bool n = (sym == MINUS) ? true : false;
        getsymbol();
        if (sym != NUMBER)
            error(16);
        if (n)
            value = -value;
    }
    if ((ic == INTTP&&sym != NUMBER) || (ic == CHARTP&&sym != CHARA))
        error(14);
    enter(CONSTANT, ic, block);  // 常量进表
    getsymbol();
}
void constsDefi(int block) {      // 处理<常量说明>   sym进时为CONST，出时为分号后第一个符号
    if (sym != CONST)
        error(13);
    getsymbol();
    if (sym != INT&&sym != CHAR)
        error(14);
    types ic = (sym == INT) ? INTTP : CHARTP;    // 记录当前常量类型为int还是char
    getsymbol();
    constDefi(ic, block);   // 处理一个常量定义
    while (sym == COMMA) {  // 还有同类常量定义
        getsymbol();
        constDefi(ic, block);
    }
    if (sym != SEMICOLON)
        error(15);
    getsymbol();
    if (sym == CONST)
        constsDefi(block);  // 递归调用，继续处理【const<常量定义>;】
}

void varDefi(types ic, int block) {     // 处理一个变量定义  sym进时为IDENT后第一个符号，出时为变量名后的第一个符号
    if (sym == LBRACK) {    // 处理数组
        getsymbol();
        if (sym != NUMBER)
            error(16);
        getsymbol();
        if (sym != RBRACK)
            error(17);
        enterArray(ic, block);   // 数组进表
        getsymbol();
    }
    else {
        enter(VARIABLE, ic, block);      // 变量进表
    }
}
void varsDefi(types ic, int block) {      // 处理一行变量定义    sym进时为IDENT后第一个符号，出时为分号后第一个符号
    if (sym == LBRACK) {    // 处理数组
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
        getsymbol();
    }
    else if (sym == COMMA) {
        varDefi(ic, block);
        do {
            getsymbol();
            if (sym != IDENT)
                error(10);
            getsymbol();
            varDefi(ic, block);
        } while (sym == COMMA);
        if (sym != SEMICOLON)
            error(15);
        getsymbol();
    }
    else if (sym == SEMICOLON) {
        varDefi(ic, block);
        getsymbol();
    }
}

void parameter(int block) {  // 处理一个参数   sym进时为(后第一个符号，出时为IDENT后第一个符号
    types tp;
    string b;
    if (sym == INT) {
        b = "int";
        tp = INTTP;
    }
    else if (sym == CHAR) {
        b = "char";
        tp = CHARTP;
    }
    else
        error(12);
    getsymbol();
    if (sym != IDENT)
        error(10);
    enter(VARIABLE, tp, block); // 参数进tab
    genQuad("para", b, id,"");
    getsymbol();
}
void parameters(int block) {  // 处理<参数表>    sym进时为(，出时为IDENT后第一个非,的符号
    if (sym == RPAREN)
        return;
    parameter(block);
    while (sym == COMMA) {
        getsymbol();
        parameter(block);
    }
}

quad_item call(int position) {   // 处理函数调用语句，参数为函数名在tab表中的位置
    quad_item x;
    stringstream ss;
    int block = tab[position].block;                // 当前函数区块编号
    int last = ftab[tab[position].ref].lastpara;    // 最后一个参数在符号表中的位置
    int index = position;   // 当前已读到的符号表中的项
    if (sym == LPAREN) {    // 有实参传入
        vector<quad_item> paras;    // 保存push 参数的中间代码，读完所有参数后在call 函数前一起输出（避免嵌套push出错）
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
        if (sym != RPAREN)
            error(21);
        for (size_t i = 0; i < paras.size(); i++) {
            genQuad("push", paras[i].name);
        }
        getsymbol();
    }
    else {  // 无实参传入
        if (index != last)
            error(20);
    }
    // 实参加载完毕，生成函数调用指令
    genQuad("call", tab[position].name);
    if (tab[position].type != VOIDTP) {
        ss << "$" << (++pt_r);
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
        if (position == -1)
            error(22);
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
            if (sym == LBRACK) {    // 该变量为数组
                if (item.type != ARRAYTP)
                    error(23);
                getsymbol();
                y = expression();   // 返回数组下标
                if (sym != RBRACK)
                    error(17);
                // 根据数组首地址item.adr和下标值找到该元素的地址，并取值到栈
                ss << "$" << (++pt_r);
                x.name = ss.str();
                x.type = atab[item.ref].eltype;
                genQuad(x.name, "=", item.name, "[", y.name, "]");
                getsymbol();
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
        getsymbol();
    }
    else
        error(25);
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
        ss << "$" << (++pt_r);
        x.name = ss.str();
        ss.str("");     // 重置 相当于.clear
        if (md == MULT) {
            // 生成乘法指令
            genQuad(x.name, "=", y.name, "*", z.name);
        }
        else {
            // 生成除法指令
            genQuad(x.name, "=", y.name, "/", z.name);
        }
        y = x;
        y.type = INTTP;     // 参与运算后的结果必为int类型
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
        // 生成取相反数指令
        ss << "$" << (++pt_r);
        x.name = ss.str();
        ss.str("");     // 重置 相当于.clear
        genQuad(x.name, "=", "0", "-", y.name);
        y = x;
        y.type = INTTP;
    }
    while (sym == PLUS || sym == MINUS) {
        pm = sym; 
        getsymbol();
        z = term();
        ss << "$" << (++pt_r);
        x.name = ss.str();
        ss.str("");     // 重置 相当于.clear
        if (pm == PLUS) {
            // 生成加法指令
            genQuad(x.name, "=", y.name, "+", z.name);
        }
        else {
            // 生成减法指令
            genQuad(x.name, "=", y.name, "-", z.name);
        }
        y = x;
        y.type = INTTP;
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
    switch (sym)
    {
    case IF:    // 识别到<条件语句>
        int else_begin,if_end;
        getsymbol();
        if (sym != LPAREN)
            error(26);
        getsymbol();
        condition();
        else_begin = ++pt_label;
        genQuad("BZ", genLabel(else_begin));  // 条件不满足，则跳转至else_begin处
        if (sym != RPAREN)
            error(21);
        getsymbol();
        statement();
        if_end = ++pt_label;
        genQuad("GOTO", genLabel(if_end));    // then执行完毕，跳转至if_end处
        if (sym != ELSE)
            error(27);
        getsymbol();
        genQuad(genLabel(else_begin), ":");   // else_begin:
        statement();
        genQuad(genLabel(if_end), ":");       // if_end:
        break;
    case PRINTF:
        getsymbol();
        if (sym != LPAREN)
            error(26);
        getsymbol();
        if (sym == STRING) {
            genQuad("printf", "\""+str+ "\"");
            stab.push_back(str);
            getsymbol();
            if (sym == RPAREN) {
                getsymbol();
            }
            else if (sym == COMMA) {
                getsymbol();
                y = expression();
                genQuad("printf", y.name);
                quad_code[quad_code.size()-1].b.type = y.type;  // 设置四元式项的type，用以区分char和int
                if (sym != RPAREN)
                    error(21);
                getsymbol();
            }
            else
                error(21);
        }
        else {
            y = expression();
            genQuad("printf", y.name);
            quad_code[quad_code.size() - 1].b.type = y.type;
            if (sym != RPAREN)
                error(21);
            getsymbol();
        }
        if (sym != SEMICOLON)
            error(15);
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
            genQuad("ret", y.name);
            if (sym != RPAREN)
                error(21);
            getsymbol();
        }
        else {
            if (tab[i].type != VOIDTP)
                error(9);
            genQuad("ret");
        }
        if (sym != SEMICOLON)
            error(7);
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
            int position = findTab(id);
            if (position == -1)
                error(22);
            if (tab[position].kind != VARIABLE)
                error(28);
            genQuad("scanf", tab[position].name);
            quad_code[quad_code.size() - 1].b.type = tab[position].type;  // 设置四元式项的type，用以区分char和int
            getsymbol();
        } while (sym == COMMA);
        if (sym != RPAREN)
            error(21);
        getsymbol();
        if (sym != SEMICOLON)
            error(15);
        getsymbol();
        break;
    case SWITCH:    // 识别到<情况语句>
        int switch_cmp,switch_end,default_begin;   // 进行比较、switch语句结束、default开始的位置
        getsymbol();
        if (sym != LPAREN)
            error(26);
        getsymbol();
        x = expression();
        if (sym != RPAREN)
            error(21);
        getsymbol();
        if (sym != LBRACE)
            error(38);
        switch_cmp = ++pt_label;
        switch_end = ++pt_label;
        genQuad("GOTO", genLabel(switch_cmp));       // 中间代码跳转至case_cmp处
        getsymbol();
        do {
            if (sym != CASE)
                error(29);
            getsymbol();
            if (sym == NUMBER){
                if (x.type != INTTP)
                    error(43);
            }
            else if (sym == PLUS || sym == MINUS) {
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
        getsymbol();
        while_head = ++pt_label;
        while_end = ++pt_label;
        genQuad(genLabel(while_head), ":");
        condition();
        if (sym != RPAREN)
            error(21);
        getsymbol();
        genQuad("BZ", genLabel(while_end));
        statement();
        genQuad("GOTO", genLabel(while_head));
        genQuad(genLabel(while_end), ":");
        break;
    case IDENT:   
        int position;
        position = findTab(id);
        if (position == -1)
            error(22);
        if (tab[position].kind == FUNCTION) {     // 识别到<有/无返回值函数调用>
            getsymbol();
            call(position);
        }
        else if (tab[position].kind == VARIABLE) {  // 识别到<赋值语句>
            if (tab[position].type == ARRAYTP) {    // 对数组某元素赋值
                getsymbol();
                if (sym != LBRACK)
                    error(33);
                getsymbol();
                y = expression();   // 获取数组下标的值
                // 判断下标范围
                if (sym != RBRACK)
                    error(17);
                getsymbol();
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
        else
            error(34);
        if (sym != SEMICOLON)
            error(15);
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
        if (sym != IDENT)
            error(10);
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
    pt_r = -1;      // 重置临时变量名
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
        getsymbol();
        parameters(pt_block);   // 处理参数
        ftab[ftab.size()-1].lastpara = tab.size() - 1;    //  记录该函数最后一个参数在符号表中的位置
        if (sym != RPAREN)
            error(21);
        getsymbol();
        if (sym != LBRACE)
            error(38);
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
    if (quad_code[quad_code.size() - 1].a.name != "ret")  // 函数最后一句若不是return，则加上
        genQuad("ret");
    if (sym != RBRACE)
        error(35);
    ftab[ftab.size() - 1].tsize = pt_r + 1;
    getsymbol();
}
void fun_main() {
    pt_r = -1;      // 重置临时变量名
    getsymbol();
    if (sym != LPAREN)
        error(26);
    getsymbol();
    if (sym != RPAREN)
        error(21);
    getsymbol();
    if (sym != LBRACE)
        error(38);
    genQuad("void", id, "(", ")");
    pt_block++;
    enter(FUNCTION, VOIDTP, pt_block);
    ftab[ftab.size() - 1].lastpara = tab.size()-1;
    getsymbol();
    comState(pt_block);   // 处理复合语句
    if (sym != RBRACE)
        error(35);
    ftab[ftab.size() - 1].tsize = pt_r + 1;
    getsymbol();
}
void program() {    // 处理<程序>
    if (sym == CONST)   // 识别到全局常量定义
        constsDefi(0);
    offset = 0;
    ftab_item nf;
    ftab.push_back(nf);
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
            error(37);
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
    } while (sym == INT || sym == CHAR || sym == VOID);
    if (sym != END)
        error(42);
}

/*void optimize(){
    struct dag_node {
        string op;
        int x, y;       // 左右子结点序号
    };
    int pt_node = 0;
    map<int, dag_node> dag;     // DAG图
    map<string, int> dag_tab;   // 结点表
    for (size_t i = 0; i < quad_code.size(); i++) {
        quadruple code = quad_code[i];
        if (code.b.name == ":") {   // 标签划分基本块

            dag.clear();
            dag_tab.clear();
        }
        int x, y, z;
        if (code.b.name == "=") {   // 赋值语句
            map<string, int>::iterator iter_tab;
            iter_tab = dag_tab.find(code.c.name);
            if (iter_tab == dag_tab.end()) {    // 结点表未找到
                x = ++pt_node;
                dag[x] = { code.c.name,0,0 };
                dag_tab[code.c.name] = x;
            }
            else
                x = iter_tab->second;
            if (code.d.name == "") {    // 语句格式为：z = x
                z = x;
                // 无需优化子表达式
            }
            else{   // z = x op y
                iter_tab = dag_tab.find(code.e.name);
                if (iter_tab == dag_tab.end()) {
                    y = ++pt_node;
                    dag[y] = { code.e.name,0,0 };
                    dag_tab[code.e.name] = y;
                }
                else
                    y = iter_tab->second;
                map<int, dag_node>::iterator iter_dag;
                for (iter_dag = dag.begin(); iter_dag != dag.end(); iter_dag++) {
                    dag_node node = iter_dag->second;
                    if (node.op == code.d.name&&node.x == x&&node.y == y)
                        break;
                }
                if (iter_dag == dag.end()) {   // DAG图中无该结点
                    z = ++pt_node;
                    dag[z] = { code.d.name,x,y };
                }
                else {
                    z = iter_dag->first;
                    // DAG图中已存在的结点，说明该子表达式之前计算过，无需再算，但问题是没保存啊！怎么用？？？

                }

            }
            iter_tab = dag_tab.find(code.a.name);
            if (iter_tab == dag_tab.end())   
                dag_tab[code.a.name] = z;
            else
                iter_tab->second = z;
        }
    }
}*/

void transform() {
    fout << endl << endl << "MIPS32汇编代码：" << endl;
    fout << ".data" << endl;
    for (size_t i = 0; i < stab.size(); i++) {
        fout << ".string" << i << ": .asciiz \"" << stab[i] << "\"" << endl;
    }
    fout << ".align 2" << endl; // 内存按word对齐
    fout << ".begin:" << endl;
    fout << ".text" << endl;
    fout << "la\t$s0, .begin" << endl;                          // $s0存全局数据区基址
    fout << "addi\t$s1, $s0, " << (ftab[0].size*4) << endl;     // $s1存当前数据区基址
    fout << "add\t$s2, $0, $sp" << endl;                        // $s2存当前临时数据区基址
    fout << "j main" << endl;
    pc = 0;
    pt_block = 0;
    int pt_stab = -1;
    offset = 0;
    quadruple code;
    string base[] = { "$s1","$s0" };
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
                if (code.c.name == "RET") {  // 格式: $ = RET
                    fout << "sw\t$v0, " << stoi(code.a.name.substr(1))*4 << "($s2)" << endl;
                    pc++;
                    continue;
                }
                tab_item a = tab[findTab(code.a.name)];
                int a_global = (a.block == 0) ? 1 : 0;
                if (code.c.name[0] == '$') {         // 格式: x = $
                    fout << "lw\t$t1, " << stoi(code.c.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0]=='-') {  // 格式: x = 1
                    fout << "addi\t$t1, $0, " << code.c.name << endl;
                }
                else {                          // 格式: x = y
                    tab_item c = tab[findTab(code.c.name)];
                    int c_global = (c.block == 0) ? 1 : 0;
                    fout << "lw\t$t1, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
                }
                fout << "sw\t$t1, " << (a.adr * 4) << "(" << base[a_global] << ")" << endl;  // 保存临时变量的值
            }
            else if (code.f.name == "") {    // 格式: $ = c op e
                if (code.c.name[0] == '$') {         // c为$
                    fout << "lw\t$t1, " << stoi(code.c.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // c为数字
                    fout << "addi\t$t1, $0, " << code.c.name << endl;
                }
                else {                          // c为变量
                    tab_item c = tab[findTab(code.c.name)];
                    int c_global = (c.block == 0) ? 1 : 0;
                    fout << "lw\t$t1, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
                }
                if (code.e.name[0] == '$') {         // e为$
                    fout << "lw\t$t2, " << stoi(code.e.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // e为数字
                    fout << "addi\t$t2, $0, " << code.e.name << endl;
                }
                else {                          // e为变量
                    tab_item e = tab[findTab(code.e.name)];
                    int e_global = (e.block == 0) ? 1 : 0;
                    fout << "lw\t$t2, " << (e.adr * 4) << "(" << base[e_global] << ")" << endl;
                }
                if (code.d.name == "+")
                    fout << "add\t$t1, $t1, $t2" << endl;
                else if (code.d.name == "-")
                    fout << "sub\t$t1, $t1, $t2" << endl;
                else if (code.d.name == "*") {
                    fout << "mult\t" << "$t1, $t2" << endl;
                    fout << "mflo\t$t1" << endl;
                }
                else if (code.d.name == "/") {
                    fout << "div\t$t1, $t1, $t2" << endl;
                    fout << "mflo\t" << "$t1" << endl;
                }
                fout << "sw\t$t1, " << stoi(code.a.name.substr(1)) * 4 << "($s2)" << endl;  // 保存临时变量的值
            }
            else {  // $ = 数组名 [ 因子 ]
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "addi\t$t1, " << base[c_global] << ", " << (c.adr * 4) << endl;     // 计算该数组的基址
                if (code.e.name[0] == '$') {         // 因子e为$
                    fout << "lw\t$t2, " << stoi(code.e.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
                    fout << "sll\t$t2, $t2, 2" << endl;     // 数组下标乘4
                    fout << "add\t$t1, $t1, $t2" << endl;   // 计算该元素的地址
                    fout << "lw\t$t1, ($t1)" << endl;
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // e为数字
                    fout << "lw\t$t1, " << (stoi(code.e.name) * 4) << "($t1)" << endl;
                }
                else {  // e为变量
                    tab_item e = tab[findTab(code.e.name)];
                    int e_global = (e.block == 0) ? 1 : 0;
                    fout << "lw\t$t2, " << (e.adr * 4) << "(" << base[e_global] << ")" << endl; // 获取数组下标
                    fout << "sll\t$t2, $t2, 2" << endl;     // 数组下标乘4
                    fout << "add\t$t1, $t1, $t2" << endl;   // 计算该元素的地址
                    fout << "lw\t$t1, ($t1)" << endl;
                }
                fout << "sw\t$t1, " << stoi(code.a.name.substr(1)) * 4 << "($s2)" << endl;  // 保存临时变量的值
            }
        }
        else if (code.e.name == "=") {   // 数组名 [ 因子 ] = 因子
            tab_item a = tab[findTab(code.a.name)];
            int a_global = (a.block == 0) ? 1 : 0;
            fout << "addi\t$t1, " << base[a_global] << ", " << (a.adr * 4) << endl;     // 计算该数组的基址
            if (code.c.name[0] == '$') {         // c为$
                fout << "lw\t$t2, " << stoi(code.c.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
                fout << "sll\t$t2, $t2, 2" << endl;
                fout << "add\t$t1, $t1, $t2" << endl;   // 计算该元素的地址
            }
            else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // c为数字
                fout << "addi\t$t1, $t1, " << (stoi(code.c.name) * 4) << endl;
            }
            else {      // c为变量
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "lw\t$t2, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl; // 获取数组下标
                fout << "sll\t$t2, $t2, 2" << endl;     // 数组下标乘4
                fout << "add\t$t1, $t1, $t2" << endl;   // 计算该元素的地址
            }
            if (code.f.name[0] == '$') {         // 因子f为$
                fout << "lw\t$t2, " << stoi(code.f.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
            }
            else if (isdigit(code.f.name[0]) || code.f.name[0] == '-') {  // f为数
                fout << "addi\t$t2, $0, " << code.f.name << endl;
            }
            else {                          // f为变量
                tab_item f = tab[findTab(code.f.name)];
                int f_global = (f.block == 0) ? 1 : 0;
                fout << "lw\t$t2, " << (f.adr * 4) << "(" << base[f_global] << ")" << endl;
            }
            fout << "sw\t$t2, ($t1)" << endl;
        }
        else if (code.a.name == "para") {
            // 参数已在符号表里，无需进行操作
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
            if (code.a.name[0] == '$')
                fout << "lw\t$t1, " << stoi(code.a.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
            else if (isdigit(code.a.name[0]))
                fout << "addi\t$t1, $0, " << code.a.name << endl;
            else {
                tab_item a = tab[findTab(code.a.name)];
                int a_global = (a.block == 0) ? 1 : 0;
                fout << "lw\t$t1, " << (a.adr * 4) << "(" << base[a_global] << ")" << endl;
            }
            if (code.c.name[0] == '$')
                fout << "lw\t$t2, " << stoi(code.c.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
            else if (isdigit(code.c.name[0])||code.c.name[0]=='-')
                fout << "addi\t$t2, $0, " << code.c.name << endl;
            else {
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "lw\t$t2, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
            }
            fout << instr << "\t$t1, $t1, $t2" << endl;
            code = quad_code[++pc];
            if (code.a.name == "BZ") {
                fout << "beq\t$t1, $0, " << code.b.name << endl;
            }
            else if (code.a.name == "BNZ") {
                fout << "bne\t$t1, $0, " << code.b.name << endl;
            }
        }
        else if (code.a.name == "GOTO") {
            fout << "j\t" << code.b.name << endl;
        }
        else if (code.a.name == "push") {
            fout << "addi\t$t1, $s1, " << (ftab[pt_block].size * 4) << endl;    // 计算下个函数体的基址
            if (code.b.name[0] == '$') {   // push $
                fout << "lw\t$t2, " << stoi(code.b.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
                fout << "sw\t$t2, " << (++offset * 4) << "($t1)" << endl;  // 参数只能为int或char，故大小每次加4即可
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {  // push 数字
                fout << "addi\t$t2, $0, " << code.b.name << endl;
                fout << "sw\t$t2, " << (++offset * 4) << "($t1)" << endl;
            }
            else {  // push 变量  
                tab_item b = tab[findTab(code.b.name)];
                int b_global = (b.block == 0) ? 1 : 0;
                fout << "lw\t$t2, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
                fout << "sw\t$t2, " << (++offset * 4) << "($t1)" << endl;
            }
        }
        else if (code.a.name == "printf") {
            if (code.b.name[0] == '\"') {    // 输出字符串
                fout << "la\t$a0, .string" << ++pt_stab << endl;    // 加载字符串地址到$a0
                fout << "addi\t$v0, $0, 4" << endl;
                fout << "syscall" << endl;
            }
            else if (code.b.name[0] == '$') {
                if (code.b.type == CHARTP) {     // char变量需要输出字符
                    fout << "lw\t$a0, " << stoi(code.b.name.substr(1)) * 4 << "($s2)" << endl;  // 将char值取到$a0
                    fout << "addi\t$v0, $0, 11" << endl;
                }
                else {
                    fout << "lw\t$t1, " << stoi(code.b.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
                    fout << "add\t$a0, $0, $t1" << endl;
                    fout << "addi\t$v0, $0, 1" << endl;
                }
                fout << "syscall" << endl;
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
                fout << "syscall" << endl;
            }
        }
        else if (code.a.name == "scanf") {
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
        else if (code.a.name == "call") {
            offset = 0;
            /* 跳转进其他函数前 */
            fout << "sw\t$ra, ($s1)" << endl;                                   // 保存当前函数的返回地址
            fout << "addi\t$s1, $s1, " << (ftab[pt_block].size * 4) << endl;    // 更新$s1基址位置(加当前函数数据区大小)
            fout << "addi\t$s2, $s2, " << (ftab[pt_block].tsize * 4) << endl;   // 更新$s2基址位置(加当前函数临时数据区大小)
            fout << "jal\t" << code.b.name << endl;
            /* 从其他函数跳回后 */
            fout << "addi\t$s1, $s1, " << -(ftab[pt_block].size * 4) << endl;   // 更新$s1基址位置(减当前函数数据区大小)
            fout << "addi\t$s2, $s2, " << -(ftab[pt_block].tsize * 4) << endl;   // 更新$s2基址位置(减当前函数临时数据区大小)
            fout << "lw\t$ra, ($s1)" << endl;                                   // 将保存的该函数返回地址赋给$ra
        }
        else if (code.a.name == "ret") {
            if (code.b.name == "") {
                if (pt_block == ftab.size() - 1) {  // main函数里的return
                    fout << "j\t.end" << endl;
                }
                else
                    fout << "jr\t$ra" << endl;
            }
            else if (code.b.name[0] == '$') {
                fout << "lw\t$t1, " << stoi(code.b.name.substr(1)) * 4 << "($s2)" << endl;  // 取出临时变量的值
                fout << "add\t$v0, $0, $t1" << endl;
                fout << "jr\t$ra" << endl;
            }
            else if (isdigit(code.b.name[0])) {
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
    fout << "源码：" << endl;
    getsymbol();
    program();
    //printTabs();
    //optimize();
    transform();
    fin.close();
    fout.close();

}

