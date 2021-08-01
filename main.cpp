#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <cctype>
#include <exception>
#include <tuple>

enum TokenType {t_Op, t_Num};
typedef int Number;

class Token {
  public:
    TokenType type;
    virtual std::string toStr() const = 0;
};

std::ostream& operator<<(std::ostream& os, const Token& token)
{
  os << token.toStr();
  return os;
}

class tNone: public Token {
  public:
    virtual std::string toStr() const {
      return std::string("<None>");
    }
};

class tOperator: public Token {
  public:
    virtual Number eval(Number num1, Number num2) = 0;
    virtual std::string toStr() const = 0;
    virtual int priority() const = 0;
};

class tAdd: public tOperator {
  public:
   
    virtual std::string toStr() const override {
      return std::string("+");
    }
    virtual Number eval(Number num1, Number num2) override {
      //std::cout << num1 << " + " << num2 << std::endl;
      return num1 + num2;
    }

    virtual int priority() const override { return 0; }
};

class tSub: public tOperator {
  public:
   
    virtual std::string toStr() const override {
      return std::string("-");
    }
    virtual Number eval(Number num1, Number num2) override {
      //std::cout << num1 << " - " << num2 << std::endl;
      return num1 - num2;
    }

    virtual int priority() const override { return 0; }
};

class tMul: public tOperator {
  public:
   
    virtual std::string toStr() const override {
      return std::string("*");
    }
    virtual Number eval(Number num1, Number num2) override {
      //std::cout << num1 << " - " << num2 << std::endl;
      return num1 * num2;
    }

    virtual int priority() const override { return 1; }
};

class tDiv: public tOperator {
  public:
    virtual std::string toStr() const override {
      return std::string("/");
    }
    virtual Number eval(Number num1, Number num2) override {
      //std::cout << num1 << " - " << num2 << std::endl;
      return num1 / num2;
    }
    virtual int priority() const override { return 1; }
};

// Singlton
auto NONE = tNone();
auto ADD = tAdd();
auto SUB = tSub();
auto MUL = tMul();
auto DIV = tDiv();

class tNumber: public Token {
  public:
    tNumber(int num): val(num) {;}
    virtual std::string toStr() const {
      return std::string(std::to_string(val));
    }
    Number val;
};

class Calculator {
  public:
    Number eval(std::string exp) {
      std::cout << "Calculating \"" << exp << "\"" << std::endl;
      std::vector<Token*> tokens;
      Token *token;
      std::string remain;

      // Read tokens
      std::tie(token, remain) = getNextToken(exp);
      while (token != &NONE) {
        tokens.push_back(token);
        std::tie(token, remain) = getNextToken(remain);
      }

      std::cout << "Infix: ";
      for (auto& t: tokens) {
        std::cout << t->toStr() << " ";
      }
      std::cout << std::endl;


      std::cout << "Posfix: ";
      auto postfix = infixToPoistfix(tokens);
      for (auto& t: postfix) {
        std::cout << t->toStr() << " ";
      }
      std::cout << std::endl;

      std::cout << "Result: " << evalPostfix(postfix) << std::endl;


      return 0; 
    }

  private:
    typedef std::tuple<Token*, std::string> lexerReturn;
    std::tuple<Token*, std::string> getNextToken(std::string str) {
      std::string sToken;
      std::cout << "Parsing " << str << std::endl;

      // Handle empty string.
      if (str.size() == 0) {
        return lexerReturn(&NONE, "");
      }

      // Ignore spaces and continue parsing.
      if (str[0] == ' ') {
        return getNextToken(str.substr(1));
      }

      // Parse number
      if (isNumber(str[0])) {
        for (int i = 1; i < str.size(); i++) {
          if (not isNumber(str[i])) {
            std::cout << "hhh" << str.substr(0, i) << std::endl;
            return lexerReturn(new tNumber(strToNum(str.substr(0, i))), str.substr(i));
          }
        }
        return lexerReturn(new tNumber(strToNum(str)), "");
      }

      // Parse operator
      switch (str[0]) {
        case '+':
          return lexerReturn(&ADD, str.substr(1));
        case '-':
          return lexerReturn(&SUB, str.substr(1));
        case '*':
          return lexerReturn(&MUL, str.substr(1));
        case '/':
          return lexerReturn(&DIV, str.substr(1));
        default:
          std::string msg("Unknown operator: \'");
          msg += str[0];
          msg += '\'';
          throw std::runtime_error(msg);
      }     
    }
    std::vector<Token*> infixToPoistfix(std::vector<Token*> infix) {
      std::stack<tOperator*> stack;
      std::vector<Token*> postfix;

      for (auto& t: infix) {
        if (dynamic_cast<tNumber*>(t)) {
          postfix.push_back(t);
        } else if (dynamic_cast<tOperator*>(t)) {
          auto op = dynamic_cast<tOperator*>(t);
          while (not stack.empty() && (stack.top()->priority() >= op->priority())) {
            postfix.push_back(stack.top());
            stack.pop();
          }
          stack.push(op);
        } else {
          std::string msg("Unknown token: ");
          msg += t->toStr();
          throw std::runtime_error(msg);
        }
      }
      while (not stack.empty()) {
        postfix.push_back(stack.top());
        stack.pop();
      }
      return postfix;
    }

    bool isNumber(char c) {
      if (std::isdigit(c)) {
        return true;
      }
      return false;
    }


    Number strToNum(std::string s) {
      return std::stoi(s);
    }

    Number evalPostfix(std::vector<Token*> postfix) {
      if (postfix.size() == 0)
        return 0;

      std::stack<Number> stack;
      Number num1, num2;
      tNumber* num;
      tOperator* op;
      for (auto& token: postfix) {
        if ((num = dynamic_cast<tNumber*>(token))) {
          stack.push(num->val);
        } else if ((op = dynamic_cast<tOperator*>(token))){
          num1 = stack.top();
          stack.pop();
          num2 = stack.top();
          stack.pop();
          stack.push(op->eval(num1, num2));
        }
      }
      
      assert(stack.size() == 1);
      return stack.top();
    }
};


int main(int argc, char *argv[])
{
  auto calculator = Calculator();
  calculator.eval(argv[1]);
  return 0;
}
