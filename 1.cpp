// ДЗ-4. Абстрактные классы. Срок сдачи — до 23.03.24
// Написать абстрактный класс (содержание произвольное, но осмысленное) и несколько (>=2) производных от него классов. 
// Определить независимый от этой иерархии класс, который работает с массивом объектов типа абстрактного класса (через указатели).
// Функция main должна демонстрировать работу с объектами указанных классов, использование виртуальных функций

#include <iostream>
#include <vector>

using namespace std;

// Абстрактный класс Вузы
class University{
public:
    virtual int count() const = 0; // Виртуальная функция для ПГАС
    virtual ~University() {} // Виртуальный деструктор
};

// Производный класс МГУ
class MSU: public University {
private:
    int students;
public:
    MSU(int s): students(s) {}
    int count() const{
        return students/10; // кол-во студентов на ПГАС = 10 процентов от студентов
    }
    // ~MSU(){}
};

// Производный класс ВШЭ
class HSE: public University {
private:
    int students;
    int exam_passed;
public:
    HSE(int s, int e) : students(s), exam_passed(e){}
    int count() const{
        return (exam_passed * 15)/students; // кол-во студентов на ПГАС = процент сдавших * 15
    }
    // ~HSE(){}
};

// Независимый класс для работы с массивом объектов типа абстрактного класса
class Universityarray {
private:
    vector<University*> Unis;
public:
    void addUniversity(University* Uni) {
        Unis.push_back(Uni);
    }

    int totalcount() const {
        int total = 0;
        for (University* Uni : Unis) {
            total += Uni->count();
        }
        return total;
    }

    ~Universityarray() {
        while(!Unis.empty()){
            Unis.pop_back();
        }
    }
};

int main() {
    MSU M(5);
    HSE H(15, 5);

    Universityarray handler;
    handler.addUniversity(&M);
    handler.addUniversity(&H);

    cout << "Всего счастливчиков: " << handler.totalcount() << endl;

    return 0;
}
