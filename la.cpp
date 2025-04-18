#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <ctime>
#include <time.h>
using namespace std;

#define BUFSIZE 256
enum type_of_lex
{
    LEX_NULL, /*0*/
    LEX_AND,
    LEX_BOOL,
    LEX_DO,
    LEX_ELSE,
    LEX_IF,
    LEX_FALSE,
    LEX_INT,
    LEX_NOT,
    LEX_OR,
    LEX_PROGRAM, /*10*/
    LEX_READ,
    LEX_STRING,
    LEX_TRUE,
    LEX_GOTO,
    LEX_BREAK,
    LEX_WHILE,
    LEX_WRITE,
    LEX_FOR,     /*18*/
    TYPE_STRUCT, /*19*/
    LEX_QUOTE,   /*20*/
    LEX_BEGIN,
    /*21*/ //'{'
    LEX_END,
    LEX_SEMICOLON, //;
    LEX_COMMA,
    LEX_COLON,
    LEX_NEQ,    //'!='
    LEX_ASSIGN, // присвоить
    LEX_LPAREN,
    /*28*/ //'('
    LEX_RPAREN,
    LEX_EQ, //"="
    LEX_LSS,
    LEX_GTR,
    LEX_PLUS,
    LEX_MINUS,
    LEX_TIMES,
    LEX_SLASH,
    LEX_LEQ,
    LEX_GEQ, /*38*/
    LEX_PERCENT,
    LEX_NUM, /*40*/
    LEX_STR, /*41*/
    LEX_ID,
    POLIZ_LABEL,
    POLIZ_ADDRESS,
    POLIZ_GO,
    POLIZ_FGO,
    LV_STRING,
    LV_INT,
    LV_BOOL
};

class Ident // класс работающий с идентификаторами составляется TID таблица идентификаторов
{
    string name;
    string string_value;
    bool declare;
    type_of_lex type;
    bool assign;
    int value;

public:
    Ident()
    {
        declare = false;
        assign = false;
    }
    bool operator==(const string &s) const
    {
        return name == s;
    }
    Ident(const string new_str)
    {
        name = new_str;
        declare = false;
        assign = false;
    }
    string get_name() const
    {
        return name;
    }
    void put_name(string new_name)
    {
        name = new_name;
    }
    bool get_declare() const
    {
        return declare;
    }
    void put_declare()
    {
        declare = true;
    }
    type_of_lex get_type() const
    {
        return type;
    }
    void put_type(type_of_lex t)
    {
        type = t;
    }
    bool get_assign() const
    {
        return assign;
    }
    void put_assign()
    {
        assign = true;
    }
    int get_value() const
    {
        return value;
    }
    void put_value(int val)
    {
        value = val;
    }
    string get_str_val() const
    {
        return string_value;
    }
    void put_str_val(string str)
    {
        string_value = str;
    }
};

class Lex // TODO класс, хранящий в себе лексему, в данный момент только возвращается в виде временного объекта планируется обрабатывать лексемы
{
    type_of_lex t_lex;
    int v_lex;
    string s_lex;

public:
    Lex(type_of_lex t = LEX_NULL, int v = 0) : t_lex(t), v_lex(v), s_lex("") {}
    Lex(type_of_lex t, string str1, int v = 0) : t_lex(t), v_lex(v), s_lex(str1) {}
    type_of_lex get_type() const
    {
        return t_lex;
    }
    int get_value() const
    {
        return v_lex;
    }
    string get_str() const
    {
        return s_lex;
    }
    void put_type(type_of_lex t)
    {
        t_lex = t;
    }
    friend ostream &operator<<(ostream &s, Lex l);
};

class Scanner // сканирует лексемы и возвращает объект Lex
{
    FILE *fp;
    char c;
    int search(const string buf, const char **list);
    void gc();

public:
    int row, column;
    static int balance; // баланс скобок
    // bool st_start = false;
    static bool lex_flag;           // флаг того что лексер закончил работу
    static const char *TW[], *TD[]; // определены ниже таблицы служебных слов и ограничителей
    Scanner(const char *program);   // создание класса сканер, подаем на вход имя файла
    Lex get_lex();
    ~Scanner() { fclose(fp); }
};
vector<Ident> TID; // таблица идентификаторов анализируемой программы

