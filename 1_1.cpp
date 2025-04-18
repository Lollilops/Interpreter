#include <iostream>
#include <cstring>
using namespace std;

class Arr{
    int *arr;
    int size;
public:
    Arr(int s, int date = 0): size(s){
        arr = new int[s];
        for(int i = 0; i<size; arr[i++] = date);
    }

    friend ostream &operator<<(ostream &ostream, const Arr &s){
        for(int i = 0; i<s.size; i++){
            cout << s.arr[i] << " ";
        }
        cout << endl;
        return ostream;
    }

    friend Arr operator|(const Arr &a, int k){
        Arr tmp(a.size);
        for(int i = 0; i < a.size; i ++){
                tmp.arr[i] = a.arr[i] + k;
        }
        return tmp;
    }
    friend Arr operator|(const Arr &a, const Arr &b){
        Arr tmp(min(a.size, b.size));
        for(int i = 0; i < tmp.size; i ++){
                tmp.arr[i] = a.arr[i] + b.arr[i];
        }
        return tmp;
    }
    friend Arr & operator|=(Arr &a, int k){
        for(int i = 0; i < a.size; i ++){
                a.arr[i] = a.arr[i] + k;
        }
        return a;
    }

    Arr operator-() const {
        Arr result(size);
        for (int i = 0; i < size; ++i) {
        result.arr[i] = -arr[i];
        }
        return result;
    }
    int &operator[](int index){
        if (index >= 0 && index < size){
            return arr[index];
        }
        else{
            exit(1);
        }
    }
    Arr & operator=(const Arr & a){
        if (this == &a)
                return *this;
            delete[] this->arr;
            arr = new int[a.size + 1];
            for(int i = 0; i<=size; i++){
                arr[i] = a.arr[i];
            }
            // cout << a.firm << firm << endl;
            size = a.size;
            // cout << "inf_cli &operator=(inf_cli &a)" << endl;
            return *this;
}

    ~Arr(){delete[] arr;}
};
int main(){
    Arr a1(15), a2(20,5), a3(30,3);
    const Arr a4(5);
    cout<<a1 << a2 << a3 << a4;
    a2 = a2|2; // объект а2 здесь не меняется 
    cout << "---------------" << endl;
    cout<<a1 <<a2<<a3<<a4;
    a2 |= 1; // а здесь меняется
    cout << "---------------" << endl;
    cout<<a1 <<a2<<a3<<a4;
    a3= (-a2) | (-a1);
    cout << "---------------" << endl;
    cout<<a1 <<a2<<a3<<a4;
    a3[5]=10;
    cout << "---------------" << endl;
    cout<<a1 <<a2<<a3<<a4;
    a1 = a4| a3;
    cout << "---------------" << endl;
    cout<<a1 <<a2<<a3<<a4;
}