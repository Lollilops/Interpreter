#include "Lexical_analyse.cpp"
extern std::vector<Ident> TID;

std::vector<int> labels;
std::vector<int> address_label;
std::vector<int> gotos;

template <class T, class T_EL>
void from_st(T &st, T_EL &i)
{
    i = st.top();
    st.pop();
}
class My_struct
{
public:
    vector<Ident> STID;
    std::string struct_name;
};
std::vector<My_struct> structs;

class Struct_info
{
public:
    std::string st_type;
    std::string st_name;
};
std::vector<Struct_info> info;
class Parser
{
    Lex curr_lex; // текущая лексема scan.get_lex();
    type_of_lex c_type;//curr_lex.get_type();
    int c_val; //curr_lex.get_value();
    std::string c_str; //curr_lex.get_str();
    Scanner scan;
    std::stack<type_of_lex> st_lex;
    std::stack<int> ints_stack;
    type_of_lex type;
    type_of_lex type_const;
    int end;//счетчик скобок
    int goto_address = -1;
    int break_address = -1;
    bool break_flag = false;
    bool goto_flag = false;
    bool lb_flag = false;
    bool cycle_flag = false;

    // процедуры РС-метода
    void P();
    void init_analyze();
    void S();
    void C();
    void H();
    void D();
    void B();
    void O();
    void O1();
    void E();
    void E1();
    void T();
    void F();
    void L();
    void K();
    void J();
    void NN();

    void dec(type_of_lex t); // семантичиеские действия
    void check_init(type_of_lex t, type_of_lex t_c);
    void check_id();
    void check_op(int r, int c);
    void check_not(int r, int c);
    void eq_type(int r, int c);
    void eq_bool(int r, int c);
    void check_id_in_read();
    void gl(); // получить очередную лексему

public:
    std::vector<Lex> poliz; // внутреннее представление программы
    Parser(const char *program) : scan(program), end(0) {}
    void analyze(); // анализатор с действиями
};

void Parser::gl() //считывание новой лексиемы и информации о ней
{
    curr_lex = scan.get_lex();
    c_type = curr_lex.get_type();
    c_val = curr_lex.get_value();
    c_str = curr_lex.get_str();
}

//--------------------------------------------------------------------------------
void new_label(int num)
{
    std::vector<int>::iterator k;
    if ((k = find(labels.begin(), labels.end(), num)) != labels.end())
    {
        string s = "Попытка переопределения метки";
        throw s;
    }
    else
    {
        labels.push_back(num);
    }
}

int new_goto_label(const int buf, const int num)
{
    std::vector<int>::iterator k;
    if ((k = find(gotos.begin(), gotos.end(), buf)) != gotos.end())
    {
        return address_label[k - gotos.begin()];
    }
    if (num != 0)
    {
        gotos.push_back(buf);
        address_label.push_back(num);
    }
    return 0;
}

int checker(const string &name_buf)//говорит является ли идентификатор структурой
{
    for (int it = 0; it < structs.size(); it++)
    {
        if (structs[it].struct_name == name_buf)
        {
            return it;
        }
    }
    cout << name_buf << endl;
    std::string s = "Этот идентификатор не является структурой";
    throw s;
}
//--------------------------------------------------------------------------------

