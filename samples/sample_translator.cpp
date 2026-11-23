#include "translator.h"
#include <iostream>
using namespace std;

int main() {
    string expression;
    cout << "Expression (only + - * / and brackets): ";
    if (!getline(cin, expression)) return 1;
    Translator translator(expression);
    try {
        cout << "Result: " << translator.calculate() << '\n';
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}