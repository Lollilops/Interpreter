
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stack>
#include <iterator>

using namespace std;

int line = 0;
string build_result_string;
string result_string;
string poliz_string;

#include <fstream>
ifstream infile;

#define INC do { CLT = get_one_lexem(); } while(0)
#define conditional_throw(x) if (x) throw CLT //conditional throw
#define PUSH(x) poliz.push_back(x)
#define SIZE (poliz.size()-1)

enum lexema_type {
    lex_program = 0,
    lex_if, lex_else,
    lex_for, lex_while,
    lex_goto, lex_continue, lex_break,
    lex_int, lex_bool, lex_real, lex_string,
    lex_id,
    
    lex_begin, lex_end,
    lex_branch_open, lex_branch_close,
    lex_ZPT, lex_DOT,
    lex_TWODOTS, lex_t_id, lex_adress,
    //label - это метка для goto
    //lex_t_id - идентификатор, имя переменной
    lex_jmp, lex_jF, lex_t_operators, lex_empty,
    /* только для Полиза */
    lex_NULL, lex_t_var, lex_t_const,
    /* войдут в конечную программу в виде oper_type */
    lex_TZPT,
    lex_read, lex_write, lex_writeln,
    lex_plus, lex_minus,
    lex_mul, lex_div, lex_mod,
    lex_G, lex_GE, lex_L, lex_LE, lex_E, lex_NE, // > < >= <=
    lex_and, lex_or, lex_not,
    lex_mov, lex_add, lex_sub, /* = += -= */
    /* войдут в программу в виде констант */
    lex_true, lex_false, lex_int_const,
    lex_string_const, lex_real_const,
    
    lex_decl, lex_struct, lex_t_struct
};

enum oper_type {
    op_read, // mt = 2, i = 1
    op_write,
    op_plus_mt, op_plus_i,
    op_minus_mt, op_minus_i,
    op_mul_mt, op_div_mt, op_mod,
    op_G_mt, op_GE_mt, op_L_mt, op_LE_mt, op_E, op_NE,
    op_and, op_or, op_not,
    op_mov_mt,
    op_add_mt, op_sub_mt,
    op_empty,
    op_decl
};

enum value_type {
    val_int, val_lv_int,
    val_bool, val_lv_bool,
    val_real, val_lv_real,
    val_string, val_lv_string,
    val_custom, val_lv_custom,
    val_not_val // думали что это переменная, но оказалась метка для goto
};

value_type to_rv(value_type val) {
    switch(val) {
    case val_lv_int: return val_int;
    case val_lv_bool: return val_bool;
    case val_lv_real: return val_real;
    case val_lv_string: return val_string;
    case val_lv_custom: return val_custom;
    default: return val;
    }
}

value_type to_lv(value_type val) {
    switch(val) {
    case val_int: return val_lv_int;
    case val_bool: return val_lv_bool;
    case val_real: return val_lv_real;
    case val_string: return val_lv_string;
    case val_custom: return val_lv_custom;
    default: return val;
    }
}

bool is_value_lv(value_type val) {
    switch(val) {
    case val_lv_int: return true;
    case val_lv_bool: return true;
    case val_lv_real: return true;
    case val_lv_string: return true;
    case val_lv_custom: return true;
    default: return false;
    }
}

bool is_value_rv(value_type val) {
    switch(val) {
    case val_int: return true;
    case val_bool: return true;
    case val_real: return true;
    case val_string: return true;
    case val_custom: return true;
    default: return false;
    }
}

int is_oper(lexema_type lex) {
    switch(lex) {
        case lex_TZPT: case lex_read: case lex_write:
        case lex_plus: case lex_minus:
        case lex_mul: case lex_div: case lex_mod:
        case lex_G: case lex_GE: case lex_L:
        case lex_LE: case lex_E: case lex_NE:
        case lex_and: case lex_or: case lex_not:
        case lex_mov: case lex_add: case lex_sub:
        return true;
        default: return false;
    }
}

int _is_const(lexema_type lex) {
    switch(lex) {
        case lex_int_const: case lex_string_const:
        case lex_real_const:
        case lex_true: case lex_false:
        return true;
        default: return false;
    }
}

value_type from_lex_type_to_val(lexema_type lex) {
    switch(lex) {
    case lex_int: return val_int;
    case lex_bool: return val_bool;
    case lex_real: return val_real;
    case lex_string: return val_string;
    default: throw "ERROR 1";
    }
}

value_type val_result(value_type t1, value_type t2) {
    t1 = to_rv(t1);
    t2 = to_rv(t2);
    if (t1 == t2)
        return t1;
    if ((t1 == val_real && t2 == val_int) || (t1 == val_int && t2 == val_real))
        return val_real;
    throw "ERROR 2";
}

enum symbols_types {
    not_a_symbol, regular_symbol, digit, special, spaces
};

symbols_types symbols[256];

void init_symbols_array() {
    int i;
    for (i = 0; i < 256; i += 1)
        symbols[i] = not_a_symbol;
    for (i = 'a'; i <= 'z'; i += 1)
        symbols[i] = regular_symbol;
    for (i = 'A'; i <= 'Z'; i += 1)
        symbols[i] = regular_symbol;
    for (i = '0'; i <= '9'; i += 1)
        symbols[i] = digit;
    symbols['_'] = regular_symbol;
    symbols_types *a = symbols;
    a['+'] = a['-'] = a['{'] = a['}'] = a['('] = special;
    a['*'] = a['/'] = a['%'] = a['<'] = a['>'] = special;
    a['&'] = a['|'] = a[','] = a[';'] = a['!'] = special;
    a[')'] = a['='] = a['.'] = a[':'] = a['^'] = special;
    symbols[' '] = spaces;
    symbols['\t'] = spaces;
    symbols['\n'] = spaces;
}

int is_space(int i) {
    if (i >= 0 && i < 256)
        return symbols[i] == spaces;
    else
        return 0;
}

int is_regular(int i) {
    if (i >= 0 && i < 256)
        return symbols[i] == regular_symbol;
    else
        return 0;
}

int is_digit(int i) {
    if (i >= 0 && i < 256)
        return symbols[i] == digit;
    else
        return 0;
}

int is_special(int i) {
    if (i >= 0 && i < 256)
        return symbols[i] == special;
    else
        return 0;
}

int is_unknown(int i) {
    if (i >= 0 && i < 256)
        return symbols[i] == not_a_symbol;
    else
        return 0;
}

struct Lexema {
    lexema_type type;
    int i;
    Lexema(){}
    Lexema(lexema_type t, int index){ type = t; i = index; }
};

// T_id
struct Id {
    string name;
    Lexema lex;
    int cust_type;
    Id(){ cust_type = -1; }
    Id(const string &s, Lexema l) : name(s), lex(l) { cust_type = -1; }
};

// база для Var
struct Field {
    bool b;
    long i;
    double r;
    string s;
    Field() :s("") { b = false; i = 0; r = 0; }
    void operator += (const Field &f) { i += f.i; r += f.r; }
    void operator -= (const Field &f) { i -= f.i; r -= f.r; }
};
// из Var состоит T_var, T_const
struct Var {
    value_type type;
    bool _is_const, is_rv, is_valued;
    Field v;
    Var(){
        _is_const = false;
        is_rv = true;
        type = val_not_val;
        is_valued = false;
    }
    Var(value_type t, bool c, bool rv ) {
        _is_const = c;
        is_rv = rv;
        type = t;
        if ( _is_const || is_rv )
            is_valued = true;
        else
            is_valued = false;
    }
};

