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
    msg[1] = "����int����"; msg[2] = "ȱ��'"; msg[3] = "�ַ������ݳ���"; msg[4] = "!ֻ����=����";
    msg[5] = "�޷�ʶ�����"; msg[6] = "��ʶ���ظ�����"; msg[7] = "ȱ��(��;"; msg[8] = "void���������з���ֵ���()";
    msg[9] = "����ȱ�ٷ���ֵ������ֵӦ��()�ڣ�"; msg[10] = "ȱ�ٱ�ʶ��"; msg[11] = "ȱ��="; msg[12] = "ȱ��int��char";
    msg[13] = "ȱ��const"; msg[14] = "�����������ͳ���"; msg[15] = "ȱ��;"; msg[16] = "ȱ���޷�������";
    msg[17] = "ȱ��]"; msg[18] = "�޲����ĺ������ò��ܸ�()"; msg[19] = "�����������"; msg[20] = "�����������"; 
    msg[21] = "ȱ��)"; msg[22] = "��ʶ��δ����"; msg[23] = "[ʹ�ó���"; msg[24] = "���õĺ����޷���ֵ";
    msg[25] = "�������ͳ���"; msg[26] = "ȱ��("; msg[27] = "ȱ��else"; msg[28] = "��ʶ���Ǳ���"; 
    msg[29] = "ȱ��case"; msg[30] = "case��ȱ�ٳ���"; msg[31] = "case����ظ�����"; msg[32] = "ȱ��:";
    msg[33] = "ȱ��["; msg[34] = "���ܶԳ������и�ֵ"; msg[35] = "ȱ��}"; msg[36] = "�޷�ʶ�����"; 
    msg[37] = "�����������"; msg[38] = "ȱ��{"; msg[39] = "�����������"; msg[40] = "main�������ͱ���Ϊvoid";
    msg[41] = "ȱ��main����"; msg[42] = "main�������ж����ַ�"; msg[43] = "case�������ʹ���"; msg[44] = "��ʶ���������";
    msg[45] = "��0����������0��ͷ";
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
    while (isspace(ch)) {   // �ж��Ƿ�Ϊ�հ��ַ�' '��'\t'��'\r'��'\n'
        getch();
    }
    if (ch == NULL) {
        sym = END;
        return;
    }
    if (ch == '_' || isalpha(ch)) { // ��Ӧ�ķ��е�<��ĸ>
        id = "";     // ���ñ�ʶ����
        do {
            id += ch;
            getch();
        } while (ch == '_' || isalnum(ch));
        transform(id.begin(), id.end(), id.begin(), tolower);   // ��stringתΪȫСд
                                                                /* ��⵱ǰid�Ƿ�Ϊ�����֣����ֲ��� */
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
        if (l > r) // û�ҵ�
            sym = IDENT;        // ��ǰ����Ϊ��ʶ��
        else
            sym = symbol(m);    // ����intֵǿ��ת��Ϊö��ֵ
    }
    else if (isdigit(ch)) {   // ��Ӧ����+/-��<����>
        value = 0;      // ����intֵ
        sym = NUMBER;   // ��������Ϊnumber
        if (ch == '0') {    
            getch();
            if (isdigit(ch))    // ������0��ͷ
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
    else if (ch == '\'') {  // ��Ӧ�ķ��еģ��ַ���  
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
    else if (ch == '\"') {  // ��Ӧ�ķ��е�<�ַ���>
        str = "";   // �����ַ���ֵ
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
    ss << ".label" << i;    // ǰ��.��ֹ�뺯������
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

void enter(kinds kind, types type, int block) {   // ��¼�����������ı���
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
    case CONSTANT:  // ���ڳ�����ֻ�轫value����adr
        nt.adr = value;
        break;
    case VARIABLE:  // ���ڷ������������Ҫ��¼��ǰ�����Ե�ַ
        nt.adr = ++offset;       
        break;
    case FUNCTION:  // ���ں�������¼�亯����Ϣ��λ�ú���ڵ�ַ
        nt.ref = ftab.size();  // ��¼��ftab�е�λ��
        ftab_item nf;
        ftab.push_back(nf);
        break;
    }
    tab.push_back(nt);
}
void enterArray(types eltype, int block) {  // ��¼�������
    tab_item nt;
    nt.adr = ++offset;
    nt.name = id;
    nt.kind = VARIABLE;
    nt.type = ARRAYTP;
    nt.block = block;
    nt.ref = atab.size();  // atabָ���µ�һ�����¼��tab��
    tab.push_back(nt);
    atab_item na;
    na.length = value;   // ��¼���鳤��
    na.eltype = eltype;  // ��¼Ԫ������
    atab.push_back(na);
    offset += value - 1;
}
int findTab(string name) { // ����name��tab�е�λ��,û����-1
    for (int i = tab.size()-1; i >= 0; i--) {
        if (tab[i].block != pt_block)
            if (tab[i].block != 0)
                continue;
        if (tab[i].name == name)
            return i;
    }
    return -1;
}

void constDefi(types ic, int block) {    // ����һ����������    sym����ʱΪIDENT����ȥʱΪ�������һ������
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
    enter(CONSTANT, ic, block);  // ��������
    getsymbol();
}
void constsDefi(int block) {      // ����<����˵��>   sym��ʱΪCONST����ʱΪ�ֺź��һ������
    if (sym != CONST)
        error(13);
    getsymbol();
    if (sym != INT&&sym != CHAR)
        error(14);
    types ic = (sym == INT) ? INTTP : CHARTP;    // ��¼��ǰ��������Ϊint����char
    getsymbol();
    constDefi(ic, block);   // ����һ����������
    while (sym == COMMA) {  // ����ͬ�ೣ������
        getsymbol();
        constDefi(ic, block);
    }
    if (sym != SEMICOLON)
        error(15);
    getsymbol();
    if (sym == CONST)
        constsDefi(block);  // �ݹ���ã���������const<��������>;��
}

void varDefi(types ic, int block) {     // ����һ����������  sym��ʱΪIDENT���һ�����ţ���ʱΪ��������ĵ�һ������
    if (sym == LBRACK) {    // ��������
        getsymbol();
        if (sym != NUMBER)
            error(16);
        getsymbol();
        if (sym != RBRACK)
            error(17);
        enterArray(ic, block);   // �������
        getsymbol();
    }
    else {
        enter(VARIABLE, ic, block);      // ��������
    }
}
void varsDefi(types ic, int block) {      // ����һ�б�������    sym��ʱΪIDENT���һ�����ţ���ʱΪ�ֺź��һ������
    if (sym == LBRACK) {    // ��������
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

void parameter(int block) {  // ����һ������   sym��ʱΪ(���һ�����ţ���ʱΪIDENT���һ������
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
    enter(VARIABLE, tp, block); // ������tab
    genQuad("para", b, id,"");
    getsymbol();
}
void parameters(int block) {  // ����<������>    sym��ʱΪ(����ʱΪIDENT���һ����,�ķ���
    if (sym == RPAREN)
        return;
    parameter(block);
    while (sym == COMMA) {
        getsymbol();
        parameter(block);
    }
}

quad_item call(int position) {   // ������������䣬����Ϊ��������tab���е�λ��
    quad_item x;
    stringstream ss;
    int block = tab[position].block;                // ��ǰ����������
    int last = ftab[tab[position].ref].lastpara;    // ���һ�������ڷ��ű��е�λ��
    int index = position;   // ��ǰ�Ѷ����ķ��ű��е���
    if (sym == LPAREN) {    // ��ʵ�δ���
        vector<quad_item> paras;    // ����push �������м���룬�������в�������call ����ǰһ�����������Ƕ��push����
        if (index == last)
            error(18);
        do {
            getsymbol();
            if (++index > last)
                error(19);
            x = expression();   // ��ȡ���ʽֵ
            paras.push_back(x);     
            // ���ֵ�����ͣ������ɼ���ָ��
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
    else {  // ��ʵ�δ���
        if (index != last)
            error(20);
    }
    // ʵ�μ�����ϣ����ɺ�������ָ��
    genQuad("call", tab[position].name);
    if (tab[position].type != VOIDTP) {
        ss << "$" << (++pt_r);
        x.name = ss.str();
        x.type = tab[position].type;
        genQuad(x.name, "=", "RET");
    }
    return x;
}

quad_item factor() {	// ����<����>
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
            // ���س���ֵitem.adr
            ss << item.adr;
            x.name = ss.str();
            x.type = item.type;
            break;
        case VARIABLE:
            // ȡitem.adr
            if (sym == LBRACK) {    // �ñ���Ϊ����
                if (item.type != ARRAYTP)
                    error(23);
                getsymbol();
                y = expression();   // ���������±�
                if (sym != RBRACK)
                    error(17);
                // ���������׵�ַitem.adr���±�ֵ�ҵ���Ԫ�صĵ�ַ����ȡֵ��ջ
                ss << "$" << (++pt_r);
                x.name = ss.str();
                x.type = atab[item.ref].eltype;
                genQuad(x.name, "=", item.name, "[", y.name, "]");
                getsymbol();
            }
            else {
                // ���ݱ�����ַitem.adrȡֵ��ջ
                x.name = item.name;
                x.type = item.type;
            }
            break;
        case FUNCTION:
            if (item.type == VOIDTP)
                error(24);
            x = call(position); // ��������
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
quad_item term() {	// ����<��>
    quad_item x, y, z;
    stringstream ss;
    y = factor();
    while (sym == MULT||sym==DIVI) {
        symbol md = sym;    // ��¼�˳�����
        getsymbol();
        z = factor();
        ss << "$" << (++pt_r);
        x.name = ss.str();
        ss.str("");     // ���� �൱��.clear
        if (md == MULT) {
            // ���ɳ˷�ָ��
            genQuad(x.name, "=", y.name, "*", z.name);
        }
        else {
            // ���ɳ���ָ��
            genQuad(x.name, "=", y.name, "/", z.name);
        }
        y = x;
        y.type = INTTP;     // ���������Ľ����Ϊint����
    }
    return y;
}
quad_item expression() { // ����<���ʽ>
    quad_item x, y, z;
    stringstream ss;
    symbol pm = PLUS;  // ��¼�Ӽ�����
    if (sym == PLUS || sym == MINUS) {
        pm = sym;
        getsymbol();
    }
    y = term();
    if (pm == MINUS) {
        // ����ȡ�෴��ָ��
        ss << "$" << (++pt_r);
        x.name = ss.str();
        ss.str("");     // ���� �൱��.clear
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
        ss.str("");     // ���� �൱��.clear
        if (pm == PLUS) {
            // ���ɼӷ�ָ��
            genQuad(x.name, "=", y.name, "+", z.name);
        }
        else {
            // ���ɼ���ָ��
            genQuad(x.name, "=", y.name, "-", z.name);
        }
        y = x;
        y.type = INTTP;
    }
    return y;
}

void condition() {  //  ����<����>
    quad_item y, z;
    y = expression();
    if (sym == EQL || sym == NEQ || sym == LSS || sym == LEQ || sym == GTR || sym == GEQ) {
        symbol op = sym;
        getsymbol();
        z = expression();
        switch (op) {
            /*���ݹ�ϵ��������ɸ���ָ��*/
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

void statement() {  // ����<���>
    quad_item x, y, z;
    vector<int> case_value;   // ��case�ĳ���ֵ
    vector<int> case_label;   // ��case�Ŀ�ʼλ��
    switch (sym)
    {
    case IF:    // ʶ��<�������>
        int else_begin,if_end;
        getsymbol();
        if (sym != LPAREN)
            error(26);
        getsymbol();
        condition();
        else_begin = ++pt_label;
        genQuad("BZ", genLabel(else_begin));  // ���������㣬����ת��else_begin��
        if (sym != RPAREN)
            error(21);
        getsymbol();
        statement();
        if_end = ++pt_label;
        genQuad("GOTO", genLabel(if_end));    // thenִ����ϣ���ת��if_end��
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
                quad_code[quad_code.size()-1].b.type = y.type;  // ������Ԫʽ���type����������char��int
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
            quad_code[quad_code.size() - 1].b.type = tab[position].type;  // ������Ԫʽ���type����������char��int
            getsymbol();
        } while (sym == COMMA);
        if (sym != RPAREN)
            error(21);
        getsymbol();
        if (sym != SEMICOLON)
            error(15);
        getsymbol();
        break;
    case SWITCH:    // ʶ��<������>
        int switch_cmp,switch_end,default_begin;   // ���бȽϡ�switch��������default��ʼ��λ��
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
        genQuad("GOTO", genLabel(switch_cmp));       // �м������ת��case_cmp��
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
            case_value.push_back(value);       // ��¼����ֵ
            case_label.push_back(++pt_label);  // ��¼��ǩ�±�
            getsymbol();
            if (sym != COLON)
                error(32);
            getsymbol();
            genQuad(genLabel(case_label[case_label.size()-1]), ":");    // ��case�����ʼλ��
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
            default_begin = switch_end;     // ��Դ������default������Ϊswitch����λ��
        if (sym != RBRACE)
            error(35);
        getsymbol();
        genQuad(genLabel(switch_cmp), ":");
        for (size_t i = 0; i < case_value.size(); i++) {
            stringstream ss;
            ss << case_value[i];
            genQuad(x.name, "==", ss.str());    
            genQuad("BNZ", genLabel(case_label[i]));   // ��������������ת����Ӧλ��
        } 
        genQuad("GOTO", genLabel(default_begin));     // ��������һ����������ת��default��
        genQuad(genLabel(switch_end), ":");           // switch������λ��
        break;
    case WHILE:     // ʶ��<ѭ�����>
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
        if (tab[position].kind == FUNCTION) {     // ʶ��<��/�޷���ֵ��������>
            getsymbol();
            call(position);
        }
        else if (tab[position].kind == VARIABLE) {  // ʶ��<��ֵ���>
            if (tab[position].type == ARRAYTP) {    // ������ĳԪ�ظ�ֵ
                getsymbol();
                if (sym != LBRACK)
                    error(33);
                getsymbol();
                y = expression();   // ��ȡ�����±��ֵ
                // �ж��±귶Χ
                if (sym != RBRACK)
                    error(17);
                getsymbol();
                if (sym != BECOME)
                    error(11);
                getsymbol();
                z = expression();   // ��ȡ��ֵ���ұߵ�ֵ
                // ���ɸ�ֵ����
                genQuad(tab[position].name, "[", y.name, "]", "=", z.name);
            }
            else {      // �Է����������ֵ
                getsymbol();
                if (sym != BECOME)
                    error(11);
                getsymbol();
                y = expression();    
                // ���ɸ�ֵ����
                genQuad(tab[position].name, "=", y.name);
            }
        }
        else
            error(34);
        if (sym != SEMICOLON)
            error(15);
        getsymbol();
        break;
    case LBRACE:    // ʶ��<�����>
        getsymbol();
        if (sym == RBRACE) {  // ����� {}
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
    case SEMICOLON: // ʶ�𵽿���� ;
        getsymbol();
        break;
    default:
        error(36);
        break;
    }
}
void comState(int block) {  // ����<�������>     sym��ʱΪ{���һ�����ţ���ʱӦΪ}
    if (sym == CONST)   // ʶ�𵽾ֲ���������
        constsDefi(block);
    while (sym == INT || sym == CHAR) {
        types ic = (sym == INT) ? INTTP : CHARTP;
        getsymbol();
        if (sym != IDENT)
            error(10);
        getsymbol();
        if (sym == LBRACK || sym == COMMA || sym == SEMICOLON)  // ʶ��[ , ; ˵���Ǿֲ���������
            varsDefi(ic, block);
        else
            error(37);
    }
    ftab[ftab.size()-1].size = 1 + offset;  // ��¼�ú����ľֲ�������(���ص�ַ+�����;ֲ�����)��С  
    while (sym == LBRACE || sym == IF || sym == WHILE || sym == SWITCH || sym == RETURN ||
        sym == SCANF || sym == PRINTF || sym == IDENT || sym == SEMICOLON)
        statement();
}

void funDefi(types icv) {
    pt_block++;     // �ú���������
    pt_r = -1;      // ������ʱ������
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
    if (sym == LPAREN) {    // �в���
        enter(FUNCTION, icv, 0);
        getsymbol();
        parameters(pt_block);   // �������
        ftab[ftab.size()-1].lastpara = tab.size() - 1;    //  ��¼�ú������һ�������ڷ��ű��е�λ��
        if (sym != RPAREN)
            error(21);
        getsymbol();
        if (sym != LBRACE)
            error(38);
        getsymbol();
    }
    else if (sym == LBRACE) {  //  �޲���
        enter(FUNCTION, icv, 0);
        ftab[ftab.size() - 1].lastpara = tab.size() - 1;
        getsymbol();
    }
    else
        error(38);
    comState(pt_block);   // ���������
    if (quad_code[quad_code.size() - 1].a.name != "ret")  // �������һ��������return�������
        genQuad("ret");
    if (sym != RBRACE)
        error(35);
    ftab[ftab.size() - 1].tsize = pt_r + 1;
    getsymbol();
}
void fun_main() {
    pt_r = -1;      // ������ʱ������
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
    comState(pt_block);   // ���������
    if (sym != RBRACE)
        error(35);
    ftab[ftab.size() - 1].tsize = pt_r + 1;
    getsymbol();
}
void program() {    // ����<����>
    if (sym == CONST)   // ʶ��ȫ�ֳ�������
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
        if (sym == LBRACK || sym == COMMA || sym == SEMICOLON) {  // ʶ��[ , ; ˵����ȫ�ֱ�������
            varsDefi(ic, 0);
            ftab[0].size = 1 + offset;      // ����ȫ��������(���ص�ַ+ȫ�ֱ���)��С
        }
        else if (sym == LPAREN || sym == LBRACE) {  // ʶ��( { ˵�����з���ֵ��������
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
            offset = 0;     // ���ú���������ƫ����
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
        int x, y;       // �����ӽ�����
    };
    int pt_node = 0;
    map<int, dag_node> dag;     // DAGͼ
    map<string, int> dag_tab;   // ����
    for (size_t i = 0; i < quad_code.size(); i++) {
        quadruple code = quad_code[i];
        if (code.b.name == ":") {   // ��ǩ���ֻ�����

            dag.clear();
            dag_tab.clear();
        }
        int x, y, z;
        if (code.b.name == "=") {   // ��ֵ���
            map<string, int>::iterator iter_tab;
            iter_tab = dag_tab.find(code.c.name);
            if (iter_tab == dag_tab.end()) {    // ����δ�ҵ�
                x = ++pt_node;
                dag[x] = { code.c.name,0,0 };
                dag_tab[code.c.name] = x;
            }
            else
                x = iter_tab->second;
            if (code.d.name == "") {    // ����ʽΪ��z = x
                z = x;
                // �����Ż��ӱ��ʽ
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
                if (iter_dag == dag.end()) {   // DAGͼ���޸ý��
                    z = ++pt_node;
                    dag[z] = { code.d.name,x,y };
                }
                else {
                    z = iter_dag->first;
                    // DAGͼ���Ѵ��ڵĽ�㣬˵�����ӱ��ʽ֮ǰ��������������㣬��������û���氡����ô�ã�����

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
    fout << endl << endl << "MIPS32�����룺" << endl;
    fout << ".data" << endl;
    for (size_t i = 0; i < stab.size(); i++) {
        fout << ".string" << i << ": .asciiz \"" << stab[i] << "\"" << endl;
    }
    fout << ".align 2" << endl; // �ڴ水word����
    fout << ".begin:" << endl;
    fout << ".text" << endl;
    fout << "la\t$s0, .begin" << endl;                          // $s0��ȫ����������ַ
    fout << "addi\t$s1, $s0, " << (ftab[0].size*4) << endl;     // $s1�浱ǰ��������ַ
    fout << "add\t$s2, $0, $sp" << endl;                        // $s2�浱ǰ��ʱ��������ַ
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
            fout << code.b.name << ":" << endl;  // ���ɺ�����ǩ
            pc++;
            continue;
        }
        /* ��ԪʽΪ��ֵ��� */
        if (code.b.name == "=") {
            if (code.d.name == "") {
                if (code.c.name == "RET") {  // ��ʽ: $ = RET
                    fout << "sw\t$v0, " << stoi(code.a.name.substr(1))*4 << "($s2)" << endl;
                    pc++;
                    continue;
                }
                tab_item a = tab[findTab(code.a.name)];
                int a_global = (a.block == 0) ? 1 : 0;
                if (code.c.name[0] == '$') {         // ��ʽ: x = $
                    fout << "lw\t$t1, " << stoi(code.c.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0]=='-') {  // ��ʽ: x = 1
                    fout << "addi\t$t1, $0, " << code.c.name << endl;
                }
                else {                          // ��ʽ: x = y
                    tab_item c = tab[findTab(code.c.name)];
                    int c_global = (c.block == 0) ? 1 : 0;
                    fout << "lw\t$t1, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
                }
                fout << "sw\t$t1, " << (a.adr * 4) << "(" << base[a_global] << ")" << endl;  // ������ʱ������ֵ
            }
            else if (code.f.name == "") {    // ��ʽ: $ = c op e
                if (code.c.name[0] == '$') {         // cΪ$
                    fout << "lw\t$t1, " << stoi(code.c.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // cΪ����
                    fout << "addi\t$t1, $0, " << code.c.name << endl;
                }
                else {                          // cΪ����
                    tab_item c = tab[findTab(code.c.name)];
                    int c_global = (c.block == 0) ? 1 : 0;
                    fout << "lw\t$t1, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
                }
                if (code.e.name[0] == '$') {         // eΪ$
                    fout << "lw\t$t2, " << stoi(code.e.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // eΪ����
                    fout << "addi\t$t2, $0, " << code.e.name << endl;
                }
                else {                          // eΪ����
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
                fout << "sw\t$t1, " << stoi(code.a.name.substr(1)) * 4 << "($s2)" << endl;  // ������ʱ������ֵ
            }
            else {  // $ = ������ [ ���� ]
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "addi\t$t1, " << base[c_global] << ", " << (c.adr * 4) << endl;     // ���������Ļ�ַ
                if (code.e.name[0] == '$') {         // ����eΪ$
                    fout << "lw\t$t2, " << stoi(code.e.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
                    fout << "sll\t$t2, $t2, 2" << endl;     // �����±��4
                    fout << "add\t$t1, $t1, $t2" << endl;   // �����Ԫ�صĵ�ַ
                    fout << "lw\t$t1, ($t1)" << endl;
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // eΪ����
                    fout << "lw\t$t1, " << (stoi(code.e.name) * 4) << "($t1)" << endl;
                }
                else {  // eΪ����
                    tab_item e = tab[findTab(code.e.name)];
                    int e_global = (e.block == 0) ? 1 : 0;
                    fout << "lw\t$t2, " << (e.adr * 4) << "(" << base[e_global] << ")" << endl; // ��ȡ�����±�
                    fout << "sll\t$t2, $t2, 2" << endl;     // �����±��4
                    fout << "add\t$t1, $t1, $t2" << endl;   // �����Ԫ�صĵ�ַ
                    fout << "lw\t$t1, ($t1)" << endl;
                }
                fout << "sw\t$t1, " << stoi(code.a.name.substr(1)) * 4 << "($s2)" << endl;  // ������ʱ������ֵ
            }
        }
        else if (code.e.name == "=") {   // ������ [ ���� ] = ����
            tab_item a = tab[findTab(code.a.name)];
            int a_global = (a.block == 0) ? 1 : 0;
            fout << "addi\t$t1, " << base[a_global] << ", " << (a.adr * 4) << endl;     // ���������Ļ�ַ
            if (code.c.name[0] == '$') {         // cΪ$
                fout << "lw\t$t2, " << stoi(code.c.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
                fout << "sll\t$t2, $t2, 2" << endl;
                fout << "add\t$t1, $t1, $t2" << endl;   // �����Ԫ�صĵ�ַ
            }
            else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // cΪ����
                fout << "addi\t$t1, $t1, " << (stoi(code.c.name) * 4) << endl;
            }
            else {      // cΪ����
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "lw\t$t2, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl; // ��ȡ�����±�
                fout << "sll\t$t2, $t2, 2" << endl;     // �����±��4
                fout << "add\t$t1, $t1, $t2" << endl;   // �����Ԫ�صĵ�ַ
            }
            if (code.f.name[0] == '$') {         // ����fΪ$
                fout << "lw\t$t2, " << stoi(code.f.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
            }
            else if (isdigit(code.f.name[0]) || code.f.name[0] == '-') {  // fΪ��
                fout << "addi\t$t2, $0, " << code.f.name << endl;
            }
            else {                          // fΪ����
                tab_item f = tab[findTab(code.f.name)];
                int f_global = (f.block == 0) ? 1 : 0;
                fout << "lw\t$t2, " << (f.adr * 4) << "(" << base[f_global] << ")" << endl;
            }
            fout << "sw\t$t2, ($t1)" << endl;
        }
        else if (code.a.name == "para") {
            // �������ڷ��ű��������в���
        }
        else if(code.b.name ==":")    // ��ǩ
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
                fout << "lw\t$t1, " << stoi(code.a.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
            else if (isdigit(code.a.name[0]))
                fout << "addi\t$t1, $0, " << code.a.name << endl;
            else {
                tab_item a = tab[findTab(code.a.name)];
                int a_global = (a.block == 0) ? 1 : 0;
                fout << "lw\t$t1, " << (a.adr * 4) << "(" << base[a_global] << ")" << endl;
            }
            if (code.c.name[0] == '$')
                fout << "lw\t$t2, " << stoi(code.c.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
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
            fout << "addi\t$t1, $s1, " << (ftab[pt_block].size * 4) << endl;    // �����¸�������Ļ�ַ
            if (code.b.name[0] == '$') {   // push $
                fout << "lw\t$t2, " << stoi(code.b.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
                fout << "sw\t$t2, " << (++offset * 4) << "($t1)" << endl;  // ����ֻ��Ϊint��char���ʴ�Сÿ�μ�4����
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {  // push ����
                fout << "addi\t$t2, $0, " << code.b.name << endl;
                fout << "sw\t$t2, " << (++offset * 4) << "($t1)" << endl;
            }
            else {  // push ����  
                tab_item b = tab[findTab(code.b.name)];
                int b_global = (b.block == 0) ? 1 : 0;
                fout << "lw\t$t2, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
                fout << "sw\t$t2, " << (++offset * 4) << "($t1)" << endl;
            }
        }
        else if (code.a.name == "printf") {
            if (code.b.name[0] == '\"') {    // ����ַ���
                fout << "la\t$a0, .string" << ++pt_stab << endl;    // �����ַ�����ַ��$a0
                fout << "addi\t$v0, $0, 4" << endl;
                fout << "syscall" << endl;
            }
            else if (code.b.name[0] == '$') {
                if (code.b.type == CHARTP) {     // char������Ҫ����ַ�
                    fout << "lw\t$a0, " << stoi(code.b.name.substr(1)) * 4 << "($s2)" << endl;  // ��charֵȡ��$a0
                    fout << "addi\t$v0, $0, 11" << endl;
                }
                else {
                    fout << "lw\t$t1, " << stoi(code.b.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
                    fout << "add\t$a0, $0, $t1" << endl;
                    fout << "addi\t$v0, $0, 1" << endl;
                }
                fout << "syscall" << endl;
            }
            else {
                tab_item b = tab[findTab(code.b.name)];
                int b_global = (b.block == 0) ? 1 : 0;
                if (code.b.type == CHARTP) {     // char������Ҫ����ַ�
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
            if (code.b.type == CHARTP) {     // ����char
                fout << "addi\t$v0, $0, 12" << endl;    // ����char��$v0
                fout << "syscall" << endl;
                fout << "sw\t$v0, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
            }
            else {
                fout << "addi\t$v0, $0, 5" << endl;     // ����int��$v0
                fout << "syscall" << endl;
                fout << "sw\t$v0, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl; // ��ֵ�浽������ַ��
            }
        }
        else if (code.a.name == "call") {
            offset = 0;
            /* ��ת����������ǰ */
            fout << "sw\t$ra, ($s1)" << endl;                                   // ���浱ǰ�����ķ��ص�ַ
            fout << "addi\t$s1, $s1, " << (ftab[pt_block].size * 4) << endl;    // ����$s1��ַλ��(�ӵ�ǰ������������С)
            fout << "addi\t$s2, $s2, " << (ftab[pt_block].tsize * 4) << endl;   // ����$s2��ַλ��(�ӵ�ǰ������ʱ��������С)
            fout << "jal\t" << code.b.name << endl;
            /* �������������غ� */
            fout << "addi\t$s1, $s1, " << -(ftab[pt_block].size * 4) << endl;   // ����$s1��ַλ��(����ǰ������������С)
            fout << "addi\t$s2, $s2, " << -(ftab[pt_block].tsize * 4) << endl;   // ����$s2��ַλ��(����ǰ������ʱ��������С)
            fout << "lw\t$ra, ($s1)" << endl;                                   // ������ĸú������ص�ַ����$ra
        }
        else if (code.a.name == "ret") {
            if (code.b.name == "") {
                if (pt_block == ftab.size() - 1) {  // main�������return
                    fout << "j\t.end" << endl;
                }
                else
                    fout << "jr\t$ra" << endl;
            }
            else if (code.b.name[0] == '$') {
                fout << "lw\t$t1, " << stoi(code.b.name.substr(1)) * 4 << "($s2)" << endl;  // ȡ����ʱ������ֵ
                fout << "add\t$v0, $0, $t1" << endl;
                fout << "jr\t$ra" << endl;
            }
            else if (isdigit(code.b.name[0])) {
                fout << "addi\t$v0, $0, " << code.b.name << endl;
                fout << "jr\t$ra" << endl;
            }
            else {  // ���ر���
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
    fout << endl << endl << "���ű�" << endl;
    fout << "[index] [name] [kind] [type] [ref] [adr] [block]" << endl;
    for (size_t i = 0; i < tab.size(); i++)    // tab������
       fout << i << "\t" << tab[i].name << "\t" << kinds_string[tab[i].kind] << "\t" << types_string[tab[i].type]
        << "\t" << tab[i].ref << "\t" << tab[i].adr << "\t" << tab[i].block << endl;
    fout << endl << endl << "������Ϣ��" << endl;
    fout << "[index] [eltype] [length]" << endl;
    for (size_t i = 0; i < atab.size(); i++)    // atab������
        fout << i << "\t" << ((atab[i].eltype == INTTP) ? "INT\t" : "CHAR") << "\t" << atab[i].length << endl;
    fout << endl << endl << "������Ϣ��" << endl;
    fout << "[index] [lastpara] [size] [tsize]" << endl; 
    for (size_t i = 0; i < ftab.size(); i++)    // ftab������
        fout << i << "\t" << ftab[i].lastpara << "\t" << ftab[i].size << "\t" << ftab[i].tsize << endl;
    fout << endl << endl << "�ַ�����" << endl;
    fout << "[index] [string]" << endl;
    for (size_t i = 0; i < stab.size(); i++)    // stab������
        fout << i << "\t" << stab[i] << endl;
    fout << endl << endl << "��Ԫʽ���룺" << endl;
    for (size_t i = 0; i < quad_code.size(); i++)
        fout << i << "\t" << quad_code[i].a.name + " " + quad_code[i].b.name + " " + quad_code[i].c.name + 
        " " + quad_code[i].d.name + " " + quad_code[i].e.name + " " + quad_code[i].f.name << endl;
}

int main()
{
    pt_label = -1;
    pt_block = 0;
    string file;
    cout << "��������Ҫ�����Դ�������·��:" << endl;
    getline(cin, file);
    fin.open(file);
    while (!fin) {
    cout << "�޷��򿪸��ļ�����ȷ���ļ��ɶ�����������·����" << endl;
    getline(cin, file);
    fin.open(file);
    }
    fin >> noskipws;    // �������ո�س�
    cout << "�������������ļ��ľ���·��:" << endl;
    getline(cin, file);
    fout.open(file);
    while (!fout) {
    cout << "�޷��򿪸��ļ�����ȷ���ļ���д����������·����" << endl;
    getline(cin, file);
    fout.open(file);
    }

    ch = ' ';
    fout << "Դ�룺" << endl;
    getsymbol();
    program();
    //printTabs();
    //optimize();
    transform();
    fin.close();
    fout.close();

}