int Scanner::balance = 0;
bool Scanner::lex_flag = true;

// таблица служебных слов модельного языка
const char *
    Scanner::TW[] = {"" /*0*/, "and" /*1*/, "bool" /*2*/, "do" /*3*/, "else" /*4*/, "if" /*5*/, "false" /*6*/, "int" /*7*/, "not" /*8*/, "or" /*9*/, "program" /*10*/,
                     "read" /*11*/, "string" /*12*/, "true" /*13*/, "goto" /*14*/, "break" /*15*/, "while" /*16*/, "write" /*17*/, "for" /*18*/, "struct" /*19*/, NULL};

// таблица ограничителей модельного языка
const char *
    Scanner::TD[] = {"\"" /*0*/, "{" /*1*/, "}" /*2*/, ";" /*3*/, "," /*4*/, ":" /*5*/, "!=" /*6*/, "=" /*7*/, "(" /*8*/, ")" /*9*/,
                     "==" /*10*/, "<" /*11*/, ">" /*12*/, "+" /*13*/, "-" /*14*/, "*" /*15*/, "/" /*16*/, "<=" /*17*/, ">=" /*18*/, "%" /*19*/, NULL};

int Scanner::search(const string buf, const char **list) // функция ищет служебное слово или ограничитель в TW или TD
{
    int i = 0;
    while (list[i])
    {
        if (buf == list[i])
            return i;
        ++i;
    }
    return 0;
}

void Scanner::gc()
{
    c = fgetc(fp);
    if ((c == EOF) && (balance != 0)) // пока баланс не будет равен 0
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Не хватает закрывающей скобки. Строка %d, cтолбец %d\n", __FILE__, __LINE__, row, column);
        string str = string(s);
        throw str;
    }
}

Scanner::Scanner(const char *program) : row(1), column(0)
{
    if (!(fp = fopen(program, "r")))
    {
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Не удалось открыть файл\n", __FILE__, __LINE__);
        string str = string(s);
        throw str;
    }
}

int put(const string &buf)
{
    vector<Ident>::iterator k;

    if ((k = find(TID.begin(), TID.end(), buf)) != TID.end())
        return k - TID.begin();
    TID.push_back(Ident(buf));
    return TID.size() - 1;
}