struct Execution_error {
    string s;
    Execution_error(const char *c) : s(c){};
};

struct Operation {
    oper_type type;
    Operation(){}
    virtual ~Operation(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c){};
    inline Var get_var(Lexema lex, vector<Var> &v, vector<Var> &c) {
        return lex.type == lex_t_var ? v[lex.i] : c[lex.i];
    }
    inline void set_var(Lexema lex, vector<Var> &v, Var &vr) {
        v[lex.i] = vr;
    }
    inline void check_mem(Var &v) {
        if ( ! v.is_valued )
            throw Execution_error("read from not-valued memory");
    }
    inline void may_be_del(Var &v, vector<Var> &ve) {
        if (v.is_rv && !v._is_const)
            ve.pop_back();
    }
};

struct Op_read : public Operation {
    Op_read(){ type = op_read; } virtual ~Op_read(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c) {
        Var vr = get_var(st.back(),v,c);
        vr.is_valued = true;
        switch (vr.type) {
        case val_int: infile >> vr.v.i; break;
        case val_real: infile >> vr.v.r; break;
        case val_bool: infile >> vr.v.b; break;
        case val_string: infile >> vr.v.s; break;
        default: throw "ERROR 3";
        }
        set_var(st.back(),v,vr);
    }
};

struct Op_write : public Operation {
    Op_write(){ type = op_write; } virtual ~Op_write(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c) {
        Var vr = get_var(st.back(),v,c);
        check_mem(vr);
        switch (vr.type) {
        case val_int: result_string += string(to_string(vr.v.i)); break;;
        case val_real: result_string += string(to_string(vr.v.r)); break;
        case val_bool: result_string += string(vr.v.b ? "true" : "false"); break;
        case val_string: result_string += string(vr.v.s); break;
        default: throw "ERROR 4";
        }
        may_be_del(vr,v);
    }
};

struct Op_decl : public Operation {
    Op_decl(){ type = op_decl; } virtual ~Op_decl(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c) {
        Lexema lex;
        Var vr1 = get_var( st.back(), v, c );
        check_mem(vr1);
        st.pop_back();
        Var vr2 = vr1;
        lex = st.back();
        st.pop_back();
        vr2._is_const = false;
        vr2.is_rv = false;
        set_var(lex,v,vr2);
    }
};

struct Op_MOV_mt : public Operation {
    Op_MOV_mt(oper_type t){ type = t; } virtual ~Op_MOV_mt(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c) {
        Var vr2 = get_var(st.back(),v,c);
        st.pop_back();
        check_mem(vr2);
        Var vr1 = get_var(st.back(),v,c);
        if ( !vr1.is_valued && this->type != op_mov_mt )
            throw Execution_error("read from not valued mem");
        vr1.is_valued = true;
        act(vr1,vr2);
        set_var(st.back(),v,vr1);
        may_be_del(vr2,v);
    }
    void act(Var &vr1, Var &vr2) {
        if (type == op_mov_mt ) {
            if (vr1.type == vr2.type)
                vr1.v = vr2.v;
            else
                if ( vr1.type == val_int )
                    vr1.v.i = vr2.v.r;
                else
                    vr1.v.r = vr2.v.i;
            return;
        }
        if (type == op_add_mt ) {
            if (vr1.type == vr2.type)
                if (vr1.type != val_string)
                    vr1.v += vr2.v;
                else
                    vr1.v.s += vr2.v.s;
            else
                if ( vr1.type == val_int )
                    vr1.v.i += vr2.v.r;
                else
                    vr1.v.r += vr2.v.i;
            return;
        }
        if (type == op_sub_mt ) {
            if (vr1.type == vr2.type)
                vr1.v -= vr2.v;
            else
                if ( vr1.type == val_int )
                    vr1.v.i -= vr2.v.r;
                else
                    vr1.v.r -= vr2.v.i;
            return;
        }
        throw "ERROR 5";
    }
    
};

struct Op_ARITHM_mt : public Operation {
    Op_ARITHM_mt(oper_type t){ type = t; } virtual ~Op_ARITHM_mt(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c) {
        Var vr2 = get_var(st.back(),v,c);
        st.pop_back();
        check_mem(vr2);
        Var vr1 = get_var(st.back(),v,c);
        st.pop_back();
        check_mem(vr1);
        may_be_del(vr2,v);
        may_be_del(vr1,v);
        vr1._is_const = false; // теперь это временный объект
        vr1.is_rv = true;
        act(vr1,vr2);
        v.push_back(vr1);
        st.push_back(Lexema(lex_t_var,v.size()-1));
    }
    void act(Var &vr1, Var &vr2) {
        if (type == op_plus_mt ) {
            if (vr1.type == vr2.type)
                if (vr1.type != val_string)
                    vr1.v += vr2.v;
                else
                    vr1.v.s += vr2.v.s;
            else {
                if ( vr1.type == val_int )
                    vr1.v.r = vr1.v.i + vr2.v.r;
                else
                    vr1.v.r += vr2.v.i;
                vr1.type = val_real;
            }
            return;
        }
        if (type == op_minus_mt ) {
            if (vr1.type == vr2.type)
                vr1.v -= vr2.v;
            else {
                if ( vr1.type == val_int )
                    vr1.v.r = vr1.v.i - vr2.v.r;
                else
                    vr1.v.r -= vr2.v.i;
                vr1.type = val_real;
            }
            return;
        }
        if (type == op_mul_mt ) {
            if (vr1.type == vr2.type) {
                vr1.v.i *= vr2.v.i;
                vr1.v.r *= vr2.v.r;
            } else {
                if ( vr1.type == val_int )
                    vr1.v.r = vr1.v.i * vr2.v.r;
                else
                    vr1.v.r *= vr2.v.i;
                vr1.type = val_real;
            }
            return;
        }
        if (type == op_div_mt ) {
            if (vr1.type == vr2.type)
                if ( vr1.type == val_int )
                    vr1.v.i /= vr2.v.i;
                else
                    vr1.v.r /= vr2.v.r;
            else {
                if ( vr1.type == val_int )
                    vr1.v.r = vr1.v.i / vr2.v.r;
                else
                    vr1.v.r /= vr2.v.i;
                vr1.type = val_real;
            }
            return;
        }
        if (type == op_mod ) {
            vr1.v.i %= vr2.v.i;
            return;
        }
        throw "ERROR 6";
    }
};

