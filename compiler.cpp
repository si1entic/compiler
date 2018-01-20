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
int prerow = 1; // ��һ������������
int currow = 1; // ��ǰ������������
int nexrow = 1; // ��һ������������
bool isError = false;
vector<map<string, string>> registers(1);

void error(int id) {
    string msg[100];
    msg[1] = "����int����"; msg[2] = "ȱ��'"; msg[3] = "�ַ������ݳ���"; msg[4] = "!ֻ����=����";
    msg[5] = "�޷�ʶ�����"; msg[6] = "��ʶ���ظ�����"; msg[7] = "ȱ��(��;"; msg[8] = "void���������з���ֵ���()";
    msg[9] = "����ȱ�ٷ���ֵ������ֵӦ��()�ڣ�"; msg[10] = "ȱ�ٱ�ʶ��"; msg[11] = "ȱ��="; msg[12] = "ȱ��int��char";
    msg[13] = "�����±�Խ��"; msg[14] = "�����������ͳ���"; msg[15] = "ȱ��;"; msg[16] = "ȱ���޷�������";
    msg[17] = "ȱ��]"; msg[18] = "�޲����ĺ������ò��ܸ�()"; msg[19] = "�����������"; msg[20] = "�����������"; 
    msg[21] = "ȱ��)"; msg[22] = "��ʶ��δ����"; msg[23] = "�������ȱ���±�"; msg[24] = "���õĺ����޷���ֵ";
    msg[25] = "�������ͳ���"; msg[26] = "ȱ��("; msg[27] = "ȱ��else"; msg[28] = "��ʶ���Ǳ���"; 
    msg[29] = "ȱ��case"; msg[30] = "case��ȱ�ٳ���"; msg[31] = "case����ظ�����"; msg[32] = "ȱ��:";
    msg[33] = "ȱ��["; msg[34] = "���ܶԳ������и�ֵ"; msg[35] = "ȱ��}"; msg[36] = "�޷�ʶ�����"; 
    msg[37] = "�����������"; msg[38] = "ȱ��{"; msg[39] = "�����������"; msg[40] = "main�������ͱ���Ϊvoid";
    msg[41] = "ȱ��main����"; msg[42] = "main�������ж����ַ�"; msg[43] = "case�������ʹ���"; msg[44] = "��ʶ���������";
    msg[45] = "��0����������0��ͷ"; msg[46] = "�޲�����������ʱ�������()"; msg[47] = "<����>0ǰ��������������";
    fout << endl << "��" << prerow << "�У�" << msg[id];
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
        if (s[0] == '0'&&value != 0)    // �������ֲ�����0��ͷ
            error(45);
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
    while (ch<0||isspace(ch)) {   // �ж�Ԥ�����Ƿ�Ϊ�հ��ַ�
        if (ch < 0) {
            isError = true;
            fout << endl << "��" << nexrow << "�У����ڷǷ��ַ�";
        }
        if (ch == '\n')
            nexrow++;
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
    enter(CONSTANT, ic, block);  // ��������
    getsymbol();
}
void constsDefi(int block) {      // ����<����˵��>   sym��ʱΪCONST����ʱΪ�ֺź��һ������
    getsymbol();
    types ic;
    if (sym != INT&&sym != CHAR) {
        error(14);
        ic = INTTP;
    }
    else
        ic = (sym == INT) ? INTTP : CHARTP;    // ��¼��ǰ��������Ϊint����char
    getsymbol();
    constDefi(ic, block);   // ����һ����������
    while (sym == COMMA) {  // ����ͬ�ೣ������
        getsymbol();
        constDefi(ic, block);
    }
    if (sym != SEMICOLON)
        error(15);
    else
        getsymbol();
    if (sym == CONST)
        constsDefi(block);  // �ݹ���ã���������const<��������>;��
}

void varDefi(types ic, int block) {     // ����һ����������  sym��ʱΪIDENT���һ�����ţ���ʱΪ��������ĵ�һ������
    if (sym == LBRACK) {    // ��������
        getsymbol();
        if (sym != NUMBER) {
            error(16);
            return;
        }
        getsymbol();
        if (sym != RBRACK) {
            error(17);
            enterArray(ic, block);   // �������
        }
        else {
            enterArray(ic, block);   // �������
            getsymbol();
        }
    }
    else {
        enter(VARIABLE, ic, block);      // ��������
    }
}
void varsDefi(types ic, int block) {      // ����һ�б�������    sym��ʱΪIDENT���һ�����ţ���ʱΪ�ֺź��һ������
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

void parameter(int block) {  // ����һ������   sym��ʱΪ(���һ�����ţ���ʱΪIDENT���һ������
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
    enter(VARIABLE, tp, block); // ������tab
    getsymbol();
}
void parameters(int block) {  // ����<������>    sym��ʱΪ(����ʱΪ)���һ������
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
    ftab[ftab.size() - 1].lastpara = tab.size() - 1;    //  ��¼�ú������һ�������ڷ��ű��е�λ��
    if (sym != RPAREN)
        error(21);
    else
        getsymbol();
}

quad_item call(int position) {   // ������������䣬����Ϊ��������tab���е�λ��
    quad_item x;
    stringstream ss;
    int block = tab[position].block;                // ��ǰ����������
    int last = ftab[tab[position].ref].lastpara;    // ���һ�������ڷ��ű��е�λ��
    int index = position;   // ��ǰ�Ѷ����ķ��ű��е���
    if (sym == LPAREN) {    // ��ʵ�δ���
        vector<quad_item> paras;    // ����PUSH �������м���룬�������в�������call ����ǰһ�����������Ƕ��PUSH����
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
        for (size_t i = 0; i < paras.size(); i++) {
            genQuad("PUSH", paras[i].name);
        }
        if (sym != RPAREN)
            error(21);
        else
            getsymbol();
    }
    else {  // ��ʵ�δ���
        if (index != last)
            error(20);
    }
    // ʵ�μ�����ϣ����ɺ�������ָ��
    genQuad("CALL", tab[position].name);
    if (tab[position].type != VOIDTP) {
        ss << "T" << (++pt_T);
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
        if (position == -1) {
            error(22);
            exit(1);
        }
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
            if (item.type == ARRAYTP) {    // �ñ���Ϊ����
                if (sym != LBRACK) {
                    error(23);
                    exit(1);
                }
                getsymbol();
                y = expression();   // ���������±�
                // �ж��±귶Χ
                if (y.name[0] == '-' || isdigit(y.name[0])) {
                    int index = stoi(y.name);
                    if (index < 0 || index >= atab[tab[position].ref].length)
                        error(13);
                }
                if (sym != RBRACK)
                    error(17);
                else
                    getsymbol();
                // ���������׵�ַitem.adr���±�ֵ�ҵ���Ԫ�صĵ�ַ����ȡֵ��ջ
                ss << "T" << (++pt_T);
                x.name = ss.str();
                x.type = atab[item.ref].eltype;
                genQuad(x.name, "=", item.name, "[", y.name, "]");
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
quad_item term() {	// ����<��>
    quad_item x, y, z;
    stringstream ss;
    y = factor();
    while (sym == MULT||sym==DIVI) {
        symbol md = sym;    // ��¼�˳�����
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
        ss.str("");     // ���� �൱��.clear

       
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
        if (isdigit(y.name[0]))
            y.name = "-" + y.name;
        else if (y.name[0] == '-')
            y.name = y.name.substr(1);
        else {
            // ����ȡ�෴��ָ��
            ss << "T" << (++pt_T);
            x.name = ss.str();
            ss.str("");     // ���� �൱��.clear
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
        ss.str("");     // ���� �൱��.clear
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
    stringstream ss;
    switch (sym)
    {
    case IF:    // ʶ��<�������>
        int else_begin,if_end;
        getsymbol();
        if (sym != LPAREN)
            error(26);
        else
            getsymbol();
        condition();
        else_begin = ++pt_label;
        genQuad("BZ", genLabel(else_begin));  // ���������㣬����ת��else_begin��
        if (sym != RPAREN)
            error(21);
        else
            getsymbol();
        statement();
        if_end = ++pt_label;
        genQuad("GOTO", genLabel(if_end));    // thenִ����ϣ���ת��if_end��
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
                quad_code[quad_code.size()-1].b.type = y.type;  // ������Ԫʽ���type����������char��int
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
                    quad_code[quad_code.size() - 1].b.type = tab[position].type;  // ������Ԫʽ���type����������char��int
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
    case SWITCH:    // ʶ��<������>
        int switch_cmp,switch_end,default_begin;   // ���бȽϡ�switch��������default��ʼ��λ��
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
        genQuad("GOTO", genLabel(switch_cmp));       // �м������ת��case_cmp��
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
            case_value.push_back(value);       // ��¼����ֵ
            case_label.push_back(++pt_label);  // ��¼��ǩ�±�
            getsymbol();
            if (sym != COLON)
                error(32);
            else
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
        else
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
        if (tab[position].kind == FUNCTION) {     // ʶ��<��/�޷���ֵ��������>
            getsymbol();
            call(position);
        }
        else if (tab[position].kind == VARIABLE) {  // ʶ��<��ֵ���>
            if (tab[position].type == ARRAYTP) {    // ������ĳԪ�ظ�ֵ
                getsymbol();
                if (sym != LBRACK) {
                    error(33);
                    y.name = "0";
                }
                else {
                    getsymbol();
                    y = expression();   // ��ȡ�����±��ֵ
                                        // �ж��±귶Χ
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
        else
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
        if (sym != IDENT) {
            error(10);
        }
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
    pt_T = -1;      // ������ʱ������
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
        parameters(pt_block);   // �������
        if (sym != LBRACE)
            error(38);
        else
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
    if (quad_code[quad_code.size() - 1].a.name != "RET")  // �������һ��������return�������
        genQuad("RET");
    if (sym != RBRACE)
        error(35);
    else
        getsymbol();
    ftab[ftab.size() - 1].tsize = pt_T + 1;
}
void fun_main() {
    pt_block++;
    pt_T = -1;      // ������ʱ������
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
    comState(pt_block);   // ���������
    if (sym != RBRACE)
        error(35);
    else
        getsymbol();
    ftab[ftab.size() - 1].tsize = pt_T + 1;
}
void program() {    // ����<����>
    if (sym == CONST)   // ʶ��ȫ�ֳ�������
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
            error(39);
        else {
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
            index == quad_code.size() - 1) { // ����һ��������
            vector<pair<string, int>> local_count_vec(local_count_map.begin(), local_count_map.end());
            sort(local_count_vec.begin(), local_count_vec.end(), cmp_by_value);   // ����
            int i = 0;
            for (vector<pair<string, int>>::iterator it = local_count_vec.begin(); it!= local_count_vec.end(); it++) {   
                if (i < 8) {    // ����������8���ֲ���������Ĵ���
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
            if (code.d.name == "") {    // ��ʽ: a = c
                var_count(local_count_map, global_count_map, code.a.name);
                var_count(local_count_map, global_count_map, code.c.name);
            }
            else if (code.f.name == "") {    // ��ʽ: T = c op e
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

    // �滻ȫ�ֱ���
    vector<pair<string, int>> global_count_vec(global_count_map.begin(), global_count_map.end());
    sort(global_count_vec.begin(), global_count_vec.end(), cmp_by_value);   // ����
    int i = 0;
    for (vector<pair<string, int>>::iterator it = global_count_vec.begin(); it != global_count_vec.end(); it++) {
        if (i < 7) {    // ����������7��ȫ�ֱ�������Ĵ���
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
        int x, y;       // �����ӽ���
    }node;
    struct dag_item {
        string name;
        int id;
    }item;
    vector<dag_node> dag;       // DAGͼ
    vector<dag_item> dag_tab;   // ����
    int pt_node = 0;
    for (vector<quadruple>::iterator iter_quad = quad_code.begin(); iter_quad != quad_code.end(); iter_quad++) {
        quadruple& code = *iter_quad;   // ���������ã������޸���Ч
        bool del = false;   // ����Ƿ���Ҫɾ����ǰ��Ԫʽ
        if (code.b.name == ":"||code.c.name=="(") {   // ��ǩ���ֻ�����
            pt_node = 0;
            dag.clear();
            dag_tab.clear();
            continue;
        }
        int x, y, z;    // ��¼a��c��e�Ľ���
        int i, tab_len, dag_len;
        if (code.b.name == "=") {   // a = c �� a = c op e �� a = c [ e ]
            if (code.c.name == "RET")
                continue;
            /* ���Ҳ��޸�c */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.c.name) {   // ���������иñ���
                    y = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // ����δ�ҵ�
                y = node.id = item.id = ++pt_node;
                node.op = item.name = code.c.name;      // Ҷ���opΪ���������ӽڵ�Ϊ0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // ��ӵ�DAGͼ
                dag_tab.push_back(item);    // ��ӵ�����
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == y && dag_tab[i].name[0] == 'T') {
                    if (code.d.name == ""&&code.a.name[0] == 'T')  // �ظ���T = array��ɾȥ
                        del = true;
                    else
                        code.c.name = dag_tab[i].name;  // ����Ԫʽ��c�޸�Ϊ��һ������ڵ����ͬ����ʱ����
                    break;
                }
            }
            if (code.d.name == "") {    // ����ʽΪ��a = c 
                x = y; 
            }
            else {   // a = c op e �� a = c [ e ]
                /* ���Ҳ��޸�e */
                for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                    if (dag_tab[i].name == code.e.name) {   // ���������иñ���
                        z = dag_tab[i].id;
                        break;
                    }
                } 
                if (i == tab_len) {    // ����δ�ҵ�
                    z = node.id = item.id = ++pt_node;
                    node.op = item.name = code.e.name;      // Ҷ���opΪ���������ӽڵ�Ϊ0
                    node.x = 0;
                    node.y = 0;
                    dag.push_back(node);        // ��ӵ�DAGͼ
                    dag_tab.push_back(item);    // ��ӵ�����
                }
                for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                    if (dag_tab[i].id == z && dag_tab[i].name[0] == 'T') {
                        code.e.name = dag_tab[i].name;  // ����Ԫʽ��e�޸�Ϊ��һ������ڵ����ͬ����ʱ����
                        break;
                    }
                }
                /* ���Ҳ��޸�cde */
                for (i = 0, dag_len = dag.size(); i < dag_len; i++) {
                    if (dag[i].op == code.d.name&&dag[i].x == y&&dag[i].y == z) {   // DAGͼ�д����ӽ��Ͳ���������ͬ�Ľ��
                        x = dag[i].id;
                        if (code.a.name[0] == 'T') { // �ظ�����ʱ�������㣬ɾȥ
                            del = true;
                        }
                        else {  // ����Ԫʽ�ұ��ظ��ı��ʽ�滻Ϊ��һ�������ͬ�ı���
                            for (int j = 0, tab_len = dag_tab.size(); j < tab_len; j++) {
                                if (dag_tab[j].id == x) {
                                    code.c.name = dag_tab[j].name;  // ����Ԫʽ��c�޸�Ϊ��һ������ڵ����ͬ�ı�����
                                    code.d.name = code.e.name = code.f.name = "";
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                if (i==dag_len) {   // DAGͼ���޸ý��
                    x = node.id = ++pt_node;
                    node.op = code.d.name;
                    node.x = y;
                    node.y = z;
                    dag.push_back(node);
                }
            }
            // ���½�����a��Ӧ�Ľ���
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.a.name) {   // �����д��ڸñ�����������½���
                    dag_tab[i].id = x;
                    break;
                }
            }
            if (i == tab_len) { // ������������һ��
                item.name = code.a.name;
                item.id = x;
                dag_tab.push_back(item);
            }
            if (del) {
                iter_quad = quad_code.erase(iter_quad); // ɾ������Ԫʽ
                iter_quad--;    // erase()�Ὣ��ǰָ��ָ��ɾ��Ԫ�صĺ�һԪ��,��ִ��for���++�����������һ��Ԫ�أ�����--
            }
        }
        else if (code.e.name == "=") {  // T [ c ] = f
            /* ���Ҳ��޸�f */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.f.name) {   // ���������иñ���
                    z = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // ����δ�ҵ�
                z = node.id = item.id = ++pt_node;
                node.op = item.name = code.f.name;      // Ҷ���opΪ���������ӽڵ�Ϊ0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // ��ӵ�DAGͼ
                dag_tab.push_back(item);    // ��ӵ�����
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == z && dag_tab[i].name[0] == 'T') {
                    code.f.name = dag_tab[i].name;  // ����Ԫʽ��f�޸�Ϊ��һ������ڵ����ͬ����ʱ����
                    break;
                }
            }
            /* ���Ҳ��޸�a */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.a.name) {   // aΪ��������ʱ����T��֮ǰ�س��ֹ�
                    x = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // ����δ�ҵ�
                x = node.id = item.id = ++pt_node;
                node.op = item.name = code.a.name;      // Ҷ���opΪ���������ӽڵ�Ϊ0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // ��ӵ�DAGͼ
                dag_tab.push_back(item);    // ��ӵ�����
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == x && dag_tab[i].name[0] == 'T') {
                    code.a.name = dag_tab[i].name;  // ����Ԫʽ��a�޸�Ϊ��һ������ڵ����ͬ����ʱ����
                    break;
                }
            }
            /* ���������·�����(��Ϊ�����е�Ԫ���ѱ��޸�) */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == x && dag_tab[i].name[0] != 'T') {
                    dag_tab[i].id = node.id = ++pt_node;
                    node.op = dag_tab[i].name[0];      // Ҷ���opΪ���������ӽڵ�Ϊ0
                    node.x = 0;
                    node.y = 0;
                    dag.push_back(node);        // ��ӵ�DAGͼ
                    break;
                }
            }
            /* ���Ҳ��޸�c */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.c.name) {   // ���������иñ���
                    y = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // ����δ�ҵ�
                y = node.id = item.id = ++pt_node;
                node.op = item.name = code.c.name;      // Ҷ���opΪ���������ӽڵ�Ϊ0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // ��ӵ�DAGͼ
                dag_tab.push_back(item);    // ��ӵ�����
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == y && dag_tab[i].name[0] == 'T') {
                    code.c.name = dag_tab[i].name;  // ����Ԫʽ��f�޸�Ϊ��һ������ڵ����ͬ����ʱ����
                    break;
                }
            }
        }
        else if (code.b.name == "<" || code.b.name == "<=" || code.b.name == ">" ||
            code.b.name == ">=" || code.b.name == "!=" || code.b.name == "==") {    // a op c
            /* ���Ҳ��޸�a */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.a.name) {   // ���������иñ���
                    x = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // ����δ�ҵ�
                x = node.id = item.id = ++pt_node;
                node.op = item.name = code.a.name;      // Ҷ���opΪ���������ӽڵ�Ϊ0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // ��ӵ�DAGͼ
                dag_tab.push_back(item);    // ��ӵ�����
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == x && dag_tab[i].name[0] == 'T') {
                    code.a.name = dag_tab[i].name;  // ����Ԫʽ��f�޸�Ϊ��һ������ڵ����ͬ����ʱ����
                    break;
                }
            }
            /* ���Ҳ��޸�c */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.c.name) {   // ���������иñ���
                    y = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // ����δ�ҵ�
                y = node.id = item.id = ++pt_node;
                node.op = item.name = code.c.name;      // Ҷ���opΪ���������ӽڵ�Ϊ0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // ��ӵ�DAGͼ
                dag_tab.push_back(item);    // ��ӵ�����
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == y && dag_tab[i].name[0] == 'T') {
                    code.c.name = dag_tab[i].name;  // ����Ԫʽ��f�޸�Ϊ��һ������ڵ����ͬ����ʱ����
                    break;
                }
            }
        }
        else if (code.a.name == "PUSH"|| code.a.name == "PRINTF"|| code.a.name == "RET") {
            if (code.b.name == "")
                continue;
            /* ���Ҳ��޸�b */
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].name == code.b.name) {   // ���������иñ���
                    x = dag_tab[i].id;
                    break;
                }
            }
            if (i == tab_len) {    // ����δ�ҵ�
                x = node.id = item.id = ++pt_node;
                node.op = item.name = code.b.name;      // Ҷ���opΪ���������ӽڵ�Ϊ0
                node.x = 0;
                node.y = 0;
                dag.push_back(node);        // ��ӵ�DAGͼ
                dag_tab.push_back(item);    // ��ӵ�����
            }
            for (i = 0, tab_len = dag_tab.size(); i < tab_len; i++) {
                if (dag_tab[i].id == x && dag_tab[i].name[0] == 'T') {
                    code.b.name = dag_tab[i].name;  // ����Ԫʽ��b�޸�Ϊ��һ������ڵ����ͬ����ʱ����
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
    fout << ".align 2" << endl; // �ڴ水word����
    fout << ".begin:" << endl;
    fout << ".text" << endl;
    fout << "la\t$gp, .begin" << endl;                          // $gp��ȫ����������ַ
    fout << "addi\t$s7, $gp, " << (ftab[0].size*4) << endl;     // $s7�浱ǰ��������ַ
                                                                // $sp�浱ǰ��ʱ��������ַ
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
            fout << code.b.name << ":" << endl;  // ���ɺ�����ǩ
            pc++;
            continue;
        }
        /* ��ԪʽΪ��ֵ��� */
        if (code.b.name == "=") {
            if (code.d.name == "") {
                if (code.c.name == "RET") {     // ��ʽ: T = RET
                    fout << "sw\t$v0, " << stoi(code.a.name.substr(1))*4 << "($sp)" << endl;
                    pc++;
                    continue;
                }
                tab_item a = tab[findTab(code.a.name)];
                int a_global = (a.block == 0) ? 1 : 0;
                if (code.c.name[0] == 'T') {         // ��ʽ: x = T
                    fout << "lw\t$t8, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0]=='-') {  // ��ʽ: x = 1
                    fout << "addi\t$t8, $0, " << code.c.name << endl;
                }
                else {                          // ��ʽ: x = y
                    tab_item c = tab[findTab(code.c.name)];
                    int c_global = (c.block == 0) ? 1 : 0;
                    fout << "lw\t$t8, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
                }
                fout << "sw\t$t8, " << (a.adr * 4) << "(" << base[a_global] << ")" << endl;  // ������ʱ������ֵ
            }
            else if (code.f.name == "") {    // ��ʽ: T = c op e
                if (code.c.name[0] == 'T') {         // cΪT
                    fout << "lw\t$t8, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // cΪ����
                    fout << "addi\t$t8, $0, " << code.c.name << endl;
                }
                else {                          // cΪ����
                    tab_item c = tab[findTab(code.c.name)];
                    int c_global = (c.block == 0) ? 1 : 0;
                    fout << "lw\t$t8, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl;
                }
                if (code.e.name[0] == 'T') {         // eΪT
                    fout << "lw\t$t9, " << stoi(code.e.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // eΪ����
                    fout << "addi\t$t9, $0, " << code.e.name << endl;
                }
                else {                          // eΪ����
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
                fout << "sw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // ������ʱ������ֵ
            }
            else {  // T = c [ e ]
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "addi\t$t8, " << base[c_global] << ", " << (c.adr * 4) << endl;  // ���������Ļ�ַ
                if (code.e.name[0] == 'T') {         // ����eΪT
                    fout << "lw\t$t9, " << stoi(code.e.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                    fout << "sll\t$t9, $t9, 2" << endl;     // �����±��4
                    fout << "add\t$t8, $t8, $t9" << endl;   // �����Ԫ�صĵ�ַ
                    fout << "lw\t$t8, ($t8)" << endl;
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // eΪ����
                    fout << "lw\t$t8, " << (stoi(code.e.name) * 4) << "($t8)" << endl;
                }
                else {  // eΪ����
                    tab_item e = tab[findTab(code.e.name)];
                    int e_global = (e.block == 0) ? 1 : 0;
                    fout << "lw\t$t9, " << (e.adr * 4) << "(" << base[e_global] << ")" << endl; // ��ȡ�����±�
                    fout << "sll\t$t9, $t9, 2" << endl;     // �����±��4
                    fout << "add\t$t8, $t8, $t9" << endl;   // �����Ԫ�صĵ�ַ
                    fout << "lw\t$t8, ($t8)" << endl;
                }
                fout << "sw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // ������ʱ������ֵ
            }
        }
        else if (code.e.name == "=") {   // a [ c ] = f
            tab_item a = tab[findTab(code.a.name)];
            int a_global = (a.block == 0) ? 1 : 0;
            fout << "addi\t$t8, " << base[a_global] << ", " << (a.adr * 4) << endl;  // ���������Ļ�ַ
            if (code.c.name[0] == 'T') {         // cΪT
                fout << "lw\t$t9, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                fout << "sll\t$t9, $t9, 2" << endl;
                fout << "add\t$t8, $t8, $t9" << endl;   // �����Ԫ�صĵ�ַ
            }
            else if (isdigit(code.c.name[0])) {  // cΪ����(�����±�Ǹ�)
                fout << "addi\t$t8, $t8, " << (stoi(code.c.name) * 4) << endl;
            }
            else {      // cΪ����
                tab_item c = tab[findTab(code.c.name)];
                int c_global = (c.block == 0) ? 1 : 0;
                fout << "lw\t$t9, " << (c.adr * 4) << "(" << base[c_global] << ")" << endl; // ��ȡ�����±�
                fout << "sll\t$t9, $t9, 2" << endl;     // �����±��4
                fout << "add\t$t8, $t8, $t9" << endl;   // �����Ԫ�صĵ�ַ
            }
            if (code.f.name[0] == 'T') {         // ����fΪT
                fout << "lw\t$t9, " << stoi(code.f.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
            }
            else if (isdigit(code.f.name[0]) || code.f.name[0] == '-') {  // fΪ��
                fout << "addi\t$t9, $0, " << code.f.name << endl;
            }
            else {                          // fΪ����
                tab_item f = tab[findTab(code.f.name)];
                int f_global = (f.block == 0) ? 1 : 0;
                fout << "lw\t$t9, " << (f.adr * 4) << "(" << base[f_global] << ")" << endl;
            }
            fout << "sw\t$t9, ($t8)" << endl;
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
            if (code.a.name[0] == 'T')
                fout << "lw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
            else if (isdigit(code.a.name[0]) || code.a.name[0] == '-')
                fout << "addi\t$t8, $0, " << code.a.name << endl;
            else {
                tab_item a = tab[findTab(code.a.name)];
                int a_global = (a.block == 0) ? 1 : 0;
                fout << "lw\t$t8, " << (a.adr * 4) << "(" << base[a_global] << ")" << endl;
            }
            if (code.c.name[0] == 'T')
                fout << "lw\t$t9, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
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
            fout << "addi\t$t8, $s7, " << (ftab[pt_block].size * 4) << endl;    // �����¸�������Ļ�ַ
            if (code.b.name[0] == 'T') {   // PUSH T
                fout << "lw\t$t9, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                fout << "sw\t$t9, " << (++offset * 4) << "($t8)" << endl;  // ����ֻ��Ϊint��char���ʴ�Сÿ�μ�4����
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {  // PUSH ����
                fout << "addi\t$t9, $0, " << code.b.name << endl;
                fout << "sw\t$t9, " << (++offset * 4) << "($t8)" << endl;
            }
            else {  // PUSH ����  
                tab_item b = tab[findTab(code.b.name)];
                int b_global = (b.block == 0) ? 1 : 0;
                fout << "lw\t$t9, " << (b.adr * 4) << "(" << base[b_global] << ")" << endl;
                fout << "sw\t$t9, " << (++offset * 4) << "($t8)" << endl;
            }
        }
        else if (code.a.name == "PRINTF") {
            if (code.b.name[0] == '\"') {    // ����ַ���
                fout << "la\t$a0, .string" << ++pt_stab << endl;    // �����ַ�����ַ��$a0
                fout << "addi\t$v0, $0, 4" << endl;         // print string
            }
            else if (code.b.name[0] == 'T') {
                if (code.b.type == CHARTP) {    
                    fout << "lw\t$a0, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // ��charֵȡ��$a0
                    fout << "addi\t$v0, $0, 11" << endl;    // print char
                }
                else {
                    fout << "lw\t$t8, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
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
                if (code.b.type == CHARTP) {     // char������Ҫ����ַ�
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
        else if (code.a.name == "CALL") {
            offset = 0;
            /* ��ת����������ǰ */
            fout << "sw\t$ra, ($s7)" << endl;                                   // ���浱ǰ�����ķ��ص�ַ
            fout << "addi\t$s7, $s7, " << (ftab[pt_block].size * 4) << endl;    // ����$s7��ַλ��(�ӵ�ǰ������������С)
            fout << "addi\t$sp, $sp, " << (ftab[pt_block].tsize * 4) << endl;   // ����$sp��ַλ��(�ӵ�ǰ������ʱ��������С)
            fout << "jal\t" << code.b.name << endl;
            /* �������������غ� */
            fout << "addi\t$s7, $s7, " << -(ftab[pt_block].size * 4) << endl;   // ����$s7��ַλ��(����ǰ������������С)
            fout << "addi\t$sp, $sp, " << -(ftab[pt_block].tsize * 4) << endl;  // ����$sp��ַλ��(����ǰ������ʱ��������С)
            fout << "lw\t$ra, ($s7)" << endl;                                   // ������ĸú������ص�ַ����$ra
        }
        else if (code.a.name == "RET") {
            if (code.b.name == "") {
                if (pt_block == ftab.size() - 1)   // main�������return
                    fout << "j\t.end" << endl;
                else
                    fout << "jr\t$ra" << endl;
            }
            else if (code.b.name[0] == 'T') {
                fout << "lw\t$t8, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                fout << "add\t$v0, $0, $t8" << endl;
                fout << "jr\t$ra" << endl;
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {
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

void transform_opt() {
    fout << ".data" << endl;
    for (size_t i = 0; i < stab.size(); i++) {
        fout << ".string" << i << ": .asciiz \"" << stab[i] << "\"" << endl;
    }
    fout << ".align 2" << endl; // �ڴ水word����
    fout << ".begin:" << endl;
    fout << ".text" << endl;
    fout << "la\t$gp, .begin" << endl;                          // $gp��ȫ����������ַ
    fout << "addi\t$s7, $gp, " << (ftab[0].size * 4) << endl;   // $s7�浱ǰ��������ַ
                                                                // $sp�浱ǰ��ʱ��������ַ
    // ����������ȫ�ֱ����Ļ�ַ���Ĵ���
    for (int i = 0; tab[i].block==0; i++) {
        if (tab[i].type == ARRAYTP) {
            if (registers[0][tab[i].name] != "")    // ���ñ��������˼Ĵ���
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
            fout << code.b.name << ":" << endl;  // ���ɺ�����ǩ
            // ���ѷ���$�Ĳ�����ֵ�ӵ�ַ���ص��Ĵ���
            int index;
            for (index = tab.size() - 1; index >= 0; index--) { // ���Һ�������tab�е�λ��
                if (tab[index].block != 0)
                    continue;
                if (tab[index].name == code.b.name)
                    break;
            }
            for (int i = index+1; i <= ftab[pt_block].lastpara; i++) {  // �����ú�������
                if (registers[pt_block][tab[i].name] != "") {   //����ò��������˼Ĵ���
                    fout << "lw\t" << registers[pt_block][tab[i].name] << ", " << (tab[i].adr * 4) << "($s7)" << endl;
                }
            }
            // �������;ֲ������ĵ�ַ���ص��Ĵ���
            for (size_t i = index + 1; i < tab.size() && tab[i].block == pt_block; i++) {
                if (tab[i].type == ARRAYTP) {
                    if (registers[pt_block][tab[i].name] != "")    // ���ñ��������˼Ĵ���
                        fout << "addi\t" << registers[pt_block][tab[i].name] << ", $sp, " << (tab[i].adr * 4) << endl;
                }
            }
            pc++;
            continue;
        }
        /* ��ԪʽΪ��ֵ��� */
        if (code.b.name == "=") {
            if (code.d.name == "") {
                if (code.c.name == "RET") {     // ��ʽ: T = RET
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
                if (t0 != "") {     // a�����˼Ĵ���
                    if (code.c.name[0] == 'T') {         // ��ʽ: x = T
                        string t1 = registers[pt_block][code.c.name];
                        if (t1 != "")   // c�����˼Ĵ���
                            fout << "add\t" << t0 << ", $0, " << t1 << endl;
                        else {
                            fout << "lw\t"<< t0 <<", " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                        }
                    }
                    else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // ��ʽ: x = 1
                        fout << "addi\t" << t0 << ", $0, " << code.c.name << endl;
                    }
                    else {                          // ��ʽ: x = y
                        tab_item c = tab[findTab(code.c.name)];
                        int r1 = (c.block == 0) ? 0 : pt_block;
                        string t1 = registers[r1][code.c.name]; 
                        if (t1 != "") { // c�����˼Ĵ���
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
                    if (code.c.name[0] == 'T') {         // ��ʽ: x = T
                        string t1 = registers[pt_block][code.c.name];
                        if (t1 != "")   // c�����˼Ĵ���
                            fout << "sw\t" << t1 << ", " << (a.adr * 4) << "(" << base_a << ")" << endl;
                        else {
                            fout << "lw\t$t8, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl; 
                            fout << "sw\t$t8, " << (a.adr * 4) << "(" << base_a << ")" << endl;
                        }
                    }
                    else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // ��ʽ: x = 1
                        fout << "addi\t$t8, $0, " << code.c.name << endl;
                        fout << "sw\t$t8, " << (a.adr * 4) << "(" << base_a << ")" << endl;
                    }
                    else {                          // ��ʽ: x = y
                        tab_item c = tab[findTab(code.c.name)];
                        int r1 = (c.block == 0) ? 0 : pt_block;
                        string t1 = registers[r1][code.c.name];
                        if (t1 != "") { // c�����˼Ĵ���
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
            else if (code.f.name == "") {    // ��ʽ: T = c op e
                string t0, t1, t2;  // ����a,c,e�ļĴ���
                if (code.c.name[0] == 'T') {         // cΪT
                    t1 = registers[pt_block][code.c.name];
                    if (t1 == "") {  // cδ����Ĵ���
                        fout << "lw\t$t8, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;
                        t1 = "$t8";
                    }
                }
                else if (isdigit(code.c.name[0]) || code.c.name[0] == '-') {  // cΪ����
                    fout << "addi\t$t8, $0, " << code.c.name << endl;
                    t1 = "$t8";
                }
                else {                          // cΪ����
                    tab_item c = tab[findTab(code.c.name)];
                    int r1 = (c.block == 0) ? 0 : pt_block;
                    string base_c = (c.block == 0) ? "$gp" : "$s7";
                    t1 = registers[r1][code.c.name];
                    if (t1 == "") {  // cδ����Ĵ���
                        fout << "lw\t$t8, " << (c.adr * 4) << "(" << base_c << ")" << endl;
                        t1 = "$t8";
                    }
                }

                if (code.e.name[0] == 'T') {         // eΪT
                    t2 = registers[pt_block][code.e.name];
                    if (t2 == "") {  // eδ����Ĵ���
                        fout << "lw\t$t9, " << stoi(code.e.name.substr(1)) * 4 << "($sp)" << endl;
                        t2 = "$t9";
                    }
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // eΪ����
                    fout << "addi\t$t9, $0, " << code.e.name << endl;
                    t2 = "$t9";
                }
                else {                          // eΪ����
                    tab_item e = tab[findTab(code.e.name)];
                    int r2 = (e.block == 0) ? 0 : pt_block;
                    string base_e = (e.block == 0) ? "$gp" : "$s7";
                    t2 = registers[r2][code.e.name];
                    if (t2 == "") {  // cδ����Ĵ���
                        fout << "lw\t$t9, " << (e.adr * 4) << "(" << base_e << ")" << endl;
                        t2 = "$t9";
                    }
                }

                t0 = registers[pt_block][code.a.name];
                if (t0 == "")   // aδ����Ĵ���
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
                    fout << "sw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // ������ʱ������ֵ
            }
            else {  // T = c [ e ]
                string t0, t1, t2;  // ����a,c,e�ļĴ���
                tab_item c = tab[findTab(code.c.name)];
                int r1 = (c.block == 0) ? 0 : pt_block;
                string base_c = (c.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.c.name];
                if (t1 == "") {  // cδ����Ĵ���
                    fout << "addi\t$t8, " << base_c << ", " << (c.adr * 4) << endl;  // ���������Ļ�ַ
                    t1 = "$t8";
                }
                if (code.e.name[0] == 'T') {         // ����eΪT
                    t2 = registers[pt_block][code.e.name];
                    if (t2 == "") {  // eδ����Ĵ���
                        fout << "lw\t$t9, " << stoi(code.e.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                        t2 = "$t9";
                    }
                    fout << "sll\t$t9, " << t2 << ", 2" << endl;        // �����±��4
                    fout << "add\t$t8, " << t1 << ", $t9" << endl;      // �����Ԫ�صĵ�ַ
                    fout << "lw\t$t8, ($t8)" << endl;               // ȡֵ
                }
                else if (isdigit(code.e.name[0]) || code.e.name[0] == '-') {  // eΪ����
                    fout << "lw\t$t8, " << (stoi(code.e.name) * 4) << "(" << t1 << ")" << endl;
                }
                else {  // eΪ����
                    tab_item e = tab[findTab(code.e.name)];
                    int r2 = (e.block == 0) ? 0 : pt_block;
                    string base_e = (e.block == 0) ? "$gp" : "$s7";
                    t2 = registers[r2][code.e.name];
                    if (t2 == "") {  // eδ����Ĵ���
                        fout << "lw\t$t9, " << (e.adr * 4) << "(" << base_e << ")" << endl;
                        t2 = "$t9";
                    }
                    fout << "sll\t$t9, " << t2 << ", 2" << endl;        // �����±��4
                    fout << "add\t$t8, " << t1 << ", $t9" << endl;      // �����Ԫ�صĵ�ַ
                    fout << "lw\t$t8, ($t8)" << endl;               // ȡֵ
                }
                t0 = registers[pt_block][code.a.name];
                if (t0 == "") {  // aδ����Ĵ���
                    fout << "sw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;
                }
                else
                    fout << "add\t" << t0 << ", $0, $t8" << endl;
            }
        }
        else if (code.e.name == "=") {   // a [ c ] = f
            string t0, t1, t2;  // ����a,c,f�ļĴ���
            tab_item a = tab[findTab(code.a.name)];
            int r0 = (a.block == 0) ? 0 : pt_block;
            string base_a = (a.block == 0) ? "$gp" : "$s7";
            t0 = registers[r0][code.a.name];
            if (t0 == "") {
                fout << "addi\t$t8, " << base_a << ", " << (a.adr * 4) << endl;  // ���������Ļ�ַ
                t0 = "$t8";
            }
            if (code.c.name[0] == 'T') {         // cΪT
                t1 = registers[pt_block][code.c.name];
                if (t1 == "") {
                    fout << "lw\t$t9, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                    t1 = "$t9";
                }
                fout << "sll\t$t9, " << t1 << ", 2" << endl;
                fout << "add\t$t8, " << t0 << ", $t9" << endl;  // �����Ԫ�صĵ�ַ
            }
            else if (isdigit(code.c.name[0])) {  // cΪ����(�����±�Ǹ�)
                fout << "addi\t$t8, " << t0 << ", " << (stoi(code.c.name) * 4) << endl;
            }
            else {      // cΪ����
                tab_item c = tab[findTab(code.c.name)];
                int r1 = (c.block == 0) ? 0 : pt_block;
                string base_c = (c.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.c.name];
                if (t1 == "") {
                    fout << "lw\t$t9, " << (c.adr * 4) << "(" << base_c << ")" << endl; // ��ȡ�����±�
                    t1 = "$t9";
                }
                fout << "sll\t$t9, " << t1 << ", 2" << endl;
                fout << "add\t$t8, " << t0 << ", $t9" << endl;  // �����Ԫ�صĵ�ַ
            }
            if (code.f.name[0] == 'T') {         // ����fΪT
                t2 = registers[pt_block][code.f.name];
                if (t2 == "") {
                    fout << "lw\t$t9, " << stoi(code.f.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                    t2 = "$t9";
                }
            }
            else if (isdigit(code.f.name[0]) || code.f.name[0] == '-') {  // fΪ��
                fout << "addi\t$t9, $0, " << code.f.name << endl;
                t2 = "$t9";
            }
            else {                          // fΪ����
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
        else if (code.b.name == ":")    // ��ǩ
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
                    fout << "lw\t$t8, " << stoi(code.a.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
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
                    fout << "lw\t$t9, " << stoi(code.c.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
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
            fout << "addi\t$t8, $s7, " << (ftab[pt_block].size * 4) << endl;    // �����¸�������Ļ�ַ
            if (code.b.name[0] == 'T') {   // PUSH T
                t1 = registers[pt_block][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$t9, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
                    t1 = "$t9";
                }
                fout << "sw\t" << t1 << ", " << (++offset * 4) << "($t8)" << endl;  // ����ֻ��Ϊint��char���ʴ�Сÿ�μ�4����
            }
            else if (isdigit(code.b.name[0]) || code.b.name[0] == '-') {  // PUSH ����
                fout << "addi\t$t9, $0, " << code.b.name << endl;
                fout << "sw\t$t9, " << (++offset * 4) << "($t8)" << endl;
            }
            else {  // PUSH ����  
                tab_item b = tab[findTab(code.b.name)];
                int r1 = (b.block == 0) ? 0 : pt_block;
                string base_b = (b.block == 0) ? "$gp" : "$s7";
                t1 = registers[r1][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$t9, " << (b.adr * 4) << "(" << base_b << ")" << endl;
                    t1 = "$t9";
                }
                fout << "sw\t" << t1 << ", " << (++offset * 4) << "($t8)" << endl;  // ����ֻ��Ϊint��char���ʴ�Сÿ�μ�4����
            }
        }
        else if (code.a.name == "PRINTF") {
            string t1;
            if (code.b.name[0] == '\"') {    // ����ַ���
                fout << "la\t$a0, .string" << ++pt_stab << endl;    // �����ַ�����ַ��$a0
                fout << "addi\t$v0, $0, 4" << endl;         // print string
            }
            else if (code.b.name[0] == 'T') {
                t1 = registers[pt_block][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$a0, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // ��charֵȡ��$a0
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
                if (code.b.type == CHARTP) {     // char������Ҫ����ַ�
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
            if (code.b.type == CHARTP) {     // ����char
                fout << "addi\t$v0, $0, 12" << endl;    // ����char��$v0
            }
            else {
                fout << "addi\t$v0, $0, 5" << endl;     // ����int��$v0
            }
            fout << "syscall" << endl;
            if (t1 == "") {
                fout << "sw\t$v0, " << (b.adr * 4) << "(" << base_b << ")" << endl; // ��ֵ�浽������ַ��
            }
            else
                fout << "add\t" << t1 << ", $v0, $0, " << endl;
        }
        else if (code.a.name == "CALL") {
            offset = 0;
            /* ��ת����������ǰ */
            fout << "sw\t$ra, ($s7)" << endl;                                   // ���浱ǰ�����ķ��ص�ַ
            fout << "addi\t$t8, $sp, " << (ftab[pt_block].tsize * 4) << endl;   // ����ֲ��Ĵ�����ֵ
            int i = 0;
            for (map<string, string>::iterator it = registers[pt_block].begin(); it != registers[pt_block].end(); it++) {
                if (it->second != "") {
                    fout << "sw\t$t" << i << ", " << i * 4 << "($t8)" << endl;
                    i++;
                }
            }
            fout << "addi\t$s7, $s7, " << (ftab[pt_block].size * 4) << endl;    // ����$s7��ַλ��(�ӵ�ǰ������������С)
            fout << "addi\t$sp, $sp, " << ((ftab[pt_block].tsize + 8) * 4) << endl;   // ����$sp��ַλ��(�ӵ�ǰ������ʱ������+�ֲ��Ĵ�����С)
            fout << "jal\t" << code.b.name << endl;
            /* �������������غ� */
            fout << "addi\t$sp, $sp, " << -((ftab[pt_block].tsize + 8) * 4) << endl;  // ����$sp��ַλ��(����ǰ������ʱ������+�ֲ��Ĵ�����С)
            fout << "addi\t$s7, $s7, " << -(ftab[pt_block].size * 4) << endl;   // ����$s7��ַλ��(����ǰ������������С)
            fout << "addi\t$t8, $sp, " << (ftab[pt_block].tsize * 4) << endl;    // ������ľֲ��Ĵ���ֵ����
            i = 0;
            for (map<string, string>::iterator it = registers[pt_block].begin(); it != registers[pt_block].end(); it++) {
                if (it->second != "") {
                    fout << "lw\t$t" << i << ", " << i * 4 << "($t8)" << endl;
                    i++;
                }
            }
            fout << "lw\t$ra, ($s7)" << endl;                                   // ������ĸú������ص�ַ����$ra
        }
        else if (code.a.name == "RET") {
            string t1;
            if (code.b.name == "") {
                if (pt_block == ftab.size() - 1)   // main�������return
                    fout << "j\t.end" << endl;
                else
                    fout << "jr\t$ra" << endl;
            }
            else if (code.b.name[0] == 'T') {
                t1 = registers[pt_block][code.b.name];
                if (t1 == "") {
                    fout << "lw\t$t8, " << stoi(code.b.name.substr(1)) * 4 << "($sp)" << endl;  // ȡ����ʱ������ֵ
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
    while (ch < 0 || isspace(ch)) {   // �ж�Ԥ�����Ƿ�Ϊ�հ��ַ�
        if (ch < 0) {
            isError = true;
            fout << endl << "��" << nexrow << "�У����ڷǷ��ַ�";
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
        cout << "Դ�����д���!" << endl;
        exit(1);
    }
    if (isError) {
        fin.close();
        fout.close();
        cout << "Դ�����д���!" << endl;
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

