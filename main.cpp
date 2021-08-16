#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <cctype>
#include <exception>
#include <tuple>
#include <cstdlib>

// TODO:
// [v] free memory
// [v] paranthesis
// [ ] floating point support
// [ ] EOF class
//
//
bool DEBUG = false;

typedef int Number;

class Token {
  public:
    virtual ~Token(){};
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
    tOperator(std::string s, int p, std::function<Number(Number, Number)> e):
      _str(s), _priority(p), _eval(e) {;}

    Number eval(Number num1, Number num2) {
      return _eval(num1, num2);
    }

    std::string toStr() const {
      return _str;
    }

    virtual int priority() const {
      return _priority;
    }

  private:
    std::string _str;
    int _priority;
    std::function<Number(Number, Number)> _eval;
};

class tNumber: public Token {
  public:
    tNumber(int num): val(num) {;}
    virtual std::string toStr() const {
      return std::string(std::to_string(val));
    }
    Number val;
};

class tLeftParenthesis: public Token {
  public:
    virtual std::string toStr() const {
      return std::string("(");
    }
};

class tRightParenthesis: public Token {
  public:
    virtual std::string toStr() const {
      return std::string(")");
    }
};



typedef std::shared_ptr<Token> TokenPtr;

class Calculator {
  public:
    Calculator():
      _ENDOFFILE(new tOperator("<EOF>", 0, [](Number, Number){ return 0;})),
      _ADD(new tOperator("+", 0, [](Number a, Number b){ return a + b;})),
      _SUB(new tOperator("-", 0, [](Number a, Number b){ return a - b;})),
      _MUL(new tOperator("*", 1, [](Number a, Number b){ return a * b;})),
      _DIV(new tOperator("/", 1, [](Number a, Number b){ return a / b;})),
      _LEFT_PARENTHESIS(new tLeftParenthesis()),
      _RIGHT_PARENTHESIS(new tRightParenthesis()){;}

    Number eval(std::string exp) {
      if (DEBUG) {
        std::cout << "Calculating \"" << exp << "\"" << std::endl;
      }
      std::vector<TokenPtr> tokens;
      TokenPtr token;
      std::string remain;

      // Read tokens
      std::tie(token, remain) = getNextToken(exp);
      while (token.get() != _ENDOFFILE.get()) {
        tokens.emplace_back(token);
        std::tie(token, remain) = getNextToken(remain);
      }

      if (DEBUG) {
        std::cout << "Infix: ";
        for (auto& t: tokens) {
          std::cout << t->toStr() << " ";
        }
        std::cout << std::endl;
      }


      auto postfix = infixToPostfix(tokens);
      if (DEBUG) {
        std::cout << "Posfix: ";
        for (auto& t: postfix) {
          std::cout << t->toStr() << " ";
        }
        std::cout << std::endl;
      }

      // Print result
      std::cout << evalPostfix(postfix) << std::endl;


      return 0;
    }

  private:
    typedef std::tuple<TokenPtr, std::string> tokenStrTuple;
    std::tuple<TokenPtr, std::string> getNextToken(std::string str) {
      if (DEBUG) {
        std::cout << "Parsing " << str << std::endl;
      }

      // Handle empty string.
      if (str.size() == 0) {
        return tokenStrTuple(_ENDOFFILE, "");
      }

      // Ignore spaces and continue parsing.
      if (str[0] == ' ') {
        return getNextToken(str.substr(1));
      }

      // Parse number
      if (isNumber(str[0])) {
        for (int i = 1; i < str.size(); i++) {
          // Get substring until non-digit char
          if (not isNumber(str[i])) {
            return tokenStrTuple(new tNumber(strToNum(str.substr(0, i))), str.substr(i));
          }
        }
        // The whole str is a number
        return tokenStrTuple(new tNumber(strToNum(str)), "");
      }

      // Parse operator
      switch (str[0]) {
        case '+':
          return tokenStrTuple(_ADD, str.substr(1));
        case '-':
          return tokenStrTuple(_SUB, str.substr(1));
        case '*':
          return tokenStrTuple(_MUL, str.substr(1));
        case '/':
          return tokenStrTuple(_DIV, str.substr(1));
        case '(':
          return tokenStrTuple(_LEFT_PARENTHESIS, str.substr(1));
        case ')':
          return tokenStrTuple(_RIGHT_PARENTHESIS, str.substr(1));
        default:
          std::string msg("Unknown operator: \'");
          throw std::runtime_error(msg);
      }
    }

    std::vector<TokenPtr> infixToPostfix(std::vector<TokenPtr> infix) {
      std::stack<TokenPtr> stack;
      std::vector<TokenPtr> postfix;

      for (auto& t: infix) {
        if (dynamic_cast<tNumber*>(t.get())) {
          postfix.push_back(t);
        } else if (dynamic_cast<tLeftParenthesis*>(t.get())) {
          stack.push(t);
        } else if (dynamic_cast<tRightParenthesis*>(t.get())) {
          while (not dynamic_cast<tLeftParenthesis*>(stack.top().get())) {
            // pop until encounter '('
            postfix.push_back(stack.top());
            stack.pop();
          }
          stack.pop();
        } else if (dynamic_cast<tOperator*>(t.get())) {
          auto op = std::dynamic_pointer_cast<tOperator>(t);
          while (not stack.empty() &&
                 not dynamic_cast<tLeftParenthesis*>(stack.top().get()) &&
                 (dynamic_cast<tOperator*>(stack.top().get())->priority() >= op->priority())) {
            postfix.push_back(stack.top());
            stack.pop();
          }
          stack.push(t);
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

    Number evalPostfix(std::vector<TokenPtr> postfix) {
      if (postfix.size() == 0)
        return 0;

      std::stack<Number> stack;
      Number num1, num2;
      for (auto& token: postfix) {
        std::shared_ptr<tNumber> num(std::dynamic_pointer_cast<tNumber>(token));
        std::shared_ptr<tOperator> op(std::dynamic_pointer_cast<tOperator>(token));
        if (num) {
          stack.push(num->val);
        } else if (op){
          num1 = stack.top();
          stack.pop();
          num2 = stack.top();
          stack.pop();
          stack.push(op->eval(num2, num1));
        }
      }

      assert(stack.size() == 1);
      return stack.top();
    }

    std::shared_ptr<tOperator> _ENDOFFILE, _ADD, _SUB, _MUL, _DIV;
    std::shared_ptr<tLeftParenthesis> _LEFT_PARENTHESIS;
    std::shared_ptr<tRightParenthesis> _RIGHT_PARENTHESIS;
};


int main(int argc, char *argv[])
{
  const char* env_p = std::getenv("DEBUG");
  if(env_p && env_p == std::string("TRUE")) {
    DEBUG = true;
  }
  auto calculator = Calculator();
  if (argc == 2) {
    calculator.eval(argv[1]);
  }
  return 0;
}