struct Op_CMP_mt : public Operation {
    Op_CMP_mt(oper_type t) { type = t; } virtual ~Op_CMP_mt(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c) {
        Var vr2 = get_var(st.back(),v,c);
        st.pop_back();
        check_mem(vr2);
        Var vr1 = get_var(st.back(),v,c);
        st.pop_back();
        check_mem(vr1);
        may_be_del(vr2,v);
        may_be_del(vr1,v);
        vr1._is_const = false;
        vr1.is_rv = true;
        vr1.v.b = cmp(vr1,vr2);
        vr1.type = val_bool;
        v.push_back(vr1);
        st.push_back(Lexema(lex_t_var,v.size()-1));
    }
    bool cmp(Var &vr1, Var &vr2) {
        if ( type == op_L_mt ) {
            if (vr1.type == vr2.type) {
                if ( vr1.type == val_int )
                    return vr1.v.i < vr2.v.i;
                else if ( vr1.type == val_real )
                    return vr1.v.r < vr2.v.r;
                else
                    return vr1.v.s < vr2.v.s;
            } else {
                if ( vr1.type == val_int )
                    return vr1.v.i < vr2.v.r;
                else
                    return vr1.v.r < vr2.v.i;
            }
        }
        if ( type == op_LE_mt ) {
            if (vr1.type == vr2.type) {
                if ( vr1.type == val_int )
                    return vr1.v.i <= vr2.v.i;
                else if ( vr1.type == val_real )
                    return vr1.v.r <= vr2.v.r;
                else
                    return vr1.v.s <= vr2.v.s;
            } else {
                if ( vr1.type == val_int )
                    return vr1.v.i <= vr2.v.r;
                else
                    return vr1.v.r <= vr2.v.i;
            }
        }
        if ( type == op_G_mt ) {
            if (vr1.type == vr2.type) {
                if ( vr1.type == val_int )
                    return vr1.v.i > vr2.v.i;
                else if ( vr1.type == val_real )
                    return vr1.v.r > vr2.v.r;
                else
                    return vr1.v.s > vr2.v.s;
            } else {
                if ( vr1.type == val_int )
                    return vr1.v.i > vr2.v.r;
                else
                    return vr1.v.r > vr2.v.i;
            }
        }
        if ( type == op_GE_mt ) {
            if (vr1.type == vr2.type) {
                if ( vr1.type == val_int )
                    return vr1.v.i >= vr2.v.i;
                else if ( vr1.type == val_real )
                    return vr1.v.r >= vr2.v.r;
                else
                    return vr1.v.s >= vr2.v.s;
            } else {
                if ( vr1.type == val_int )
                    return vr1.v.i >= vr2.v.r;
                else
                    return vr1.v.r >= vr2.v.i;
            }
        }
        if ( type == op_E ) {
            if ( vr1.type == val_int )
                return vr1.v.i == vr2.v.i;
            else if ( vr1.type == val_real )
                return vr1.v.r == vr2.v.r;
            else if ( vr1.type == val_string )
                return vr1.v.s == vr2.v.s;
            else
                return vr1.v.b == vr2.v.b;
        }
        if ( type == op_NE ) {
            if ( vr1.type == val_int )
                return vr1.v.i != vr2.v.i;
            else if ( vr1.type == val_real )
                return vr1.v.r != vr2.v.r;
            else if ( vr1.type == val_string )
                return vr1.v.s != vr2.v.s;
            else
                return vr1.v.b != vr2.v.b;
        }
        throw "ERROR 7";
    }
};

struct Op_UNAR : public Operation {
    Op_UNAR(oper_type t) { type = t; } virtual ~Op_UNAR(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c) {
        Var vr = get_var(st.back(),v,c);
        st.pop_back();
        check_mem(vr);
        may_be_del(vr,v);
        vr._is_const = false;
        vr.is_rv = true;
        if (type == op_minus_i) {
            vr.v.i = -vr.v.i;
            vr.v.r = -vr.v.r;
        } else if ( type == op_not ) {
            vr.v.b = !vr.v.b;
        }
        v.push_back(vr);
        st.push_back(Lexema(lex_t_var,v.size()-1));
    }
};

struct Op_LOG : public Operation {
    Op_LOG(oper_type t) { type = t; } virtual ~Op_LOG(){}
    virtual void apply(vector<Lexema> &st, vector<Var> &v, vector<Var> &c) {
        Var vr2 = get_var(st.back(),v,c);
        st.pop_back();
        check_mem(vr2);
        Var vr1 = get_var(st.back(),v,c);
        st.pop_back();
        check_mem(vr1);
        may_be_del(vr2,v);
        may_be_del(vr1,v);
        vr1._is_const = false;
        vr1.is_rv = true;
        if (type == op_and) {
            vr1.v.b = vr1.v.b && vr2.v.b;
        } else {
            vr1.v.b = vr1.v.b || vr2.v.b;
        }
        vr1.type = val_bool;
        v.push_back(vr1);
        st.push_back(Lexema(lex_t_var,v.size()-1));
    }
};

struct param_struct {
    string name;
    value_type type;
};

struct Structura {
    string name;
    int size;
    int cust_type;
    vector<param_struct> param;
};

class Parser {
    lexema_type CLT; // текущая считанная лексема
private:
    char *filename;
    FILE *file;
    int current_line;

public:
    Parser(const char *s) {
        filename = strdup(s);
        cur_chr = ' ';
        file = fopen(filename,"r");
        if (file == NULL)
            throw "unknown filename";
        current_line = 1;
    }
    ~Parser() {
        free(filename);
        fclose(file);
        while (!T_oper.empty()) {
            delete T_oper.back();
            T_oper.pop_back();
        }
    }
    void analyze();

private:
    char cur_chr;
    string bufer;
    lexema_type get_one_lexem();
    lexema_type read_number();
    lexema_type read_word();
    lexema_type read_operator();
    const char* lex_name(lexema_type t);

    vector <Var> T_var;
    vector <Var> T_const;
    vector <Id> T_id;
    vector <Id> T_label;
    vector <Lexema> poliz;
    vector <Operation*> T_oper;

    void declar_label();
    Lexema declar_local_var(value_type);
    Lexema init_var(value_type &);
    Lexema init_const(lexema_type lex, value_type &t);
    Lexema init_const_string();
    Lexema init_const_real();
    Lexema init_const_bool();
    Lexema init_const_int();
    Lexema init_oper (lexema_type lex, value_type op1 = val_not_val,
                    value_type op2 = val_not_val);

    void analysis();
    void DECLAR_LOCAL();
    void OPS();
    void OP();
    void READ_WRITE();
    void MOVE(value_type);
    value_type IDENTIF();
    void IF();
    void FOR();
    void WHILE();
    void CONTINUE_BREAK();
    void GOTO();

    value_type VALUE();
    value_type level0();
    value_type level1();
    value_type level2();
    value_type level3();
    value_type level4();
    value_type level5();
    value_type level6();
    
    stack<Lexema> continue_break_counter;
    vector <Id> goto_counter;
    void compare_goto_and_label();
    void execute();
    
    void DESCRIBE_STRUCT();
    void DECLAR_GLOBAL_STRUCT();
    int struct_type;
    Lexema struct_lex;
    vector<Structura> T_struct;
};





value_type Parser::level6() {
    value_type t1;
    if ( CLT == lex_id ) {
        t1 = IDENTIF();
        if (t1 == val_not_val)
            throw "unexpected label";
        return t1;
    }
    if ( _is_const(CLT) ) {
        PUSH(init_const(CLT,t1));
        INC;
        return t1;
    }
    if ( CLT == lex_branch_open ) {
        INC;
        t1 = VALUE();
        conditional_throw(CLT != lex_branch_close);
        INC;
        return t1;
    }
    throw CLT;
}

value_type Parser::level5() {
    value_type t1;
    if (CLT == lex_plus || CLT == lex_minus || CLT == lex_not) {
        lexema_type lx = CLT;
        INC;
        t1 = level6();
        PUSH(init_oper(lx,t1));
        return to_rv(t1);
    }
    return level6();
}

