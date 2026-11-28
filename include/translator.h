#ifndef ALGEBRAIC_EXPRESSION_TRANSLATOR_TRANSLATOR_H
#define ALGEBRAIC_EXPRESSION_TRANSLATOR_TRANSLATOR_H

#include <iostream>
#include <cmath>

using namespace std;

constexpr size_t MAX_VECTOR_SIZE = 100000000;
constexpr size_t COEFF = 2;

// for comparing double with zero
constexpr double COMPARE_NUM = 0.0000001;

template<typename T>
class Vector
{
protected:
  size_t sz;
  size_t capacity;
  T* pMem;
  static size_t checked_size(size_t s) {
    if (s == 0)
      throw out_of_range("Vector size should be greater than zero");
    if (s > MAX_VECTOR_SIZE)
      throw out_of_range("Too much vector size");
    return s;
  }
  static size_t expanded_capacity(size_t size) {
    if (size > (MAX_VECTOR_SIZE - 1) / COEFF)
      return MAX_VECTOR_SIZE;

    return size * COEFF + 1;
  }
  void reallocate(size_t new_capacity) {
    T* temp = new T[new_capacity]();

    try {
      for (size_t i = 0; i < sz; ++i)
        temp[i] = pMem[i];
    }
    catch (...) {
      delete[] temp;
      throw;
    }
    delete[] pMem;
    pMem = temp;
    capacity = new_capacity;
  }
public:
  explicit Vector(size_t size = 0) {
    if (size == 0) {
      pMem = nullptr;
      sz = 0;
      capacity = 0;
    } else {
      sz = checked_size(size);
      capacity = expanded_capacity(sz);
      pMem = new T[capacity]();
    }
  }
  Vector(const Vector& v): sz(v.sz), capacity(v.capacity), pMem(capacity == 0 ? nullptr : new T[capacity]()) {
    try {
      for (size_t i = 0; i < sz; ++i)
        pMem[i] = v.pMem[i];
    }
    catch (...) {
      delete[] pMem;
      throw;
    }
  }
  Vector& operator=(const Vector& v) {
    if (this != &v) {
      Vector temp(v);
      swap(sz, temp.sz);
      swap(capacity, temp.capacity);
      swap(pMem, temp.pMem);
    }
    return *this;
  }
  ~Vector() {
    delete[] pMem;
    pMem = nullptr;
    sz = 0;
    capacity = 0;
  }
  void push_back(T val) {
    if (sz == MAX_VECTOR_SIZE)
      throw out_of_range("Vector reached maximum size");

    if (sz == capacity)
      reallocate(expanded_capacity(sz));

    pMem[sz] = val;
    ++sz;
  }
  void pop_back() {
    if (sz == 0)
      throw std::out_of_range("Vector is empty");
    pMem[sz - 1] = T{};
    --sz;
  }
  T& operator[](size_t index) {
    if (index >= sz)
      throw out_of_range("Vector index out of range");
    return pMem[index];
  }
  const T& operator[](size_t index) const {
    if (index >= sz)
      throw out_of_range("Vector index out of range");
    return pMem[index];
  }

  const T& top() const {
    if (is_empty())
      throw std::out_of_range("Vector is empty");
    return pMem[sz - 1];
  }

  size_t size() const noexcept { return sz; }
  size_t get_capacity() const noexcept { return capacity; }
  bool is_empty() const noexcept { return sz == 0; }
};

template<class M>
class Stack: public Vector<M> {
public:
  const M& top() const {
    if (is_empty())
      throw std::out_of_range("Stack is empty");
    return this->pMem[this->sz - 1];
  }
  void push(const M& value) {
    this->push_back(value);
  }
  void pop() {
    if (is_empty())
      throw std::out_of_range("Stack is empty");
    this->pop_back();
  }
  bool is_empty() const noexcept { return this->sz == 0; }
  bool is_full() const noexcept { return this->sz == this->capacity; }
};

template<typename T>
class Queue {
protected:
  Stack<T> input;
  Stack<T> output;
  size_t capacity;

  void transfer() {
    if (!output.is_empty())
      return;

    while (!input.is_empty()) {
      output.push(input.top());
      input.pop();
    }
  }
public:
  explicit Queue(size_t max_size = 100): capacity(max_size) {
    if (capacity == 0 || capacity > MAX_VECTOR_SIZE)
      throw invalid_argument("Invalid queue capacity");
  }

  void push(const T& val) {
    if (is_full())
      throw overflow_error("Queue is full");
    input.push(val);
  }
  const T& front() {
    if (is_empty())
      throw out_of_range("Queue is empty");
    if (output.is_empty())
      transfer();
    return output.top();
  }
  void pop() {
    if (is_empty())
      throw out_of_range("Queue is empty");
    if (output.is_empty())
      transfer();
    output.pop();
  }

  size_t size() const noexcept {
    return input.size() + output.size();
  }
  size_t get_capacity() const noexcept {
    return capacity;
  }

