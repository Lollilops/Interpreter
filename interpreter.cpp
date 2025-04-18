#include "Syntactic_analyse.cpp"


class Executer {
public:
    void execute(std::vector<Lex> & poliz);
};


class Interpretator {
    Parser pars;
    Executer E;
public:
    Interpretator ( const char* program ) : pars (program) {}
    void interpretation ();
};
void Executer::execute(vector<Lex> & poliz) {
    Lex pc_el;
    stack <Lex> args;
    Lex i, j;
    int index = 0, size = poliz.size();
    int tmp;
    while ( index < size ) {
        pc_el = poliz[index];
        switch ( pc_el.get_type() ) {
            case LEX_TRUE: case LEX_FALSE: case LEX_NUM: case LEX_STR: case POLIZ_ADDRESS: case POLIZ_LABEL:
                args.push(pc_el);
                break;
            case LEX_ID:
                tmp = pc_el.get_value();
                if( TID[tmp].get_assign() ) {
                    if(TID[tmp].get_type() == LV_STRING)
                        args.push( Lex(TID[tmp].get_type(), TID[tmp].get_str_val()) );
                    else
                        args.push( Lex(TID[tmp].get_type(), TID[tmp].get_value()) );
                    break;
                }
                else{
                    cout << "Ошибка. Лексема: " << pc_el;
                    char s[BUFSIZE];
                    sprintf(s, "%s (%d): Неопределенный идентификатор\n", __FILE__, __LINE__);
                    string str = string(s);
                    throw str;
                }
            case LEX_NOT:
                from_st( args, i );
                args.push( Lex( i.get_type(), !i.get_value()) );
                break;
            case LEX_OR:
                from_st( args, i );
                from_st( args, j );
                args.push( Lex(i.get_type(), j.get_value() || i.get_value()) );
                break;
            case LEX_AND:
                from_st( args, i );
                from_st( args, j );
                args.push( Lex(i.get_type(), j.get_value() && i.get_value()) );
                break;
            case POLIZ_GO:
                from_st( args, i );
                index = i.get_value() - 1;
                break;
            case POLIZ_FGO:
                from_st ( args, i );
                from_st ( args, j );
                if ( j.get_value() == 0 ) index = i.get_value() - 1;
                break;
            case LEX_WRITE:
                from_st ( args, i );
                if(i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR)
                    cout << i.get_str() << endl;
                else if(i.get_type() == LEX_INT || i.get_type() == LV_INT || i.get_type() == LEX_NUM)
                    cout << i.get_value() << endl;
                else{
                    if(i.get_value() == 0)
                        cout << "false" << endl;
                    else
                        cout << "true" << endl;
                }
                break;
            case LEX_READ:
                int k;
                from_st ( args, i );
                tmp = i.get_value();
                if( TID[tmp].get_type () == LEX_INT || TID[tmp].get_type () == LV_INT ) {
                    cout << "Введите целое число для " << TID[tmp].get_name () << endl;
                    cin >> k;
                    TID[tmp].put_value(k);
                }
                else if( TID[tmp].get_type () == LEX_BOOL || TID[tmp].get_type () == LV_BOOL ){
                    string str;
                    while (1) {
                        cout << "Введите переменную логического типа (true or false) для " << TID[tmp].get_name() << endl;
                        cin >> str;
                        if ( str != "true" && str != "false" ) {
                            cout << "Ошибка ввода: нужно ввести true/false" << endl;
                            continue;
                        }
                        k = ( str == "true" ) ? 1 : 0;
                        TID[tmp].put_value(k);
                        break;
                    }
                }
                else{
                    string str;
                    cout << "Введите строку для " << TID[tmp].get_name() << endl;
                    cin >> str;
                    TID[tmp].put_str_val(str);

                }
                TID[tmp].put_assign();
                break;
            case LEX_PLUS:
                from_st ( args, i );
                from_st ( args, j );
                if( i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR )
                    args.push ( Lex(i.get_type(), j.get_str() + i.get_str()) );
                else
                    args.push ( Lex(i.get_type(), j.get_value() + i.get_value()) );
                break;
            case LEX_TIMES:
                from_st ( args, i );
                from_st ( args, j );
                args.push ( Lex(i.get_type(), j.get_value() * i.get_value()) );
                break;
            case LEX_MINUS:
                from_st ( args, i );
                from_st ( args, j );
                args.push ( Lex(i.get_type(), j.get_value() - i.get_value()) );
                break;
            case LEX_SLASH:
                from_st ( args, i );
                from_st ( args, j );
                if (i.get_value() != 0) {
                    args.push ( Lex(i.get_type(), j.get_value() / i.get_value()) );
                    break;
                }
                else{
                    cout << "Ошибка. Лексема: " << pc_el;
                    char s[BUFSIZE];
                    sprintf(s, "%s (%d): Деление на ноль\n", __FILE__, __LINE__);
                    string str = string(s);
                    throw str;
                }
            case LEX_EQ:
                from_st ( args, i );
                from_st ( args, j );
                if( i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR )
                    args.push ( Lex(LEX_BOOL, j.get_str() == i.get_str()) );
                else
                    args.push ( Lex(LEX_BOOL, j.get_value() == i.get_value()) );
                break;
            case LEX_LSS:
                from_st ( args, i );
                from_st ( args, j );
                if( i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR )
                    args.push ( Lex(LEX_BOOL, j.get_str() < i.get_str()) );
                else
                    args.push ( Lex(LEX_BOOL, j.get_value() < i.get_value()) );
                break;
            case LEX_GTR:
                from_st ( args, i );
                from_st ( args, j );
                if( i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR )
                    args.push ( Lex(LEX_BOOL, j.get_str() > i.get_str()) );
                else
                    args.push ( Lex(LEX_BOOL, j.get_value() > i.get_value()) );
                break;

            case LEX_LEQ:
                from_st ( args, i );
                from_st ( args, j );
                args.push ( Lex(LEX_BOOL, j.get_value() <= i.get_value()) );
                break;
            case LEX_GEQ:
                from_st ( args, i );
                from_st ( args, j );
                args.push ( Lex(LEX_BOOL, j.get_value() >= i.get_value()) );
                break;
            case LEX_NEQ:

                from_st ( args, i );
                from_st ( args, j );
                if( i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR )
                    args.push ( Lex(LEX_BOOL, j.get_str() != i.get_str()) );
                else
                    args.push ( Lex(LEX_BOOL, j.get_value() != i.get_value()) );
                break;
            case LEX_ASSIGN:
                from_st ( args, i );
                from_st ( args, j );
                if( i.get_type() == LEX_STRING || i.get_type() == LV_STRING || i.get_type() == LEX_STR ){
                    TID[j.get_value()].put_str_val(i.get_str());
                    args.push( Lex(POLIZ_ADDRESS, j.get_str(), j.get_value()) );
                }
                else if(i.get_type() == POLIZ_ADDRESS){
                    TID[j.get_value()].put_value(TID[i.get_value()].get_value());
                    args.push( Lex(POLIZ_ADDRESS, j.get_str(), j.get_value()) );
                }
                else{
                    TID[j.get_value()].put_value(i.get_value());
                    args.push( Lex(POLIZ_ADDRESS, j.get_str(), j.get_value()) );
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
        } //end of switch
        ++index;
    }; //end of while
    cout << "Finish of executing!!!" << endl;
}
void Interpretator::interpretation () {
    pars.analyze();
    E.execute(pars.poliz);
}


int main(int argc, char *argv[]) {
    if(argc < 2){
        cout << "Дайте на вход имя файла" << endl;
        exit(1);
    }
    Scanner scan(argv[1]);
    while (1) {
        try{
            Lex l1 = scan.get_lex();
            cout<<l1;
        }
        catch(string str){
            if (str == "+++++++++++OK+++++++++++") {
                Lex l2 = Lex(LEX_END, '}');
                cout<<l2;
                cout << str << endl;
                break;
            }
            else {
                cout << str << endl;
                return 0;
            }
        }
    }
    try{
        Interpretator I(argv[1]);
        I.interpretation();
    }
    catch(string str){ cout << str; }
    return 0;
}