value_type Parser::level4() {
    value_type t1 = level5(),t2;
    if ( CLT != lex_mul && CLT != lex_div && CLT != lex_mod )
        return t1;
    lexema_type lx;
    while ( CLT == lex_mul || CLT == lex_div || CLT == lex_mod ) {
        lx = CLT;
        INC;
        t2 = level5();
        PUSH(init_oper(lx,t1,t2));
        t1 = val_result(t1,t2);
    }
    return to_rv(t1);
}

value_type Parser::level3() {
    value_type t1 = level4(),t2;
    if ( CLT != lex_plus && CLT != lex_minus )
        return t1;
    lexema_type lx;
    while ( CLT == lex_plus || CLT == lex_minus ) {
        lx = CLT;
        INC;
        t2 = level4();
        PUSH(init_oper(lx,t1,t2));
        t1 = val_result(t1,t2);
    }
    return to_rv(t1);
}

value_type Parser::level2() {
    value_type t1 = level3(),t2;
    if ( CLT == lex_G || CLT == lex_GE || CLT == lex_L || CLT == lex_LE || CLT == lex_E || CLT == lex_NE ) {
        lexema_type lx = CLT;
        INC;
        t2 = level3();
        PUSH(init_oper(lx,t1,t2));
        return val_bool;
    }
    return t1;
}

value_type Parser::level1() {
    value_type t1 = level2(),t2;
    if ( CLT != lex_and )
        return t1;
    while ( CLT == lex_and ) {
        INC;
        t2 = level2();
        PUSH(init_oper(lex_and,t1,t2));
    }
    return val_bool;
}

value_type Parser::level0() {
    value_type t1 = level1(),t2;
    if ( CLT != lex_or )
        return t1;
    lexema_type lx;
    while ( (lx = CLT) == lex_or ) {
        INC;
        t2 = level1();
        PUSH(init_oper(lx,t1,t2));
    }
    return val_bool;
}

value_type Parser::VALUE() {
    value_type t2, t1 = level0();
    while ( CLT == lex_mov || CLT == lex_add || CLT == lex_sub ) {
        if ( is_value_rv(t1) ) throw "assignment to RValue";
        lexema_type lx = CLT;
        INC;
        t2 = VALUE();
        PUSH(init_oper(lx,t1,t2));
        t1 = t2;
    }
    return t1;
}


void Parser::CONTINUE_BREAK() {
    PUSH( Lexema(lex_adress,0) );
    continue_break_counter.push(Lexema(CLT,SIZE));
    PUSH( Lexema(lex_jmp,0) );
    INC;
    conditional_throw(CLT != lex_TZPT);
    PUSH(init_oper(lex_TZPT));
    INC;
}

void Parser::GOTO() {
    INC;
    conditional_throw( CLT != lex_id );
    PUSH( Lexema(lex_adress,0) );
    Lexema lex(lex_adress,SIZE);
    PUSH( Lexema(lex_jmp,0) );
    goto_counter.push_back(Id(bufer,lex));
    compare_goto_and_label();
    INC;
    conditional_throw(CLT != lex_TZPT);
    INC;
}

void Parser::FOR() {
    INC;
    conditional_throw( CLT != lex_branch_open );
    INC;
    if ( CLT != lex_TZPT )
        VALUE();
    conditional_throw( CLT != lex_TZPT );
    PUSH(init_oper(lex_TZPT));
    INC;
    /* labels and jmps */
    int label_1 = SIZE;
    value_type t = to_rv( VALUE() );
    if (t != val_bool)
        throw " unexpected type in FOR condition";
    int save_1;
    int save_2;
    PUSH( Lexema(lex_adress,0) );
    save_1 = SIZE;
    PUSH( Lexema(lex_jF,0) );
    PUSH( Lexema(lex_adress,0) );
    save_2 = SIZE;
    PUSH( Lexema(lex_jmp,0) );
    int label_2 = SIZE;
    conditional_throw(CLT != lex_TZPT);
    INC;
    if ( CLT != lex_branch_close )
        VALUE();
    conditional_throw( CLT != lex_branch_close );
    PUSH(init_oper(lex_TZPT));
    INC;
    PUSH( Lexema(lex_adress,label_1) );
    PUSH( Lexema(lex_jmp,0) );
    poliz[save_2].i = SIZE;
    continue_break_counter.push(Lexema(lex_begin,-100));
    OP();
    PUSH( Lexema(lex_adress,label_2) );
    PUSH( Lexema(lex_jmp,0) );
    poliz[save_1].i = SIZE;
    while ( continue_break_counter.top().i != -100 ) {
        Lexema lex = continue_break_counter.top();
        continue_break_counter.pop();
        if (lex.type == lex_continue )
            poliz[lex.i].i = label_1;
        else
            poliz[lex.i].i = SIZE;
    }
    continue_break_counter.pop();
}

void Parser::WHILE() {
    INC;
    conditional_throw( CLT != lex_branch_open );
    INC;
    int label_1 = SIZE;
    value_type t = to_rv( VALUE() );
    if (t != val_bool)
        throw " unexpected type in WHILE condition";
    conditional_throw( CLT != lex_branch_close );
    INC;
    int kk = SIZE+1;
    PUSH(Lexema(lex_adress,0));
    PUSH(Lexema(lex_jF,0));
    continue_break_counter.push(Lexema(lex_begin,-100));
    OP();
    PUSH(Lexema(lex_adress,label_1));
    PUSH(Lexema(lex_jmp,0));
    poliz[kk].i = SIZE;
    while ( continue_break_counter.top().i != -100 ) {
        Lexema lex = continue_break_counter.top();
        continue_break_counter.pop();
        if (lex.type == lex_continue )
            poliz[lex.i].i = label_1;
        else
            poliz[lex.i].i = SIZE;
    }
    continue_break_counter.pop();
}

void Parser::MOVE(value_type t1) {
    value_type t2;
    lexema_type lx;
    conditional_throw(CLT != lex_mov && CLT != lex_add && CLT != lex_sub );
    lx = CLT;
    INC;
    
    int op_prew = struct_type;
    Lexema lex_prew = struct_lex;
    
    t2 = VALUE();
    if (t1 == val_lv_custom) {
        if ( t2 != val_lv_custom && t2 != val_custom )
            throw "you can't use Structuras with base types";
        if (op_prew != struct_type)
            throw "you can't assign one type to other";
        Structura st = T_struct[struct_type];
        for (int i = 0; i < st.size; i += 1) {
            PUSH(Lexema(lex_t_var,lex_prew.i + i));
            PUSH(Lexema(lex_t_var,struct_lex.i + i));
            PUSH(init_oper(lx,st.param[i].type,st.param[i].type));
        }
    } else {
        PUSH(init_oper(lx,t1,t2));
    }
    conditional_throw(CLT != lex_TZPT);
    PUSH(init_oper(lex_TZPT));
    INC;
    return;
}