  bool is_empty() const noexcept {
    return input.is_empty() && output.is_empty();
  }
  bool is_full() const noexcept {
    return size() == capacity;
  }
  void clear() {
    while (!input.is_empty())
      input.pop();
    while (!output.is_empty())
      output.pop();
  }
};











enum class  types {number, operation, open_bracket, close_bracket};

class Term{
protected:
  types type;
public:
  virtual ~Term() = default;
  types get_type() const noexcept { return type; }
};

class Number : public Term{
  double value;
public:
  explicit Number(double val){
  value = val;
  type = types::number;
  }
  double get_value() const noexcept { return value; }
};

class Operation: public Term{
  char op;
  size_t priority;
public:
  explicit Operation(char _op){
    op = _op;
    if (op == '+' || op == '-'){
      priority = 1;
    }
    else if (op == '*' || op == '/'){
      priority = 2;
    }
    else {
      throw invalid_argument("Unknown operation");
    }
    type = types::operation;
  }

  double apply(double left, double right) const {
    if (op == '/' && abs(right) < COMPARE_NUM)
      throw domain_error("Division by zero");

    double result = 0.0;

    switch (op) {
      case '+':
        result = left + right;
        break;

      case '-':
        result = left - right;
        break;

      case '*':
        result = left * right;
        break;

      case '/':
        result = left / right;
        break;

      default:
        throw invalid_argument("Unknown operation");
    }

    if (!std::isfinite(result))
      throw overflow_error("Non-finite result");

    return result;
  }

  char get_operation() const noexcept {
    return op;
  }

  size_t get_priority() const noexcept {
    return priority;
  }
};

class OpenBracket : public Term{
  char bracket;
public:
  OpenBracket(){
    bracket = '(';
    type = types::open_bracket;
  }
};

class CloseBracket : public Term{
  char bracket;
public:
    CloseBracket(){
      bracket = ')';
      type = types::close_bracket;
    }
};

class Translator{
protected:
  string expression;
  Vector<Term *> terms;
  Vector<Term *> polish_notation;

  static void clear_pointers(Vector<Term*>& v) {
    while (v.size() != 0)
      v.pop_back();
  }
  void clear_terms() {
    clear_pointers(polish_notation);
    for (size_t i = 0; i < terms.size(); ++i)
      delete terms[i];
    clear_pointers(terms);
  }
  static bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
  }
  void add_term(Term* term) {
    try {
      terms.push_back(term);
    }
    catch (...) {
      delete term;
      throw;
    }
  }