Lex Scanner::get_lex()
{
    enum state
    {
        H, // начальное состояние
        IDENT,
        ST_IDENT,  //! пока лишнее
        NUMB,      // состояние считывания чисел
        SLASH,     // либо деление либо комментарий
        COM,       // начало коментария
        COM_TIMES, // ждем конец коментария
        ALE,       // знаки сравнения =<>
        NEQ,       //"!=""
        STR,       // состояние считывания строки
        START,     //! пока лишнее
        FINISH     //!=пока лишнее
    };
    int d, j;
    string buf;  // хранит в себе лексему до момента ее распознавания в одном из состояний
    string buf1; //! пока лишнее
    state CS = H;
    do
    {
        column++;
        gc(); // проверка баланса скобок и конца файла
        switch (CS)
        {
        case H:
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') // пропускаем лишние символы или если нашли \n то переходи на новую строку
            {
                if (c == '\n')
                {
                    column = 0;
                    row++;
                }
            }
            else if (c == '{')
            {
                balance++;
                return Lex(LEX_BEGIN, '{');
            }
            else if (c == '}')
            {
                // if (st_start == true)
                // {
                //     st_start = false;
                // }
                balance--;
                cout << balance << endl;
                if (balance < 0)
                {
                    char s[BUFSIZE];
                    sprintf(s, "%s (%d): Непонятная закрывающая скобка. Строка %d, cтолбец %d\n", __FILE__, __LINE__, row, column);
                    string str = string(s);
                    throw str;
                }
                if ((balance == 0) && (lex_flag))
                {
                    string s = "OK_LETS_GO";
                    lex_flag = false;
                    throw s;
                }
                return Lex(LEX_END, '}');
            }
            else if (/*isalpha(c)*/ islower(c))
            {
                buf.push_back(c);
                CS = IDENT;
            }
            else if (isdigit(c))
            {
                d = c - '0';
                CS = NUMB;
            }
            else if (c == '/')
            {
                buf.push_back(c);
                CS = SLASH;
            }
            else if (c == '=' || c == '<' || c == '>')
            {
                buf.push_back(c);
                CS = ALE;
            }
            else if (c == '!')
            {
                buf.push_back(c);
                CS = NEQ;
            }
            else if (c == '"')
            {
                CS = STR;
            }
            else
            {
                buf.push_back(c);
                if ((j = search(buf, TD)))
                {
                    return Lex((type_of_lex)(j + (int)LEX_QUOTE), j);
                }
            }
            break;
        case IDENT:
            if (isalpha(c) /*(c>='a'&& c<='z')||(c>='A'&& c<='Z')*/ || isdigit(c) || c == '.')
            {
                buf.push_back(c);
            }
            else
            {
                ungetc(c, fp);
                column--;
                if ((j = search(buf, TW)))
                {
                    return Lex((type_of_lex)j, j);
                }
                else
                {
                    j = put(buf); // TODO лишние символы
                    return Lex(LEX_ID, j);
                }
            }
            break;
        case NUMB:
            if (isdigit(c))
            {
                d = d * 10 + (c - '0');
            }
            else
            {
                ungetc(c, fp);
                column--;
                return Lex(LEX_NUM, d);
            }
            break;
        case SLASH:
            if (c == '*')
            {
                CS = COM;
                buf.pop_back();
            }
            else
            {
                ungetc(c, fp);
                column--;
                j = search(buf, TD);
                return Lex(LEX_SLASH, j);
            }
            break;
        case COM:
            if (c == '*')
                CS = COM_TIMES;
            break;
        case COM_TIMES:
            if (c == '/')
                CS = H;
            else
                CS = COM;
            break;
        case ALE:
            if (c == '=')
            {
                buf.push_back(c);
                j = search(buf, TD);
                return Lex((type_of_lex)(j + (int)LEX_QUOTE), j);
            }
            else
            {
                ungetc(c, fp);
                column--;
                j = search(buf, TD);
                return Lex((type_of_lex)(j + (int)LEX_QUOTE), j);
            }
            break;
        case NEQ:
            if (c == '=')
            {
                buf.push_back(c);
                j = search(buf, TD);
                return Lex(LEX_NEQ, j);
            }
            else
            {
                char s[BUFSIZE];
                sprintf(s, "%s (%d): После '!' должно идти '='. Строка %d, cтолбец %d\n", __FILE__, __LINE__, row, column);
                string str = string(s);
                throw str;
            }
            break;
        case STR:
            if (c != '"')
            {
                buf.push_back(c);
            }
            else
                return Lex(LEX_STR, buf);
            break;
        } // end switch
    } while (true);
}
ostream &operator<<(ostream &s, Lex l)
{
    string t;
    if (l.t_lex <= TYPE_STRUCT)
        t = Scanner::TW[l.t_lex];
    else if (l.t_lex >= LEX_QUOTE && l.t_lex <= LEX_PERCENT)
        t = Scanner::TD[l.t_lex - LEX_QUOTE];
    else if (l.t_lex == LEX_NUM)
        t = "NUMB";
    else if (l.t_lex == LEX_ID)
        t = TID[l.v_lex].get_name();
    else if (l.t_lex == LEX_STR)
    {
        s << "STR," << l.s_lex << endl;
        return s;
    }
    else if (l.t_lex == POLIZ_LABEL)
        t = "Label";
    else if (l.t_lex == POLIZ_ADDRESS)
        t = "Addr";
    else if (l.t_lex == POLIZ_GO)
        t = "!";
    else if (l.t_lex == POLIZ_FGO)
        t = "!F";
    else
    {
        cout << l;
        char s[BUFSIZE];
        sprintf(s, "%s (%d): Ошибка печати, недопустимая лексема\n", __FILE__, __LINE__);
        string str = string(s);
        throw str;
    }
    s << t << ',' << l.v_lex << endl;
    return s;
}