void Parser::DESCRIBE_STRUCT() {
    int cust_type = 0;
    while (CLT == lex_struct) {
        INC;
        conditional_throw(CLT != lex_id);
        Structura st;
        st.name = bufer;
        for (int i = T_struct.size()-1; i >= 0; i -= 1)
            if (bufer == T_struct[i].name)
                throw string("struct with name '") + bufer + "' declarated 2 times";
        INC;
        conditional_throw(CLT != lex_begin);
        INC;
        param_struct pr;
        int cnt = 0, i;
        do {
            // поля структуры
            conditional_throw( CLT != lex_int && CLT != lex_bool && CLT != lex_string && CLT != lex_real );
            pr.type = from_lex_type_to_val(CLT);
            INC;
            conditional_throw( CLT != lex_id );
            pr.name = bufer;
            INC;
            conditional_throw( CLT != lex_TZPT );
            INC;
            for (i = 0; i < cnt; i += 1)
                if (st.param[i].name == pr.name)
                    throw string("name '") + pr.name + "' declarated 2 times in one struct";
            st.param.push_back(pr);
            cnt++;
        } while ( CLT != lex_end );
        INC;
        conditional_throw( CLT != lex_TZPT );
        INC;
        st.size = cnt;
        st.cust_type = cust_type;
        cust_type += 1;
        T_struct.push_back(st);
    }
}

void Parser::DECLAR_GLOBAL_STRUCT() {
    INC;
    int numb;
    for (numb = T_struct.size()-1; numb >= 0; numb -= 1)
        if ( T_struct[numb].name == bufer )
            goto good_case;
    throw string("Structura name '") + bufer + "' don't exist";
good_case:
    Structura st = T_struct[numb];
    do {
        INC;
        conditional_throw( CLT != lex_id );
        /* копипаст declar_global_var */
        int i = T_id.size() - 1;
        while ( i >= 0 ) {
            if (T_id[i].name == bufer)
                throw std::string("variable " + T_id[i].name + " declarated twice");
            i -= 1;
        }
        Lexema lex(lex_t_struct,T_var.size());
        T_id.push_back(Id(bufer,lex));
        T_id.back().cust_type = st.cust_type;
        for (i = 0; i < (int)st.size; i += 1)
            T_var.push_back(Var(st.param[i].type,false,false));
        /* */
        INC;
        if ( CLT == lex_ZPT )
            continue;
    } while ( CLT != lex_TZPT );
    INC;
}

value_type Parser::IDENTIF() {
    value_type t;
    string __s = bufer;
    lexema_type __lex = CLT;
    INC;
    if (CLT == lex_TWODOTS) {
        bufer = __s;
        CLT = __lex;
        PUSH( Lexema(lex_empty,0) );
        declar_label();
        compare_goto_and_label();
        INC;
        return val_not_val;
    } else if (CLT == lex_DOT) {
        bufer = __s;
        CLT = __lex;
        init_var(t);
        if (t != val_custom && t != val_lv_custom)
            throw "you can use '.' with Structuras, not with based types";
        INC;
        Structura st = T_struct[struct_type];
        // struct_lex.i; это ссылка на T_var на первую переменную внутри объекта
        for (int i = 0; i < st.size; i += 1) {
            if ( bufer == st.param[i].name ) {
                PUSH(Lexema(lex_t_var,struct_lex.i + i));
                INC;
                return to_lv(st.param[i].type);
            }
        }
        throw string("Structura '") + st.name + "' hasn't field '" + bufer + "'.";
    } else {
        // real variable?
        swap(__s,bufer);
        Lexema lex = init_var(t);
        if (t == val_custom || t == val_lv_custom) {
        } else {
            PUSH(lex);
        }
        swap(__s,bufer);
        return to_lv(t);
    }
    throw CLT;
}

void Parser::READ_WRITE() {
    lexema_type rw = CLT;
    INC;
    conditional_throw(CLT != lex_branch_open);
    INC;
    value_type t;
    if ( rw == lex_read ) {
        
        while (1) {
            t = IDENTIF();
            if (!is_value_lv(t))
                throw "don't use READ() with rvalue";
            if (t == val_lv_custom)
                throw "can't read Structura";
            PUSH(init_oper(lex_read,t));
            if (CLT == lex_ZPT) {
                INC;
                continue;
            }
            if (CLT == lex_branch_close)
                break;
            throw CLT;
        }
    } else {
        if (rw == lex_writeln && CLT == lex_branch_close)
            goto writeln_case;
        while (1) {
            t = VALUE();
            PUSH(init_oper(lex_write,t));
            if (CLT == lex_ZPT) {
                INC;
                continue;
            }
            if (CLT == lex_branch_close)
                break;
            throw CLT;
        }
writeln_case:
        if (rw == lex_writeln) {
            bufer = "\n";
            PUSH( init_const_string() );
            PUSH( init_oper(lex_write,val_string) );
        }
    }
    INC;
    conditional_throw(CLT != lex_TZPT);
    PUSH(init_oper(lex_TZPT));
    INC;
}

void Parser::IF() {
    INC;
    conditional_throw( CLT != lex_branch_open );
    INC;
    value_type t = to_rv( VALUE() );
    if (t != val_bool)
        throw " unexpected type in IF condition";
    conditional_throw( CLT != lex_branch_close );
    INC;
    int k = SIZE + 1;
    PUSH( Lexema(lex_adress,0) );
    PUSH( Lexema(lex_jF,0) );
    OP();
    if ( CLT == lex_else ) {
        INC;
        int kk = SIZE+1;
        PUSH(Lexema(lex_adress,0));
        PUSH(Lexema(lex_jmp,0));
        poliz[k].i = SIZE;
        OP();
        poliz[kk].i = SIZE;
    } else {
        poliz[k].i = SIZE; // SIZE + 1 ????
    }
}

void Parser::OP() {
    if ( CLT == lex_id ) {
        value_type v = IDENTIF();
        if (v == val_not_val)
            return;
        MOVE(v);
        return;
    }
    if ( CLT == lex_read || CLT == lex_write || CLT == lex_writeln ) {
        READ_WRITE();
        return;
    }
    if ( CLT == lex_if ) {
        IF();
        return;
    }
    if ( CLT == lex_begin ) {
        INC;
        OPS();
        conditional_throw(CLT != lex_end);
        INC;
        return;
    }
    if ( CLT == lex_while ) {
        WHILE();
        return;
    }
    if ( CLT == lex_for ) {
        FOR();
        return;
    }
    if ( CLT == lex_continue || CLT == lex_break ) {
        CONTINUE_BREAK();
        return;
    }
    if ( CLT == lex_goto ) {
        GOTO();
        return;
    }
    throw CLT;
}

void Parser::OPS() {
    while ( CLT != lex_end )
        OP();
}

void Parser::DECLAR_LOCAL() {
    while ( CLT == lex_int || CLT == lex_bool || CLT == lex_string || CLT == lex_real || CLT == lex_struct ) {
        if (CLT == lex_struct) {
            DECLAR_GLOBAL_STRUCT();
            continue;
        }
        value_type t1 = from_lex_type_to_val(CLT), t2; //lexema_type->value_type
        do {
            INC;
            if ( CLT != lex_id ) throw CLT;
            Lexema lex1 = declar_local_var(t1);
            INC;
            if ( CLT == lex_mov ) {
                INC;
                if ( !_is_const(CLT) ) throw CLT; // нельзя int a = b;
                Lexema lex2 = init_const(CLT,t2);
                if ( t1 != t2 ) throw "different types of var and const"; // int a = "abc";
                T_var[lex1.i].is_valued = true;
                T_var[lex1.i].v = T_const[lex2.i].v;
                INC;
            }
            if ( CLT == lex_ZPT )
                continue;
        } while ( CLT != lex_TZPT );
        INC;
    }
}