void Parser::analyze() //метод запускающий анализ рекурсивный спуск
{
    gl();
    P();
    if (c_type != LEX_END || end != 0)//проверка на зкрывающуюся скобку в конце
    {
        cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Не хватает закрывающей скобки. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    for (Lex l : poliz)
        cout << l;
    cout << "OK" << endl;
}

void Parser::P()//program { <S;><С;> B}
{
    if (c_type == LEX_PROGRAM)
    {
        gl();
    }
    else
    {
        cout << "Ошибка. Имя правила: P, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Программа должна начинаться со слова program. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    if (c_type == LEX_BEGIN)
    {
        gl();
        end++;
    }
    else
    {
        cout << "Ошибка. Имя правила: P, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): После слова program должна идти открывающая фигурная скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    while (c_type == TYPE_STRUCT)
    {
        S();//инициализация структуры
        gl();
    }
    init_analyze();//инициализация переменных в блоке инициализации
    B();
    if ((c_type == LEX_END) && (end == 1))
    {
        end--;
    }
}
void Parser::init_analyze()//инициализационный анализ, кладет в стек значения переменных и сохраняет в векторы инормацию о переменных в стеке
{
    if (c_type == LEX_INT || c_type == LEX_BOOL || c_type == LEX_STRING)
    {
        while (c_type == LEX_INT || c_type == LEX_BOOL || c_type == LEX_STRING)
        {
            type = c_type;
            gl();
            C();
            if (c_type == LEX_SEMICOLON)
                gl();
            else
            {
                cout << "Ошибка. Имя правила: P, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Каждое описание должно заканчиваться ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        }
    }
    else if (c_type == LEX_ID)
    {
        int row, value;
        std::string struct_type;
        Struct_info new_info;
        row = checker(TID[c_val].get_name());
        struct_type = TID[c_val].get_name();
        new_info.st_type = struct_type;
        gl();
        new_info.st_name = TID[c_val].get_name();
        info.push_back(new_info);
        std::string cur_id;
        cur_id = TID[c_val].get_name();
        ints_stack.push(c_val);
        dec(TYPE_STRUCT);
        for (int i = 0; i < structs[row].STID.size(); i++)
        {
            cur_id = cur_id + '.' + structs[row].STID[i].get_name();
            value = put(cur_id);
            ints_stack.push(value);
            dec(structs[row].STID[i].get_type());
            cur_id = TID[c_val].get_name();
        }
        gl();
        while (c_type == LEX_COMMA)
        {
            gl();
            cur_id = TID[c_val].get_name();
            new_info.st_type = struct_type;
            new_info.st_name = TID[c_val].get_name();
            info.push_back(new_info);
            ints_stack.push(c_val);
            dec(TYPE_STRUCT);
            for (int i = 0; i < structs[row].STID.size(); i++)
            {
                cur_id = cur_id + '.' + structs[row].STID[i].get_name();
                value = put(cur_id);
                ints_stack.push(value);
                dec(structs[row].STID[i].get_type());
                cur_id = TID[c_val].get_name();
            }
            gl();
        }
        if (c_type == LEX_SEMICOLON)
            gl();
        else
        {
            cout << "Ошибка. Имя правила: P, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Каждое описание должно заканчиваться ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type != LEX_BEGIN)
        {
            init_analyze();
        }
    }
    else if (c_type == LEX_END)
    {
        std::string s = "OK";
        throw s;
    }
    else
    {
        std::string s = "ERROR";
        throw s;
    }
}
void Parser::S() //считывание структуры
{
    type_of_lex type_buf;
    Ident buffer;
    My_struct str_buffer;
    gl();
    if (c_type == LEX_ID)
    {
        str_buffer.struct_name = TID[c_val].get_name();
        gl();
    }
    else
    {
        cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): У структуры должно быть имя. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    if (c_type == LEX_BEGIN)
        gl();
    else
    {
        cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Описание структуры должно начинаться с открывающейся фигурной скобки. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    while (c_type != LEX_END)
    {
        if (c_type == LEX_INT || c_type == LEX_BOOL || c_type == LEX_STRING)
        {
            if (c_type == LEX_STRING)
            {
                type_buf = LEX_STR;
            }
            else
            {
                type_buf = c_type;
            }
        }
        else
        {
            cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Неверное поле в структуре. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        gl();
        while (c_type != LEX_SEMICOLON)
        {
            if (c_type == LEX_ID)
            {
                buffer.put_name(TID[c_val].get_name());
                buffer.put_type(type_buf);
                str_buffer.STID.push_back(buffer);
                gl();
            }
            else
            {
                cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Ошибка инициализации. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
            if (c_type == LEX_COMMA)
            {
                gl();
            }
            else if (c_type != LEX_SEMICOLON)
            {
                cout << "Ошибка. Имя правила: S, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Неверный символ при конце инициализации типа. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        }
        gl();
    }
    structs.push_back(str_buffer);//закидываем в вектор структур
}
void Parser::C() //[int | bool | string] H <,H>
{
    H();
    while (c_type == LEX_COMMA)
    {
        gl();
        H();
    }
}
void Parser::H()//I | I = NN
{
    int tmp_val;
    if (c_type == LEX_ID)
    {
        tmp_val = c_val;
        ints_stack.push(tmp_val);
        dec(type);
        gl();
    }
    else
    {
        cout << "Ошибка. Имя правила: H, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Отсутствует идентификатор. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    if (c_type == LEX_ASSIGN)
    {
        poliz.push_back(Lex(POLIZ_ADDRESS, tmp_val));
        gl();
        NN();
        check_init(type, type_const);
    }
}
void Parser::B() //<O>
{
    while (c_type != LEX_END)
        O();
}
void Parser::O()//if(E) O [else O]|while(E)O1|for(C; E; E)O1|read(I);|write(E <,E>);|goto I;|I:O;|{B}|E;
{
    int pl0, pl1, pl2, pl3, pl4, pl5, pl6, pl7;

    if (c_type == LEX_IF)
    {
        gl();
        if (c_type == LEX_LPAREN)//'('
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После слова if должна идти открывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        E();
        eq_bool(tmp_r, tmp_c);
        pl2 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        if (c_type == LEX_RPAREN)//')'
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После выражения в операторе if должна идти закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        O();
        if (c_type == LEX_ELSE)
        {
            pl3 = poliz.size();
            poliz.push_back(Lex());
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl2] = Lex(POLIZ_LABEL, poliz.size());
            gl();
            O();
            poliz[pl3] = Lex(POLIZ_LABEL, poliz.size());
        }
        else
            poliz[pl2] = Lex(POLIZ_LABEL, poliz.size());
    }
    else if (c_type == LEX_WHILE)
    {
        cycle_flag = true;
        pl0 = poliz.size();
        gl();
        if (c_type == LEX_LPAREN)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После слова while должна идти открывающая скобка. Строка %d, столбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        int tmp = poliz.size();
        E();
        pl1 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        eq_bool(tmp_r, tmp_c);
        if (c_type == LEX_RPAREN)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После выражения в операторе while должна идти закрывающая скобка. Строка %d, столбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        O();
        if (break_flag)
        {
            end++;
            break_flag = false;
            poliz[pl1] = Lex(POLIZ_LABEL, poliz.size());
            gl();
        }
        else
        {
            poliz.push_back(Lex(POLIZ_LABEL, pl0));
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl1] = Lex(POLIZ_LABEL, poliz.size());
        }
        cycle_flag = false;
    }
    else if (c_type == LEX_BREAK)
    {
        if (cycle_flag)
        {
            break_flag = true;
            gl();
            if (c_type == LEX_SEMICOLON)
            {
                gl();
            }
            else
            {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): После слова break должна идти ';'. Строка %d, столбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
            while (c_type != LEX_END)
            {
                gl();
            }
            end--;
            cycle_flag = false;
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Break не может быть вне циклов. Строка %d, столбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else if (c_type == LEX_FOR)
    {
        cycle_flag = true;
        gl();
        if (c_type == LEX_LPAREN)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После слова for должна идти открывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type != LEX_SEMICOLON)
        {
            E();
            if (c_type == LEX_SEMICOLON)
            {
                gl();
            }
            else
            {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Обязательно должна идти ';'. Строка %d, столбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        }
        else
        {
            gl();
        }
        pl6 = int(poliz.size());
        if (c_type != LEX_SEMICOLON)
        {
            E();
            if (c_type == LEX_SEMICOLON)
            {
                gl();
            }
            else
            {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Обязательно должна идти ';'. Строка %d, столбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        }
        else
        {
            gl();
        }
        pl4 = int(poliz.size());
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        pl5 = int(poliz.size());
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_GO));
        pl7 = int(poliz.size());
        if (c_type != LEX_SEMICOLON && c_type != LEX_RPAREN)
        {
            E();
            if (c_type == LEX_RPAREN)
            {
                gl();
            }
            else
            {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): После всех аргументов for должна идти закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        }
        poliz.push_back(Lex(POLIZ_LABEL, pl6));
        poliz.push_back(Lex(POLIZ_GO));
        poliz[pl5] = Lex(POLIZ_LABEL, int(poliz.size()));
        O();
        if (break_flag)
        {
            end++;
            break_flag = false;
            poliz[pl4] = Lex(POLIZ_LABEL, poliz.size());
            gl();
        }
        else
        {
            poliz.push_back(Lex(POLIZ_LABEL, pl7));
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl4] = Lex(POLIZ_LABEL, int(poliz.size()));
        }
        cycle_flag = false;
    }
    else if (c_type == LEX_GOTO)
    {
        gl();
        if (c_type == LEX_ID)
        {
            int buf1 = new_goto_label(c_val, 0);
            if (buf1 != 0)
            {
                poliz.push_back(Lex(POLIZ_LABEL, buf1));
                poliz.push_back(Lex(POLIZ_GO));
            }
            else
            {
                new_goto_label(c_val, int(poliz.size()));
                poliz.push_back(Lex());
                poliz.push_back(Lex(POLIZ_GO));
            }
            gl();
            if (c_type == LEX_SEMICOLON)
            {
                gl();
            }
            else
            {
                cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): После goto должна идти ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ошибка метки в goto. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else if (c_type == LEX_READ)
    {
        gl();
        if (c_type == LEX_LPAREN)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После слова read должна идти открывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type == LEX_ID)
        {
            check_id_in_read();
            poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): В операторе read может стоять только идентификатор. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type == LEX_RPAREN)
        {
            poliz.push_back(Lex(LEX_READ));
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После идентификатора в операторе read должна идти закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type == LEX_SEMICOLON)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После закрывающей скобки в операторе read должна идти ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else if (c_type == LEX_WRITE)
    {
        gl();
        if (c_type == LEX_LPAREN)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После слова write должна идти открывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type == LEX_ID)
        {
            for (int it = 0; it < info.size(); it++)
            {
                if (info[it].st_name == TID[c_val].get_name())
                {
                    cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                    char s[BUFSIZE];
                    sprintf(s, "%s (%d): Попытка вывести структуру. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                    string str = string(s);
                    throw str;
                }
            }
        }
        E();
        poliz.push_back(Lex(LEX_WRITE));
        while (c_type == LEX_COMMA)
        {
            gl();
            E();
            poliz.push_back(Lex(LEX_WRITE));
        }
        if (c_type == LEX_RPAREN)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После выражений в операторе write должна идти закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        if (c_type == LEX_SEMICOLON)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После закрывающей скобки в операторе write должна идти ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else if (c_type == LEX_BEGIN)
    {
        gl();
        end++;
        B();
        if (c_type == LEX_END)
        {
            end--;
            if (end == 0)
            {
                for (Lex l : poliz)
                    cout << l;

                /*std::string s = "OK";
                throw s; */
                cout << "OK" << endl;
            }
            else
            {
                gl();
            }
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Нарушен баланс фигурных скобок. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else
    {
        E();
        if (c_type == LEX_SEMICOLON)
        {
            poliz.push_back(curr_lex);
            gl();
        }
        else if (lb_flag)
        {
            lb_flag = false;
        }
        else
        {
            cout << "Ошибка. Имя правила: O, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): После выражения должна идти ';'. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
}

void Parser::E()
{
    int p;
    std::string buf_name;
    int flag = -2;
    if (c_type == LEX_ID)
    {
        buf_name = TID[c_val].get_name();
        flag = -1;
        for (int it = 0; it < info.size(); it++)
        {
            if (info[it].st_name == buf_name)
            {
                flag = it;
            }
        }
    }
    if (flag == -1)
    {
        p = poliz.size();
        E1();
        if (c_type == LEX_ASSIGN)
        {
            poliz[p].put_type(POLIZ_ADDRESS);
            st_lex.push(c_type);
            int tmp_r = scan.row;
            int tmp_c = scan.column;
            gl();
            E();
            eq_type(tmp_r, tmp_c);
        }
    }
    else if (flag != -2)
    {
        std::string type1 = info[flag].st_type;
        std::string name1 = info[flag].st_name;
        std::string type2;
        std::string name2;
        std::vector<Ident> IDS;
        std::string cur_name1, cur_name2;
        int i1, i2;
        for (int it = 0; it < structs.size(); it++)
        {
            if (structs[it].struct_name == type1)
            {
                IDS = structs[it].STID;
            }
        }
        gl();
        if (c_type == LEX_SEMICOLON)
        {
            return;
        }
        if (c_type != LEX_ASSIGN)
        {
            cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Неверная операция над цельными структурами. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        gl();
        buf_name = TID[c_val].get_name();
        flag = -1;
        for (int it = 0; it < info.size(); it++)
        {
            if (info[it].st_name == buf_name)
            {
                flag = it;
                break;
            }
        }
        if (flag == -1)
        {
            cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Неверный идентификатор справа от знака равенства. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        else
        {
            type2 = info[flag].st_type;
            name2 = info[flag].st_name;
            if (type1 != type2)
            {
                cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Попытка присваивания структур различных типов. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
            for (int i = 0; i < IDS.size(); i++)
            {
                p = poliz.size();
                cur_name1 = name1 + '.' + IDS[i].get_name();
                cur_name2 = name2 + '.' + IDS[i].get_name();
                i1 = find(TID.begin(), TID.end(), cur_name1) - TID.begin();
                i2 = find(TID.begin(), TID.end(), cur_name2) - TID.begin();
                TID[i1].put_assign();
                poliz.push_back(Lex(LEX_ID, i1));
                st_lex.push(TID[i1].get_type());
                poliz[p].put_type(POLIZ_ADDRESS);
                st_lex.push(LEX_ASSIGN);
                poliz.push_back(Lex(LEX_ID, i2));
                st_lex.push(TID[i2].get_type());
                poliz.push_back(LEX_ASSIGN);
                poliz.push_back(Lex(LEX_SEMICOLON));
            }
            poliz.pop_back();
            gl();
        }
    }
    else
    {
        E1();
    }
}

void Parser::E1()
{
    T();
    while (c_type == LEX_OR)
    {
        st_lex.push(c_type);
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        gl();
        T();
        check_op(tmp_r, tmp_c);
    }
}

void Parser::T()
{
    F();
    while (c_type == LEX_AND)
    {
        st_lex.push(c_type);
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        gl();
        F();
        check_op(tmp_r, tmp_c);
    }
}

void Parser::F()
{
    L();
    while (c_type == LEX_EQ || c_type == LEX_LSS || c_type == LEX_GTR ||
           c_type == LEX_LEQ || c_type == LEX_GEQ || c_type == LEX_NEQ)
    {
        st_lex.push(c_type);
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        gl();
        L();
        check_op(tmp_r, tmp_c);
    }
}

void Parser::L()
{
    K();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS)
    {
        st_lex.push(c_type);
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        gl();
        K();
        check_op(tmp_r, tmp_c);
    }
}

void Parser::K()
{
    J();
    while (c_type == LEX_TIMES || c_type == LEX_SLASH || c_type == LEX_PERCENT)
    {
        st_lex.push(c_type);
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        gl();
        J();
        check_op(tmp_r, tmp_c);
    }
}
void Parser::J()
{
    if (c_type == LEX_ID)
    {
        if (!TID[c_val].get_declare())
        {
            int buf7 = c_val;
            gl();
            if (c_type == LEX_COLON)
            {
                lb_flag = true;
                new_label(buf7);
                int buf8;
                if ((buf8 = new_goto_label(buf7, int(poliz.size()))) != 0)
                {
                    int buf9 = int(poliz.size());
                    poliz[buf8] = Lex(POLIZ_LABEL, buf9);
                }
                gl();
                O();
            }
            else
            {
                cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Идентификатор не описан. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
                string str = string(s);
                throw str;
            }
        }
        else
        {
            poliz.push_back(curr_lex);
            st_lex.push(TID[c_val].get_type());
            gl();
        }
    }
    else if (c_type == LEX_NOT)
    {
        int tmp_r = scan.row;
        int tmp_c = scan.column;
        gl();
        J();
        check_not(tmp_r, tmp_c);
    }
    else if (c_type == LEX_LPAREN)
    {
        gl();
        E();
        if (c_type == LEX_RPAREN)
        {
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: J, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалась закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else
    {
        NN();
    }
}

void Parser::NN()
{
    if (c_type == LEX_MINUS)
    {
        gl();
        if (c_type == LEX_NUM)
        {
            type_const = c_type;
            st_lex.push(LEX_INT);
            poliz.push_back(Lex(curr_lex.get_type(), -curr_lex.get_value()));
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: NN, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалось число. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else if (c_type == LEX_PLUS)
    {
        gl();
        if (c_type == LEX_NUM)
        {
            type_const = c_type;
            st_lex.push(LEX_INT);
            poliz.push_back(curr_lex);
            gl();
        }
        else
        {
            cout << "Ошибка. Имя правила: NN, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Ожидалось число. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else if (c_type == LEX_NUM)
    {
        type_const = c_type;
        st_lex.push(LEX_INT);
        poliz.push_back(curr_lex);
        gl();
    }
    else if (c_type == LEX_STR)
    {
        type_const = c_type;
        st_lex.push(LEX_STRING);
        poliz.push_back(curr_lex);
        gl();
    }
    else if (c_type == LEX_TRUE)
    {
        type_const = c_type;
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_TRUE, 1));
        gl();
    }
    else if (c_type == LEX_FALSE)
    {
        type_const = c_type;
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_FALSE, 0));
        gl();
    }
    else
    {
        cout << "Ошибка. Имя правила: NN, тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Недопустимое выражение. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
}

void Parser::dec(type_of_lex t)
{
    int i;
    while (!ints_stack.empty())
    {
        from_st(ints_stack, i);
        if (TID[i].get_declare())
        {
            cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Повторное описание идентификатора. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
        else
        {
            TID[i].put_declare();
            if (t == LEX_INT)
                TID[i].put_type(LV_INT);
            else if (t == LEX_BOOL)
                TID[i].put_type(LV_BOOL);
            else
                TID[i].put_type(LV_STRING);
        }
    }
}
void Parser::check_init(type_of_lex t, type_of_lex t_c)
{
    if (t == LEX_BOOL)
    {
        if (t_c != LEX_TRUE && t_c != LEX_FALSE)
        {
            cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Несовпадение типов при инициализации. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    else if (t == LEX_INT && t_c != LEX_NUM)
    {
        cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Несовпадение типов при инициализации. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    else if (t == LEX_STRING && t_c != LEX_STR)
    {
        cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Несовпадение типов при инициализации. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    poliz.push_back(Lex(LEX_ASSIGN));
    poliz.push_back(Lex(LEX_SEMICOLON));
}

void Parser::check_id_in_read()
{
    for (int it = 0; it < info.size(); it++)
    {
        if (info[it].st_name == TID[c_val].get_name())
        {
            cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Попытка ввести структуру. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
            string str = string(s);
            throw str;
        }
    }
    if (!TID[c_val].get_declare())
    {
        cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Идентификатор не описан. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
}

void Parser::check_id()
{
    if (!TID[c_val].get_declare())
    {
        cout << "Ошибка. Тип лексемы: " << c_type << ", значение лексемы: " << curr_lex;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Идентификатор не описан. Строка %d, cтолбец %d\n", __FILE__, __LINE__, scan.row, scan.column);
        string str = string(s);
        throw str;
    }
    poliz.push_back(curr_lex);
}

void Parser::check_op(int r, int c)
{
    type_of_lex t1, t2, op, t = LEX_INT, rez = LEX_BOOL;

    from_st(st_lex, t2);
    from_st(st_lex, op);
    from_st(st_lex, t1);

    if (op == LEX_MINUS || op == LEX_TIMES || op == LEX_SLASH || op == LEX_PERCENT)
    {
        rez = LEX_INT;
    }
    else if (op == LEX_PLUS)
    {
        if (t1 == LEX_INT || t1 == LV_INT)
        {
            rez = LEX_INT;
        }
        else
        {
            rez = LEX_STRING;
            t = LEX_STRING;
        }
    }
    else if (op == LEX_LSS || op == LEX_GTR || op == LEX_EQ || op == LEX_NEQ)
    {
        if (t1 == LEX_INT || t1 == LV_INT)
        {
            t = LEX_INT;
        }
        else
        {
            t = LEX_STRING;
        }
    }
    else if (op == LEX_OR || op == LEX_AND)
    {
        t = LEX_BOOL;
    }

    if (t == LEX_INT && (t1 != LEX_INT && t1 != LV_INT || t2 != LEX_INT && t2 != LV_INT))
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Неверные типы в двухместной операции: %d. Строка %d, cтолбец %d\n", __FILE__, __LINE__, op, r, c);
        string str = string(s);
        throw str;
    }
    else if (t == LEX_BOOL && (t1 != LEX_BOOL && t1 != LV_BOOL || t2 != LEX_BOOL && t2 != LV_BOOL))
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Неверные типы в двухместной операции: %d. Строка %d, cтолбец %d\n", __FILE__, __LINE__, op, r, c);
        string str = string(s);
        throw str;
    }
    else if (t == LEX_STRING && (t1 != LEX_STRING && t1 != LV_STRING || t2 != LEX_STRING && t2 != LV_STRING))
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Неверные типы в двухместной операции: %d. Строка %d, cтолбец %d\n", __FILE__, __LINE__, op, r, c);
        string str = string(s);
        throw str;
    }
    st_lex.push(rez);
    poliz.push_back(Lex(op));
}

void Parser::check_not(int r, int c)
{
    if (st_lex.top() != LEX_BOOL && st_lex.top() != LV_BOOL)
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Неверные типы в операции not. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }
    st_lex.pop();
    st_lex.push(LEX_BOOL);
    poliz.push_back(Lex(LEX_NOT));
}

void Parser::eq_type(int r, int c)
{
    type_of_lex t2, op, t1;
    from_st(st_lex, t2);
    from_st(st_lex, op);
    from_st(st_lex, t1);
    if (t1 != LV_BOOL && t1 != LV_INT && t1 != LV_STRING)
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Не L-value выражение в операции '='. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }

    if (t1 == LV_INT && t2 != LEX_INT && t2 != LV_INT)
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Неверные типы в операции '='. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }
    else if (t1 == LV_BOOL && t2 != LEX_BOOL && t2 != LV_BOOL)
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Неверные типы в операции '='. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }
    else if (t1 == LV_STRING && t2 != LEX_STRING && t2 != LV_STRING)
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Неверные типы в операции '='. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }
    st_lex.push(t1);
    poliz.push_back(Lex(op));
}

void Parser::eq_bool(int r, int c)
{
    if (st_lex.top() != LEX_BOOL && st_lex.top() != LV_BOOL)
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): В операторах if, while и for выражение должно быть логического типа. Строка %d, cтолбец %d\n", __FILE__, __LINE__, r, c);
        string str = string(s);
        throw str;
    }
    st_lex.pop();
}