// extern vector<Ident> TID;
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

vector<int> labels;
vector<int> address_label;
vector<int> gotos;

template <class T, class T_EL>
void from_st(T &st, T_EL &i) // достает из стека
{
    i = st.top();
    st.pop();
}
class My_struct
{
public:
    vector<Ident> STID;
    string struct_name;
};
vector<My_struct> structs;

class Struct_info
{
public:
    string st_type;
    string st_name;
};
vector<Struct_info> info;

// P  -> program { <S;><С;> B}
// S  -> struct I { <C;> }
// С  -> [int | bool | string] H <,H>
// H  -> I | I = NN
// B  -> <O>
// O  ->   if(E) O [else O] 	|
//         while(E) O1 	|
//         for(C; E; E) O1	|
//         read(I); 	|
//         write(E <,E>);	|
//         goto I;   |
//         I:O;      |
//         {B} 	  |
//         E;
// O1 -> O|break;
// E  -> E1 = E | E1
// E1 -> T < or T>
// T  -> F < and F>
// F  -> L < [ == | < | > | <= | >= | != ] L>
// L  -> K < [ + | - ] K>
// K  -> J < [ * | / | % ] J>
// J  -> I | NN | not J | (E) | D

// NN -> [-|+] N | "S" | true | false
// I  -> almost any combination of English letters, numbers and one dot (optional) 		LEX_ID
// N  -> 0|1|...|9|N0|N1|...|N9			    LEX_NUM
// S  -> any combinations of symbols

class Parser
{
    Lex curr_lex;       // текущая лексема scan.get_lex();
    type_of_lex c_type; // curr_lex.get_type();
    int c_val;          // curr_lex.get_value();
    string c_str;       // curr_lex.get_str();
    Scanner scan;
    stack<type_of_lex> st_lex;
    stack<int> ints_stack;
    type_of_lex type;
    type_of_lex type_const;
    int end; // счетчик скобок
    int goto_address = -1;
    int break_address = -1;
    bool goto_flag = false;
    bool lb_flag = false;
    int cycle_flag = 0;

    // процедуры РС-метода
    void P();
    void init_analyze();
    void S();
    void C();
    void H();
    // void D();
    void B();
    void O();
    // void O1();
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
    vector<Lex> poliz; // внутреннее представление программы
    Parser(const char *program) : scan(program), end(0) {}
    void analyze(); // анализатор с действиями
    vector<int> break_counter;
};

void Parser::gl() // считывание новой лексиемы и информации о ней
{
    curr_lex = scan.get_lex();
    c_type = curr_lex.get_type();
    c_val = curr_lex.get_value();
    c_str = curr_lex.get_str();
}

//--------------------------------------------------------------------------------
void new_label(int num) //
{
    vector<int>::iterator k;
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
    vector<int>::iterator k;
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

int checker(const string &name_buf) // говорит является ли идентификатор структурой
{
    for (int it = 0; it < structs.size(); it++)
    {
        if (structs[it].struct_name == name_buf)
        {
            return it;
        }
    }
    cout << name_buf << endl;
    string s = "Этот идентификатор не является структурой";
    throw s;
}
//--------------------------------------------------------------------------------

void Parser::analyze() // метод запускающий анализ рекурсивный спуск
{
    gl();
    P();
    if (c_type != LEX_END || end != 0) // проверка на зкрывающуюся скобку в конце
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

void Parser::P() // program { <S;><С;> B}
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
        S(); // инициализация структуры
        gl();
    }
    init_analyze(); // инициализация переменных в блоке инициализации
    B();
    if ((c_type == LEX_END) && (end == 1))
    {
        end--;
    }
}
void Parser::init_analyze() // инициализационный анализ, кладет в стек значения переменных и сохраняет в векторы инормацию о переменных в стеке
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
        string struct_type;
        Struct_info new_info;
        row = checker(TID[c_val].get_name());
        struct_type = TID[c_val].get_name();
        new_info.st_type = struct_type;
        gl();
        new_info.st_name = TID[c_val].get_name();
        info.push_back(new_info);
        string cur_id;
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
        string s = "OK";
        throw s;
    }
    //неотлаженный участок мб проблемы
    // else if (c_type != LEX_END)
    //     return;
    else
    {
        string s = "ERROR";
        throw s;
    }
}
void Parser::S() // считывание структуры
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
    structs.push_back(str_buffer); // закидываем в вектор структур
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
void Parser::H() // I | I = NN
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