void Parser::analysis() {
    INC;
    DESCRIBE_STRUCT();
    if ( CLT != lex_program ) throw CLT;
    INC;
    if ( CLT != lex_begin ) throw CLT;
    INC;
    DECLAR_LOCAL();
    OPS();
    if ( CLT != lex_end ) throw CLT;
    INC;
    if ( CLT != lex_NULL ) throw CLT;
    PUSH(Lexema(lex_NULL,0));
    if ( !continue_break_counter.empty() )
        throw "BREAK or CONTINUE not in cycle";
    if ( !goto_counter.empty() )
        throw "GOTO with no label";
}




void Parser::declar_label() {
    int i = T_label.size() - 1;
    while ( i >= 0 ) {
        if (T_label[i].name == bufer)
            throw string("two labels '") + bufer + "' in one context";
        i -= 1;
    }
    Lexema lex(lex_adress, SIZE);
    T_label.push_back(Id(bufer,lex));
}

// унарные операторы: op2 == val_not_val
// throw ЕСЛИ недопустимые типы для данного оператора
// если норм, то
Lexema Parser::init_oper (lexema_type lex, value_type op1, value_type op2) {
    op1 = to_rv(op1);
    op2 = to_rv(op2);
    oper_type ot;
    if (lex == lex_TZPT)
        return Lexema(lex_TZPT,0);
    if ( op1 == val_int && op2 == val_not_val ) {
        switch (lex) {
            case lex_plus: ot = op_plus_i; break;
            case lex_minus: ot = op_minus_i; break;
            case lex_read: ot = op_read; break;
            case lex_write: ot = op_write; break;
            default: throw lex;
        }
        goto good_case;
    }
    if ( op1 == val_real && op2 == val_not_val ) {
        switch (lex) {
            case lex_plus: ot = op_plus_i; break;
            case lex_minus: ot = op_minus_i; break;
            case lex_read: ot = op_read; break;
            case lex_write: ot = op_write; break;
            default: throw lex;
        }
        goto good_case;
    }
    if ( op1 == val_bool && op2 == val_not_val ) {
        switch (lex) {
            case lex_not: ot = op_not; break;
            case lex_read: ot = op_read; break;
            case lex_write: ot = op_write; break;
            default: throw lex;
        }
        goto good_case;
    }
    if ( op1 == val_string && op2 == val_not_val ) {
        switch (lex) {
            case lex_read: ot = op_read; break;
            case lex_write: ot = op_write; break;
            default: throw lex;
        }
        goto good_case;
    }
    if ( op1 == val_int && op2 == val_int ) {
        switch (lex) {
            case lex_plus: ot = op_plus_mt; break;
            case lex_minus: ot = op_minus_mt; break;
            case lex_mul: ot = op_mul_mt; break;
            case lex_div: ot = op_div_mt; break;
            case lex_mod: ot = op_mod; break;
            case lex_G: ot = op_G_mt; break;
            case lex_GE: ot = op_GE_mt; break;
            case lex_L: ot = op_L_mt; break;
            case lex_LE: ot = op_LE_mt; break;
            case lex_E: ot = op_E; break;
            case lex_NE: ot = op_NE; break;
            case lex_mov: ot = op_mov_mt; break;
            case lex_add: ot = op_add_mt; break;
            case lex_sub: ot = op_sub_mt; break;
            case lex_decl: ot = op_decl; break;
            default: throw lex;
        }
        goto good_case;
    }
    if ( op1 == val_real && op2 == val_real ) {
        switch (lex) {
            case lex_plus: ot = op_plus_mt; break;
            case lex_minus: ot = op_minus_mt; break;
            case lex_mul: ot = op_mul_mt; break;
            case lex_div: ot = op_div_mt; break;
            case lex_G: ot = op_G_mt; break;
            case lex_GE: ot = op_GE_mt; break;
            case lex_L: ot = op_L_mt; break;
            case lex_LE: ot = op_LE_mt; break;
            case lex_E: ot = op_E; break;
            case lex_NE: ot = op_NE; break;
            case lex_mov: ot = op_mov_mt; break;
            case lex_add: ot = op_add_mt; break;
            case lex_sub: ot = op_sub_mt; break;
            case lex_decl: ot = op_decl; break;
            default: throw lex;
        }
        goto good_case;
    }
    if ( (op1 == val_int && op2 == val_real) || (op1 == val_real && op2 == val_int) ) {
        switch (lex) {
            case lex_plus: ot = op_plus_mt; break;
            case lex_minus: ot = op_minus_mt; break;
            case lex_mul: ot = op_mul_mt; break;
            case lex_div: ot = op_div_mt; break;
            case lex_G: ot = op_G_mt; break;
            case lex_GE: ot = op_GE_mt; break;
            case lex_L: ot = op_L_mt; break;
            case lex_LE: ot = op_LE_mt; break;
            case lex_mov: ot = op_mov_mt; break;
            case lex_add: ot = op_add_mt; break;
            case lex_sub: ot = op_sub_mt; break;
            default: throw lex;
        }
        goto good_case;
    }
    if ( op1 == val_string && op2 == val_string) {
        switch (lex) {
            case lex_plus: ot = op_plus_mt; break;
            case lex_G: ot = op_G_mt; break;
            case lex_GE: ot = op_GE_mt; break;
            case lex_L: ot = op_L_mt; break;
            case lex_LE: ot = op_LE_mt; break;
            case lex_E: ot = op_E; break;
            case lex_NE: ot = op_NE; break;
            case lex_mov: ot = op_mov_mt; break;
            case lex_add: ot = op_add_mt; break;
            case lex_decl: ot = op_decl; break;
            default: throw lex;
        }
        goto good_case;
    }
    if ( op1 == val_bool && op2 == val_bool) {
        switch (lex) {
            case lex_E: ot = op_E; break;
            case lex_NE: ot = op_NE; break;
            case lex_mov: ot = op_mov_mt; break;
            case lex_or: ot = op_or; break;
            case lex_and: ot = op_and; break;
            case lex_decl: ot = op_decl; break;
            default: throw lex;
        }
        goto good_case;
    }
    if (op1 == val_custom || op2 == val_custom)
        throw "unknown operation with Structura";
    throw lex;
good_case:
    int i = T_oper.size()-1;
    while (i >= 0) {
        if ( T_oper[i]->type == ot )
            return Lexema(lex_t_operators,i);
        i--;
    }
    switch(ot) {
        case op_read: T_oper.push_back(new Op_read); break;
        case op_write: T_oper.push_back(new Op_write); break;
        case op_plus_mt:
        case op_minus_mt:
        case op_mul_mt:
        case op_div_mt:
        case op_mod: T_oper.push_back(new Op_ARITHM_mt(ot)); break;
        case op_not:
        case op_plus_i:
        case op_minus_i: T_oper.push_back(new Op_UNAR(ot)); break;
        case op_G_mt:
        case op_GE_mt:
        case op_L_mt:
        case op_LE_mt:
        case op_E:
        case op_NE: T_oper.push_back(new Op_CMP_mt(ot)); break;
        case op_and:
        case op_or: T_oper.push_back(new Op_LOG(ot)); break;
        case op_mov_mt:
        case op_add_mt:
        case op_sub_mt: T_oper.push_back(new Op_MOV_mt(ot)); break;
        case op_decl: T_oper.push_back(new Op_decl); break;
        default: throw "ERROR 8";
    }
    return Lexema(lex_t_operators,T_oper.size()-1);
}