public:
  Translator(string expr): expression(expr) {}
  Translator(const Translator&) = delete;
  Translator& operator=(const Translator&) = delete;
  ~Translator() {
    for (size_t i = 0; i < terms.size(); ++i) delete terms[i];
  }
  void expression_to_terms() {
    clear_terms();
    try {
      bool status = false;
      string N;
      bool has_separator = false;

      for (size_t i = 0; i < expression.size(); ++i) {
        char c = expression[i];
        if (isspace(static_cast<unsigned char>(c))) {
          if (status) {
            if (N.back() == '.') { throw std::invalid_argument("Number finishes with separator"); }
            double num = stod(N);
            add_term(new Number(num));
            N.clear();
            status = false;
            has_separator = false;
          }
          continue;
        }
        if (!status) {
          if (is_digit(c) || c == '.' || c == ',') {
            status = true;
            N = c;
            if (c == '.' || c == ',') {
              throw std::invalid_argument("Number started with separator");
            }
          } else {
            switch (c) {
              case '+':
              case '-':
              case '*':
              case '/':
                add_term(new Operation(c));
                break;
              case '(':
              case '[':
              case '{':
                add_term(new OpenBracket());
                break;
              case ')':
              case ']':
              case '}':
                add_term(new CloseBracket());
                break;
              default:
                throw invalid_argument("Unknown character at position " + to_string(i + 1));
            }

          }
          continue;
        }
        if (status) {
          if (is_digit(c) || c == '.' || c == ',') {
            if (c == '.' || c == ',') {
              if (has_separator) {throw std::invalid_argument("Repeated separator");}
              has_separator = true;
              N += '.';
            } else {
              N += c;
            }
          } else {
            if (N.back() == '.') { throw std::invalid_argument("Number finishes with separator"); }
            double num = stod(N);
            add_term(new Number(num));
            N.clear();
            status = false;
            has_separator = false;
            switch (c) {
              case '+':
              case '-':
              case '*':
              case '/':
                add_term(new Operation(c));
                break;
              case '(':
              case '[':
              case '{':
                add_term(new OpenBracket());
                break;
              case ')':
              case ']':
              case '}':
                add_term(new CloseBracket());
                break;
              default:
                throw invalid_argument("Unknown character at position " + to_string(i + 1));
            }
          }
          continue;
        }
      }
      if (status) {
        if (N.back() == '.') { throw std::invalid_argument("Number finishes with separator"); }
        double num = stod(N);
        add_term(new Number(num));
      }
    }
    catch (...) {
      clear_terms();
      throw;
    }
  }

  int is_bracket_correct() {
    Stack<char> stack;
    for (size_t i = 0; i < expression.size(); ++i) {
      char c = expression[i];
      switch (c) {
        case '(':
        case '[':
        case '{':
          stack.push(c);
          break;
        case ')':
          if (stack.is_empty() || stack.top() != '(') { return 1; }
          stack.pop();
          break;
        case ']':
          if (stack.is_empty() || stack.top() != '[') { return 1; }
          stack.pop();
          break;
        case '}':
          if (stack.is_empty() || stack.top() != '{') { return 1; }
          stack.pop();
          break;
      }
    }
    return !stack.is_empty();
  }

  void syntax_check() {
    if (terms.size() == 0) throw invalid_argument("Empty expression");
    if (is_bracket_correct() == 1) throw invalid_argument("Invalid bracket sequence");

    types current = types::number;
    if (terms[0]->get_type() == types::close_bracket || terms[0]->get_type() == types::operation) {throw std::invalid_argument("Expression starts with close bracket or operator");}
    if (terms[0]->get_type() == types::open_bracket) {current = types::open_bracket;}

    for (size_t i = 1; i < terms.size(); ++i) {
      // number
      if (current == types::number) {
        switch (terms[i]->get_type()) {
          case types::open_bracket:
          case types::number:
            throw std::invalid_argument("Invalid expression");
          case types::close_bracket:
            current = types::close_bracket;
            break;
          case types::operation:
            current = types::operation;
        }
      }
      // operation
      else if (current == types::operation) {
        switch (terms[i]->get_type()) {
          case types::close_bracket:
          case types::operation:
            throw std::invalid_argument("Invalid expression");
          case types::open_bracket:
            current = types::open_bracket;
            break;
          case types::number:
            current = types::number;
        }
      }
      // close bracket
      else if (current == types::close_bracket) {
        switch (terms[i]->get_type()) {
          case types::number:
          case types::open_bracket:
            throw std::invalid_argument("Invalid expression");
          case types::operation:
            current = types::operation;
            break;
          case types::close_bracket:
            current = types::close_bracket;
        }
      }
      // open bracket
      else if (current == types::open_bracket) {
        switch (terms[i]->get_type()) {
          case types::operation:
          case types::close_bracket:
            throw std::invalid_argument("Invalid expression");
          case types::open_bracket:
            current = types::open_bracket;
            break;
          case types::number:
            current = types::number;
        }
      }
    }
    if (terms.top()->get_type() == types::open_bracket || terms.top()->get_type() == types::operation) {
      throw std::invalid_argument("Invalid finishing of expression");
    }
  };

  void to_polish_notation() {
    syntax_check();
    while (polish_notation.size() != 0)
      polish_notation.pop_back();

    Stack<Term*> stack;
    for (size_t i = 0; i < terms.size(); ++i) {
      if (terms[i]->get_type() == types::number) {polish_notation.push_back(terms[i]);}
      else if (terms[i]->get_type() == types::open_bracket) {stack.push(terms[i]);}
      else if (terms[i]->get_type() == types::close_bracket) {
        while (!stack.is_empty() && stack.top()->get_type() != types::open_bracket) {
          polish_notation.push_back(stack.top());
          stack.pop();
        }
        if (stack.is_empty())
          throw invalid_argument("Unmatched closing bracket");
        stack.pop();
      }
      else if (terms[i]->get_type() == types::operation) {
        const Operation* current = static_cast<const Operation*>(terms[i]);

        while (!stack.is_empty() && stack.top()->get_type() == types::operation) {
          const Operation* previous = static_cast<const Operation*>(stack.top());

          if (previous->get_priority() < current->get_priority()) break;

          polish_notation.push_back(stack.top());
          stack.pop();
        }
        stack.push(terms[i]);
      }
    }
    while (!stack.is_empty()) {
      if (stack.top()->get_type() == types::open_bracket)
        throw invalid_argument("Unmatched opening bracket");
      polish_notation.push_back(stack.top());
      stack.pop();
    }
  };
  double calculate() {
    expression_to_terms();
    syntax_check();
    to_polish_notation();
    Stack<double> stack;
      for (size_t i = 0; i < polish_notation.size(); ++i) {
        if (polish_notation[i]->get_type() == types::number) {
          const Number* number = static_cast<const Number*>(polish_notation[i]);
          stack.push(number->get_value());
        }
        else if (polish_notation[i]->get_type() == types::operation) {
          if (stack.size() < 2)
            throw invalid_argument("Not enough operands");
          double b = stack.top();
          stack.pop();
          double a = stack.top();
          stack.pop();
          const Operation* operation = static_cast<const Operation*>(polish_notation[i]);

          stack.push(operation->apply(a, b));
        }
        else {
          throw invalid_argument("Unexpected term in polish notation");
        }
      }
    if (stack.size() != 1)
      throw invalid_argument("Invalid polish notation");
    return stack.top();
  };
};

#endif //ALGEBRAIC_EXPRESSION_TRANSLATOR_TRANSLATOR_H