void Parser::O() // if(E) O [else O]|while(E)O1|for(C; E; E)O1|read(I);|write(E <,E>);|goto I;|I:O;|{B}|E;
{
    int pl0, pl1, pl2, pl3, pl4, pl5, pl6, pl7;

    if (c_type == LEX_IF)
    {
        gl();
        if (c_type == LEX_LPAREN) //'('
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
        if (c_type == LEX_RPAREN) //')'
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
        cycle_flag++;
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
        /**/ break_counter.push_back(-1);
        O();
        poliz.push_back(Lex(POLIZ_LABEL, pl0));
        poliz.push_back(Lex(POLIZ_GO));
        poliz[pl1] = Lex(POLIZ_LABEL, poliz.size());
        /**/
        while (break_counter.back() > 0)
        {
            // cout << "+++++++++++++++\n";
            int k = break_counter.back();
            break_counter.pop_back();
            poliz[k] = Lex(POLIZ_LABEL, poliz.size());
        }
        break_counter.pop_back();
        /**/
        cycle_flag--;
    }
    else if (c_type == LEX_BREAK)
    {
        if (cycle_flag)
        { // Я ЗДЕСЬ
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
            // end--;
            //  cycle_flag = false; ХЗХЗХЗ
            break_counter.push_back(poliz.size());
            poliz.push_back(Lex());
            poliz.push_back(Lex(POLIZ_GO));
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
        cycle_flag++;
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
        /**/ break_counter.push_back(-1);
        O();
        poliz.push_back(Lex(POLIZ_LABEL, pl7));
        poliz.push_back(Lex(POLIZ_GO));
        poliz[pl4] = Lex(POLIZ_LABEL, int(poliz.size()));
        cycle_flag--;
        /**/
        while (break_counter.back() > 0)
        {
            // cout << "----------------\n";
            int k = break_counter.back();
            break_counter.pop_back();
            poliz[k] = Lex(POLIZ_LABEL, poliz.size());
        }
        break_counter.pop_back();
        /**/
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
    string buf_name;
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
        string type1 = info[flag].st_type;
        string name1 = info[flag].st_name;
        string type2;
        string name2;
        vector<Ident> IDS;
        string cur_name1, cur_name2;
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
//
//
//
//
//
//
//
//
// Функция void Parser::dec (type_of_lex type) считывает из стека номера строк таблицы
// TID, заносит в них информацию о типе соответствующих переменных, о наличии их описаний и контролирует повторное описание переменных
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

void Parser::check_init(type_of_lex t, type_of_lex t_c) // проверка на совпадение типов при инициализации
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

void Parser::check_id_in_read() // проверка при вызове read был ли задекларирован вводимый идентификатор
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

// Если операнд — лексема-переменная, то необходимо проверить, описана ли она; если
// описана, то ее тип надо занести в стек. Эти действия можно выполнить с помощью функции
// check_id():
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
// Тогда для контроля контекстных условий каждой тройки — операнд – операция –
// операнд (для проверки соответствия типов операндов данной двуместной операции) будем
// использовать функцию check_op():
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
// Для контроля за типом операнда одноместной операции not будем использовать
// функцию check_not():
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

// В результате контроля контекстных условий выражения E в стеке останется тип этого выражения (как тип результата последней операции); если при анализе идентификатора I
// проверить, описан ли он, и занести его тип в тот же стек (для этого можно использовать
// функцию check_id() ), то достаточно будет в нужный момент считать из стека два элемента и
// сравнить их:
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

// В результате контроля контекстных условий выражения E в стеке останется тип этого
// выражения (как тип результата последней операции); следовательно, достаточно извлечь его
// из стека и проверить:
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

class Executer
{
public:
    void execute(std::vector<Lex> &poliz);
};

class Interpretator
{
    Parser pars;
    Executer E;

public:
    Interpretator(const char *program) : pars(program) {}
    void interpretation();
};
void Executer::execute(vector<Lex> &poliz)
{
    Lex pc_el;
    stack<Lex> args;
    Lex i, j;
    int index = 0, size = poliz.size();
    int tmp;
    while (index < size)
    {
        pc_el = poliz[index];
        switch (pc_el.get_type())
        {
        case LEX_TRUE:
        case LEX_FALSE:
        case LEX_NUM:
        case LEX_STR:
        case POLIZ_ADDRESS:
        case POLIZ_LABEL:
            args.push(pc_el);
            break;
        case LEX_ID:
            tmp = pc_el.get_value();
            if (TID[tmp].get_assign())
            {
                if (TID[tmp].get_type() == LV_STRING)
                    args.push(Lex(TID[tmp].get_type(), TID[tmp].get_str_val()));
                else
                    args.push(Lex(TID[tmp].get_type(), TID[tmp].get_value()));
                break;
            }
            else
            {
                cout << "Ошибка. Лексема: " << pc_el;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Неопределенный идентификатор\n", __FILE__, __LINE__);
                string str = string(s);
                throw str;
            }
        case LEX_NOT:
            from_st(args, i);
            args.push(Lex(i.get_type(), !i.get_value()));
            break;
        case LEX_OR:
            from_st(args, i);
            from_st(args, j);
            args.push(Lex(i.get_type(), j.get_value() || i.get_value()));
            break;
        case LEX_AND:
            from_st(args, i);
            from_st(args, j);
            args.push(Lex(i.get_type(), j.get_value() && i.get_value()));
            break;
        case POLIZ_GO:
            from_st(args, i);
            index = i.get_value() - 1;
            break;
        case POLIZ_FGO:
            from_st(args, i);
            from_st(args, j);
            if (j.get_value() == 0)
                index = i.get_value() - 1;
            break;
        case LEX_WRITE:
            from_st(args, i);
            if (i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR)
                cout << i.get_str() << endl;
            else if (i.get_type() == LEX_INT || i.get_type() == LV_INT || i.get_type() == LEX_NUM)
                cout << i.get_value() << endl;
            else
            {
                if (i.get_value() == 0)
                    cout << "false" << endl;
                else
                    cout << "true" << endl;
            }
            break;
        case LEX_READ:
            int k;
            from_st(args, i);
            tmp = i.get_value();
            if (TID[tmp].get_type() == LEX_INT || TID[tmp].get_type() == LV_INT)
            {
                cout << "Введите целое число для " << TID[tmp].get_name() << endl;
                cin >> k;
                TID[tmp].put_value(k);
            }
            else if (TID[tmp].get_type() == LEX_BOOL || TID[tmp].get_type() == LV_BOOL)
            {
                string str;
                while (1)
                {
                    cout << "Введите переменную логического типа (true or false) для " << TID[tmp].get_name() << endl;
                    cin >> str;
                    if (str != "true" && str != "false")
                    {
                        cout << "Ошибка ввода: нужно ввести true/false" << endl;
                        continue;
                    }
                    k = (str == "true") ? 1 : 0;
                    TID[tmp].put_value(k);
                    break;
                }
            }
            else
            {
                string str;
                cout << "Введите строку для " << TID[tmp].get_name() << endl;
                cin >> str;
                TID[tmp].put_str_val(str);
            }
            TID[tmp].put_assign();
            break;
        case LEX_PLUS:
            from_st(args, i);
            from_st(args, j);
            if (i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR)
                args.push(Lex(i.get_type(), j.get_str() + i.get_str()));
            else
                args.push(Lex(i.get_type(), j.get_value() + i.get_value()));
            break;
        case LEX_TIMES:
            from_st(args, i);
            from_st(args, j);
            args.push(Lex(i.get_type(), j.get_value() * i.get_value()));
            break;
        case LEX_MINUS:
            from_st(args, i);
            from_st(args, j);
            args.push(Lex(i.get_type(), j.get_value() - i.get_value()));
            break;
        case LEX_SLASH:
            from_st(args, i);
            from_st(args, j);
            if (i.get_value() != 0)
            {
                args.push(Lex(i.get_type(), j.get_value() / i.get_value()));
                break;
            }
            else
            {
                cout << "Ошибка. Лексема: " << pc_el;
                char s[BUFSIZE];
                sprintf(s, "%s (%d): Деление на ноль\n", __FILE__, __LINE__);
                string str = string(s);
                throw str;
            }
        case LEX_EQ:
            from_st(args, i);
            from_st(args, j);
            if (i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR)
                args.push(Lex(LEX_BOOL, j.get_str() == i.get_str()));
            else
                args.push(Lex(LEX_BOOL, j.get_value() == i.get_value()));
            break;
        case LEX_LSS:
            from_st(args, i);
            from_st(args, j);
            if (i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR)
                args.push(Lex(LEX_BOOL, j.get_str() < i.get_str()));
            else
                args.push(Lex(LEX_BOOL, j.get_value() < i.get_value()));
            break;
        case LEX_GTR:
            from_st(args, i);
            from_st(args, j);
            if (i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR)
                args.push(Lex(LEX_BOOL, j.get_str() > i.get_str()));
            else
                args.push(Lex(LEX_BOOL, j.get_value() > i.get_value()));
            break;

        case LEX_LEQ:
            from_st(args, i);
            from_st(args, j);
            args.push(Lex(LEX_BOOL, j.get_value() <= i.get_value()));
            break;
        case LEX_GEQ:
            from_st(args, i);
            from_st(args, j);
            args.push(Lex(LEX_BOOL, j.get_value() >= i.get_value()));
            break;
        case LEX_NEQ:

            from_st(args, i);
            from_st(args, j);
            if (i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR)
                args.push(Lex(LEX_BOOL, j.get_str() != i.get_str()));
            else
                args.push(Lex(LEX_BOOL, j.get_value() != i.get_value()));
            break;
        case LEX_ASSIGN:
            from_st(args, i);
            from_st(args, j);
            if (i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR)
            {
                TID[j.get_value()].put_str_val(i.get_str());
                args.push(Lex(POLIZ_ADDRESS, j.get_str(), j.get_value()));
            }
            else if (i.get_type() == POLIZ_ADDRESS)
            {
                TID[j.get_value()].put_value(TID[i.get_value()].get_value());
                args.push(Lex(POLIZ_ADDRESS, j.get_str(), j.get_value()));
            }
            else
            {
                TID[j.get_value()].put_value(i.get_value());
                args.push(Lex(POLIZ_ADDRESS, j.get_str(), j.get_value()));
            }
            TID[j.get_value()].put_assign();
            break;
        case LEX_SEMICOLON:
            args.pop();
            break;
        default:
            cout << "Ошибка. Лексема: " << pc_el;
            char s[BUFSIZE];
            sprintf(s, "%s (%d): Недопустимый элемент в ПОЛИЗе\n", __FILE__, __LINE__);
            string str = string(s);
            throw str;
        } // end of switch
        ++index;
    }; // end of while
    cout << "Finish of executing!!!" << endl;
}
void Interpretator::interpretation()
{
    pars.analyze();
    E.execute(pars.poliz);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Дайте на вход имя файла" << endl;
        exit(1);
    }

    Scanner scan(argv[1]);
    while (1)
    {
        try
        {
            Lex l1 = scan.get_lex();
            // cout << l1.get_value();
            cout << l1;
        }
        catch (string str)
        {
            if (str == "OK_LETS_GO")
            {
                Lex l2 = Lex(LEX_END, '}');
                cout << l2;
                cout << str << endl;
                break;
            }
            else
            {
                cout << str << endl;
                return 0;
            }
        }
    }
    unsigned int start_time = clock();
    try
    {
        Interpretator I(argv[1]);
        I.interpretation();
    }
    catch (string str)
    {
        cout << str;
    }
    unsigned int end_time = clock();
    std::cout << 1000.0 * (end_time - start_time) / CLOCKS_PER_SEC << std::endl;
    return 0;
}