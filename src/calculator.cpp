#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"

#include "std_lib_facilities.h"

double expression();

class Variable
{
public:
    string name;
    double value;
    Variable(string s, double d) :name{s}, value{d} {}
};

class Variable_table
{
public:
    double get_value(string s);
    void set_value(string s, double d);
    void add_var(string s, double d);
    bool exists(string s);
    void add_const(string s, double d);
    bool is_constant(string s);
private:
    vector<string> const_table;
    vector<Variable> var_table;
};

Variable_table vt;

double Variable_table::get_value(string s)
{
    for (const Variable& v: var_table)
        if (v.name == s) return v.value;
    error("get: undefined variable ", s);
}

void Variable_table::set_value(string s, double d)
{
    for (Variable& v: var_table)
        if (v.name == s) {
            v.value = d;
            return;
        }
}

void Variable_table::add_var(string s, double d)
{
    Variable var = Variable(s, d);
    var_table.push_back(var);
}

bool Variable_table::exists(string s)
{
    for (const Variable& v: var_table)
        if (v.name == s) return true;
    return false;
}

void Variable_table::add_const(string s, double d)
{
    Variable con = Variable(s, d);
    var_table.push_back(con);
    const_table.push_back(s);
}

bool Variable_table::is_constant(string s)
{
    for (const string& v: const_table)
        if (v == s) return true;
    return false;
}

class Token
{
public:
    char kind;
    double value;
    string name;
    Token() :kind{0} {}
    Token(char k) :kind{k}, value{0.0} {}
    Token(char k, double v) :kind{k}, value(v) {}
    Token(char ch, string n) :kind{ch}, name{n} {}
};

class Token_stream
{
public:
    Token get();
    void putback(Token t);
    void ignore(char c);
private:
    bool full {false};
    Token buffer {0};
};

const char number = '8';
const char quit = 'q';
const char print = ';';
const char name = 'a';
const char let = 'L';
const char assign = '=';
const string declkey = "let";

Token Token_stream::get()
{
    if (full) {
        full = false;
        return buffer;
    }
    char ch;
    cin >> ch;

    switch (ch) {
        case print:
        case quit:
        case assign:
        case '(':
        case ')':
        case '+':
        case '-':
        case '*':
        case '/':
            return Token(ch);
        case '.': case '0': case '1': case '2': case '3': case '4': 
        case '5': case '6': case '7': case '8': case '9':
        {
            cin.putback(ch);
            double val;
            cin >> val;
            return Token(number, val);
        }
        default:
            if (isalpha(ch)) {
                string s;
                s += ch;
                while (cin.get(ch) && (isalpha(ch) || isdigit(ch))) s += ch;
                cin.putback(ch);
                if (s == declkey) return Token{let};
                return Token{name, s};
            }
            error("Bad token\n");
    }
}

void Token_stream::putback(Token t) 
{
    if (full) error("putback() into a full buffer");
    buffer = t;
    full = true;
}

void Token_stream::ignore(char c)
{
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;

    char ch = 0;
    while (cin >> ch)
        if (ch == c) return;
}

Token_stream ts;

double primary()
{
    Token t = ts.get();
    switch (t.kind) {
        case '(':
        {
            double d = expression();
            t = ts.get();
            if (t.kind != ')') error("')' expected");
            return d;
        }
        case number:
        {
            return t.value;
        }
        case '-':
        {
            return - primary();
        }
        case '+':
        {
            return primary();
        }
        case name:
        {
            return vt.get_value(t.name);
        }
        default:
            error("primary expected");
    }
}

double term()
{
    double left = primary();
    Token t = ts.get();

    while (true) {
        switch (t.kind) {
            case '*':
            {
                left *= primary();
                t = ts.get();
                break;
            }
            case '/':
            {
                double d = primary();
                if (d == 0) error("divide by zero");
                left /= d;
                t = ts.get();
                break;
            }
            default:
            {
                ts.putback(t);
                return left;
            }
        }
    }
}

double expression()
{
    double left = term();
    Token t = ts.get();

    while (true) {
        switch (t.kind) {
            case '+':
            {
                left += term();
                t = ts.get();
                break;
            }
            case '-':
            {
                left -= term();
                t = ts.get();
                break;
            }
            default:
            {
                ts.putback(t);
                return left;
            }
        }
    }
}

double declaration()
{
    Token t = ts.get();
    if (t.kind == quit) error("q is a reserved name and cannot be declared as a variable");
    if (t.kind != name) error("name expected in declaration");
    if (vt.is_constant(t.name)) error(t.name, " is a reserved name and cannot be declared as a variable");
    string var_name = t.name;

    Token t2 = ts.get();
    if (t2.kind != '=') error("= missing in declaration of ", var_name);

    double d = expression();
    if (vt.exists(var_name))
        vt.set_value(var_name, d);
    else
        vt.add_var(var_name, d);
    return d;
}

double statement()
{
    Token t = ts.get();
    switch (t.kind) {
        case let:
            return declaration();
        default:
            ts.putback(t);
            return expression();
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

const char prompt = '>';
const char result = '=';

void calculate()
{
    while (cin)
    try {
        cout << prompt << ' ';
        Token t = ts.get();
        while (t.kind == print) t = ts.get();
        if (t.kind == quit) return;
        ts.putback(t);
        cout << result << ' ' << statement() << '\n';
    }
    catch (exception& e) {
        cerr << e.what() << '\n';
        clean_up_mess();
    }
}

int main()
try 
{
    vt.add_const("pi", 3.1415926535);
    vt.add_const("e", 2.7182818284);

    calculate();
    return 0;
}
catch (exception& e) {
    cerr << e.what() << '\n';
    return 1;
}
catch (...) {
    cerr << "exception\n";
    return 2;
}

#pragma GCC diagnostic pop