void Parser::compare_goto_and_label() {
    int i = T_label.size() - 1;
    int j;
    while ( i >= 0 ) {
        j = goto_counter.size() - 1;
        while (j >= 0) {
                if (T_label[i].name == goto_counter[j].name) {
                    int from = goto_counter[j].lex.i;
                    int to = T_label[i].lex.i;
                    poliz[from].i = to; /* указали метку куда прыгать */
                    goto_counter.erase(goto_counter.begin()+j);
                }
            j -= 1;
        }
        i -= 1;
    }
}

Lexema Parser::declar_local_var(value_type t) {
    int i = T_id.size() - 1;
    while ( i >= 0 ) {
        if (T_id[i].name == bufer)
            // повторное объявление
            throw std::string("variable '" + T_id[i].name + "' declarated twice");
        i -= 1;
    }
    Lexema lex(lex_t_var, T_var.size());
    T_var.push_back(Var(t,false,false));
    T_id.push_back(Id(bufer,lex));
    return lex;
}

// СОПОСТАВИТЬ идентификатор и его же в T_var
Lexema Parser::init_var(value_type &t) {
    int i = T_id.size() - 1;
    while ( i >= 0 ) {
        if (T_id[i].name == bufer) {
            Lexema lex = T_id[i].lex;
            if ( T_id[i].cust_type != -1) {
                struct_type = T_id[i].cust_type;
                struct_lex = lex;
                t = val_lv_custom;
            } else {
                t = T_var[lex.i].type;
            }
            return lex;
        }
        i -= 1;
    }
    throw string("undeclarated variable: " + bufer);
}

Lexema Parser::init_const(lexema_type lex, value_type &t) {
    switch(lex) {
    case lex_true: case lex_false:
        t = val_bool; return init_const_bool();
    case lex_int_const:
        t = val_int; return init_const_int();
    case lex_real_const:
        t = val_real; return init_const_real();
    case lex_string_const:
        t = val_string; return init_const_string();
    default: throw "ERROR 9";
    }
}

Lexema Parser::init_const_string() {
    int i = T_const.size() - 1;
    while ( i >= 0 ) {
        if (T_const[i].type == val_string)
            if (T_const[i].v.s == bufer)
                return Lexema(lex_t_const,i);
        i -= 1;
    }
    T_const.push_back(Var(val_string,true,true));
    T_const[T_const.size()-1].v.s = bufer;
    return Lexema (lex_t_const,T_const.size()-1);
}

Lexema Parser::init_const_real() {
    double k = strtod(bufer.c_str(),NULL);
    int i = T_const.size() - 1;
    while ( i >= 0 ) {
        if (T_const[i].type == val_real)
            if (T_const[i].v.r == k)
                return Lexema(lex_t_const,i);
        i -= 1;
    }
    T_const.push_back(Var(val_real,true,true));
    T_const[T_const.size()-1].v.r = k;
    return Lexema (lex_t_const,T_const.size()-1);
}

Lexema Parser::init_const_bool() {
    bool k = bufer == "true";
    int i = T_const.size() - 1;
    while ( i >= 0 ) {
        if (T_const[i].type == val_bool)
            if (T_const[i].v.b == k)
                return Lexema(lex_t_const,i);
        i -= 1;
    }
    T_const.push_back(Var(val_bool,true,true));
    T_const[T_const.size()-1].v.b = k;
    return Lexema (lex_t_const,T_const.size()-1);
}

Lexema Parser::init_const_int() {
    int i = T_const.size() - 1;
    long l = strtol(bufer.c_str(),NULL,10);
    while ( i >= 0 ) {
        if (T_const[i].type == val_int)
            if (T_const[i].v.i == l)
                return Lexema(lex_t_const,i);
        i -= 1;
    }
    T_const.push_back(Var(val_int,true,true));
    T_const[T_const.size()-1].v.i = l;
    return Lexema (lex_t_const,T_const.size()-1);
}




lexema_type Parser::read_operator() {
    char c1 = cur_chr, c2;
    fscanf(file,"%c",&cur_chr);
    /* однозначно односимвольные лексемы */
    switch (c1) {
        case ':': return lex_TWODOTS;
        case ',': return lex_ZPT;
        case '.': return lex_DOT;
        case ';': return lex_TZPT;
        case '{': return lex_begin;
        case '}': return lex_end;
        case '(': return lex_branch_open;
        case ')': return lex_branch_close;
        case '*': return lex_mul;
        case '/': return lex_div;
        case '%': return lex_mod;
        default: break;
    }
    /* перебор односимвольных комбинаций */
    c2 = cur_chr;
    if ( c1 == '+' && c2 != '=' )
        return lex_plus;
    if ( c1 == '-' && c2 != '=' )
        return lex_minus;
    if ( c1 == '=' && c2 != '=' )
        return lex_mov;
    if ( c1 == '<' && c2 != '=' )
        return lex_L;
    if ( c1 == '>' && c2 != '=' )
        return lex_G;
    if ( c1 == '!' && c2 != '=' )
        return lex_not;
    if ( !is_special(c2) )
        throw "unknown lexema";
    /* двусимвольные комбинации */
    fscanf(file,"%c",&cur_chr);
    if ( c1 == '&' && c2 == '&' )
        return lex_and;
    if ( c1 == '|' && c2 == '|' )
        return lex_or;
    if ( c1 == '+' && c2 == '=' )
        return lex_add;
    if ( c1 == '-' && c2 == '=' )
        return lex_sub;
    if ( c1 == '=' && c2 == '=' )
        return lex_E;
    if ( c1 == '<' && c2 == '=' )
        return lex_LE;
    if ( c1 == '>' && c2 == '=' )
        return lex_GE;
    if ( c1 == '!' && c2 == '=' )
        return lex_NE;
    throw "unknown lexema";
}

lexema_type Parser::read_number() {
    int was_dot = 0;
    bufer.push_back(cur_chr);
    while (1) {
        fscanf(file,"%c",&cur_chr);
        if ( is_digit(cur_chr) ) {
            bufer.push_back(cur_chr);
            continue;
        }
        if ( cur_chr == '.' ) {
            if (was_dot)
                throw "two DOTs in one number";
            was_dot = 1;
            bufer.push_back('.');
            fscanf(file,"%c",&cur_chr);
            if ( !is_digit(cur_chr) )
                throw "unexpected DOT after digit, bun not a real number";
            bufer.push_back(cur_chr);
            continue;
        }
        if ( is_space(cur_chr) || is_special(cur_chr) ) {
            if (was_dot)
                return lex_real_const;
            else
                return lex_int_const;
        }
        if ( is_regular(cur_chr) )
            throw "alpha after digit";
        throw cur_chr;
    }
}

lexema_type Parser::read_word() {
    bufer.push_back(cur_chr);
    while (1) {
        fscanf(file,"%c",&cur_chr);
        if ( is_regular(cur_chr) || is_digit(cur_chr) ) {
            bufer.push_back(cur_chr);
            continue;
        }
        if ( is_space(cur_chr) || is_special(cur_chr) )
            break;
        throw cur_chr;
    }
    if ( bufer == "program" )
        return lex_program;
    if ( bufer == "struct" )
        return lex_struct;
    if ( bufer == "read" )
        return lex_read;
    if ( bufer == "write" )
        return lex_write;
    if ( bufer == "writeln" )
        return lex_writeln;
    if ( bufer == "if" )
        return lex_if;
    if ( bufer == "else" )
        return lex_else;
    if ( bufer == "for" )
        return lex_for;
    if ( bufer == "while" )
        return lex_while;
    if ( bufer == "goto" )
        return lex_goto;
    if ( bufer == "continue" )
        return lex_continue;
    if ( bufer == "break" )
        return lex_break;
    if ( bufer == "true" )
        return lex_true;
    if ( bufer == "false" )
        return lex_false;
    if ( bufer == "int" )
        return lex_int;
    if ( bufer == "bool" )
        return lex_bool;
    if ( bufer == "string" )
        return lex_string;
    if ( bufer == "real" )
        return lex_real;
    return lex_id;
}

lexema_type Parser::get_one_lexem() {
    bufer.erase();
    /* пропуск пробелов */
    if (cur_chr == '\n') current_line += 1;
    while ( is_space(cur_chr) ) {
        if ( fscanf(file,"%c",&cur_chr) == EOF )
            return lex_NULL;
        if ( cur_chr == '#' ) /* однострочный коммент */
            while (cur_chr != '\n')
                fscanf(file,"%c",&cur_chr);
        if (cur_chr == '\n')
            current_line += 1;
    }
    if ( is_regular(cur_chr) )
        return read_word();
    else if ( is_special(cur_chr) )
        return read_operator();
    else if ( is_digit(cur_chr) )
        return read_number();
    else if (cur_chr == '"') {
        /* константная строка */
        do {
            fscanf(file,"%c",&cur_chr);
            if (cur_chr == '\n') throw "string cut by ENTER";
            bufer.push_back(cur_chr);
        } while ( cur_chr != '"' );
        cur_chr = ' ';
        bufer.resize(bufer.size()-1);
        return lex_string_const;
    }
    throw cur_chr;
}

const char* Parser::lex_name(lexema_type t) {
    switch(t) {
        case lex_program: return "program";
        case lex_if: return "if";
        case lex_else: return "else";
        case lex_for: return "for";
        case lex_while: return "while";
        case lex_goto: return "goto";
        case lex_continue: return "continue";
        case lex_break: return "break";
        case lex_int: return "int";
        case lex_bool: return "bool";
        case lex_real: return "real";
        case lex_string: return "string";
        case lex_id: return "id";
        case lex_begin: return "{";
        case lex_end: return "}";
        case lex_branch_open: return "(";
        case lex_branch_close: return ")";
        case lex_ZPT: return ",";
        case lex_DOT: return ".";
        case lex_TWODOTS: return ":";
        case lex_NULL: return "END_PROGRAM";
        case lex_TZPT: return ";";
        case lex_read: return "read";
        case lex_write: return "write";
        case lex_plus: return "+";
        case lex_minus: return "-";
        case lex_mul: return "*";
        case lex_div: return "/";
        case lex_mod: return "%";
        case lex_G: return ">";
        case lex_GE: return ">=";
        case lex_L: return "<";
        case lex_LE: return "<=";
        case lex_E: return "==";
        case lex_NE: return "!=";
        case lex_and: return "&&";
        case lex_or: return "||";
        case lex_not: return "!";
        case lex_mov: return "=";
        case lex_add: return "+=";
        case lex_sub: return "-=";
        case lex_true: return "true";
        case lex_false: return "false";
        case lex_int_const: return "const_int";
        case lex_string_const: return "const_string";
        case lex_real_const: return "const_real";
        case lex_adress: return "adress";
        case lex_t_operators: return "oper";
        case lex_t_var: return "var";
        case lex_t_const: return "const";
        case lex_jmp: return "jmp";
        case lex_jF: return "jF";
        case lex_empty: return "EMPTY";
        default: throw "ERROR 10";
    }
}


void Parser::execute() {
    int size_gl = T_var.size();
    Lexema lex;
    std::vector <Lexema> st;
    int index = 0, kk;
    Var vr;
    while ( poliz[index].type != lex_NULL ) {
        lex = poliz[index];
        switch(lex.type) {
        case lex_empty: break;
        case lex_t_var: case lex_t_const: case lex_adress:
            st.push_back(lex);
            break;
        case lex_t_operators:
            T_oper[lex.i]->apply(st,T_var,T_const);
            break;
        case lex_TZPT:
            st.clear();
            while ((int)T_var.size() != size_gl)
                T_var.pop_back();
            break;
        case lex_jmp:
            index = st.back().i; // сейчас в стеке Lexema<lex_adress, поле_куда_прыгать>
            st.pop_back();
            break;
        case lex_jF:
            kk = st.back().i;
            st.pop_back();
            vr = st.back().type == lex_t_var ? T_var[st.back().i] : T_const[st.back().i];
            if (vr.is_rv && !vr._is_const)
                T_var.pop_back();
            st.pop_back();
            if ( ! vr.v.b )
                index = kk;
            break;
        default:
        throw "ERROR 11";
        }
        index += 1;
    }
}




void Parser::analyze() {
    infile.open("/Users/yurygornostaev/VisualcodeProjects /prak/prak 4/inter/input.txt");
    
    init_symbols_array();
    build_result_string = "";
    poliz_string = "";
    result_string = "";
    line = 0;
    try {
        analysis();
        for (int i = 0; i < (int)poliz.size(); i++) {
            poliz_string += string(lex_name(poliz[i].type));
            poliz_string += string(" - ");
            poliz_string += string(to_string(poliz[i].i));
            poliz_string += string("\n");
        }
        // cout << poliz_string << endl;
        build_result_string += "Build Succeeded";
        execute();
        result_string += "\nCOMPLETE";
    }
    catch (char c) {
        build_result_string += string(&"unexpected symbol: " [ c]);
        line = current_line;
    }
    catch (const char * source) {
        build_result_string += string(source);
        line = current_line;
    }
    catch (std::string &source) {
        build_result_string += source;
        line = current_line;
    }
    catch (int source) {
        build_result_string += "int throw\n";
        line = 0;
    }
    catch (lexema_type source) {
        
        string t = "unexpected lexema ";
        if (source == lex_id || _is_const(lex_id)) {
            build_result_string = string(t + lex_name(source) + ' ' + bufer);
        } else {
            build_result_string = string(t + lex_name(source));
        }
        line = current_line;
    }
    catch (Execution_error s) {
        string ex_er = "execution error";
        build_result_string = string(ex_er + s.s.c_str());
    }
    infile.close();
}

void interpretator(const char * program) {
    Parser p(program);
    p.analyze();
}

int lineNumber() {
    return line;
}

const char * build_result() {
    return build_result_string.data();
}

const char * run_result() {
    return result_string.data();
}

const char* poliz_result() {
    return poliz_string.data();
}

int main(int argc, char *argv[]) {
    if(argc < 2){
        cout << "Дайте на вход имя файла" << endl;
        exit(1);
    }
    int line_n;
    interpretator(argv[1]);
    line_n = lineNumber();
    if(line_n == 0){
        cout << lineNumber() << endl;
        cout << "-----------------------------" << endl;
        cout << build_result() << endl;
        cout << "-----------------------------" << endl;
        cout << run_result() << endl;
        cout << "-----------------------------\nПОЛИЗ:\n" << endl;
        cout << poliz_string << endl;
        return 0;
    }
    else{
        cout << lineNumber() << endl;
        cout << build_result() << endl;
    }
}