// tool.cpp — OOP refactor, encapsulated commands as class methods
// C++17, portable across Linux/macOS/Windows.

#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#if __has_include(<filesystem>)
  #include <filesystem>
  namespace fs = std::filesystem;
#endif

#ifdef _WIN32
  #define OS_WIN 1
  #include <direct.h>
  #define GETCWD _getcwd
#else
  #define OS_WIN 0
  #include <unistd.h>
  #define GETCWD getcwd
#endif

using namespace std;

// -------- Parser for calculator --------
class Parser {
    string s; size_t i{0};
    void ws() { while (i < s.size() && isspace(static_cast<unsigned char>(s[i]))) ++i; }
    bool match(char c) { ws(); if (i < s.size() && s[i] == c) { ++i; return true; } return false; }
    double number() {
        ws();
        size_t j = i;
        if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
        bool any = false;
        while (i < s.size() && (isdigit(static_cast<unsigned char>(s[i])) || s[i] == '.')) { any = true; ++i; }
        if (!any) throw runtime_error("expected number");
        return stod(s.substr(j, i - j));
    }
    double factor() {
        ws();
        if (match('+')) return factor();
        if (match('-')) return -factor();
        if (match('(')) { double v = expr(); if (!match(')')) throw runtime_error("missing ')'"); return v; }
        return number();
    }
    double power() {
        double v = factor(); ws();
        while (match('^')) { double r = factor(); v = pow(v, r); ws(); }
        return v;
    }
    double term() {
        double v = power(); ws();
        while (true) {
            if (match('*')) v *= power();
            else if (match('/')) v /= power();
            else if (match('%')) { long long a = static_cast<long long>(v), b = static_cast<long long>(power()); v = static_cast<double>(a % b); }
            else return v;
            ws();
        }
    }
    double addsub() {
        double v = term(); ws();
        while (true) {
            if (match('+')) v += term();
            else if (match('-')) v -= term();
            else if (i + 1 < s.size() && s[i] == '/' && s[i + 1] == '/') { i += 2; long long a = static_cast<long long>(v), b = static_cast<long long>(term()); v = static_cast<double>(a / b); }
            else return v;
            ws();
        }
    }
public:
    explicit Parser(string expr) : s(move(expr)) {}
    double expr() { return addsub(); }
    bool finished() { ws(); return i == s.size(); }
};

// -------- Console Tool class --------
class Tool {
public:
    void run();

private:
    // helpers
    static string run_capture(const string& cmd);
    static int run_system(const string& cmd);
    static int getInt(const string& prompt);
    static string getStr(const string& prompt);
    static bool safe_eval(const string& e, double& out);

    // commands
    void mdir();
    void read_file();
    void write_file();
    void append_file();
    void sfile();
    void mkpasswd();
    void guess();
    void calc();
    void local_info();
    void osi();
    void ohd();
    void wdh();
    void pchk();

    void print_help();
    const vector<string> CMDS = {
        "help: this help list.",
        "mdir: makes a directory.",
        "read: opens and reads a file.",
        "write: writes to a file.",
        "append: appends to a file.",
        "sfile: search a file.",
        "mkpasswd: makes a random password.",
        "guess: runs a guessing game.",
        "calc: a simple calculator.",
        "local: prints local system information.",
        "osi: displays OSI model info.",
        "ohd: displays ASCII conversions.",
        "wdh: whois/dig/host lookups.",
        "pchk: checks services on a port.",
        "exit: exit the console."
    };
};

// -------- Tool method definitions --------
string Tool::run_capture(const string& cmd) {
    string data;
#if OS_WIN
    FILE* fp = _popen(cmd.c_str(), "r");
#else
    FILE* fp = popen(cmd.c_str(), "r");
#endif
    if (!fp) return data;
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) data += buf;
#if OS_WIN
    _pclose(fp);
#else
    pclose(fp);
#endif
    return data;
}

int Tool::run_system(const string& cmd) {
    int rc = system(cmd.c_str());
    if (rc != 0) cerr << "[ERROR] Command failed: " << cmd << " (rc=" << rc << ")\n";
    return rc;
}

int Tool::getInt(const string& prompt) {
    cout << prompt;
    string s; getline(cin, s);
    try { return stoi(s); } catch (...) { return 0; }
}

string Tool::getStr(const string& prompt) {
    cout << prompt;
    string s; getline(cin, s);
    return s;
}

bool Tool::safe_eval(const string& e, double& out) {
    try {
        Parser p(e);
        out = p.expr();
        if (!p.finished()) throw runtime_error("trailing characters");
        return true;
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return false;
    }
}

void Tool::mdir() {
    string d = getStr("Directory name please: ");
#if __has_include(<filesystem>)
    error_code ec;
    fs::create_directories(d, ec);
    if (ec) cerr << "[ERROR] mkdir: " << ec.message() << "\n";
#else
  #if OS_WIN
    run_system(string("mkdir \"") + d + "\"");
  #else
    run_system(string("mkdir -p \"") + d + "\"");
  #endif
#endif
    char cwd[1024];
    if (GETCWD(cwd, sizeof(cwd))) cout << "OK, have made a directory called: '" << d << "'\nPATH: " << cwd << "\n";
}

void Tool::read_file() {
    string fname = getStr("Filename:\n> ");
    ifstream in(fname);
    if (!in) { cout << "File not found!\n"; return; }
    cout << in.rdbuf();
}

void Tool::write_file() {
    string fname = getStr("Filename:\n> ");
    string text  = getStr("> ");
    ofstream out(fname, ios::app);
    if (!out) { cout << "File not found!\n"; return; }
    out << text;
}

void Tool::append_file() {
    string fname = getStr("Filename please.\n$: ");
    string text  = getStr("To add to the file...$: ");
    ofstream out(fname, ios::app);
    if (!out) { cout << "File not found!\n"; return; }
    out << "\n" << text;
    cout << "Written to file...\n";
}

void Tool::sfile() {
    string fn = getStr("Filename: ");
    string s  = getStr("Search String: ");
#if OS_WIN
    string cmd = string("findstr /I /C:\"") + s + "\" \"" + fn + "\"";
#else
    string cmd = string("grep -i -- \"") + s + "\" \"" + fn + "\"";
#endif
    string out = run_capture(cmd);
    if (out.empty()) cout << "No matches found.\n"; else cout << out;
}

void Tool::mkpasswd() {
    const string letters = "abcdefghijklmnopqrstuvwxyz";
    const string numbs   = "0123456789";
    const string special = "!^*£$";
    random_device rd; mt19937 gen(rd());
    uniform_int_distribution<> dLet(0, static_cast<int>(letters.size()) - 1);
    uniform_int_distribution<> dNum(0, static_cast<int>(numbs.size()) - 1);
    uniform_int_distribution<> dSp(0, static_cast<int>(special.size()) - 1);
    uniform_int_distribution<> pick(0, 2);
    string pass; pass.reserve(8);
    pass += letters[dLet(gen)];
    for (int i = 1; i < 8; i++) {
        int sel = pick(gen);
        if (sel == 0) pass += letters[dLet(gen)];
        else if (sel == 1) pass += numbs[dNum(gen)];
        else pass += special[dSp(gen)];
    }
    cout << "Password is: " << pass << "\nLength: " << pass.size() << "\n";
}

void Tool::guess() {
    cout << "Ok... guessing game, 5 difficulty levels\n";
    array<array<int, 2>, 5> r{{{0,2},{0,4},{0,5},{0,6},{0,9}}};
    int d = getInt("Please select difficulty 1-5 (default 2): ");
    if (d < 1 || d > 5) { cout << "Value Error... defaulting to 2\n"; d = 2; }
    random_device rd; mt19937 gen(rd());
    uniform_int_distribution<> dist(r[d - 1][0], r[d - 1][1]);
    int player = dist(gen), cpu = dist(gen);
    cout << "Your number is: " << player << ".\nComputer's is: " << cpu << ".\n";
}

void Tool::calc() {
    string e = getStr("Please type a sum, e.g. '1+2*3': ");
    double v = 0.0;
    if (safe_eval(e, v)) cout << "= " << setprecision(15) << v << "\n";
    else cout << "Error: evaluation failed.\n";
}

void Tool::local_info() {
    string fname = "local_system_information.txt";
    ofstream out(fname);
    if (!out) { cout << "Could not open output file.\n"; return; }
#if OS_WIN
    vector<string> cmds = { "whoami", "tasklist", "netstat -ano" };
#else
    vector<string> cmds = { "w -i -p", "who -a", "service --status-all", "netstat -tuln" };
#endif
    for (auto& c : cmds) out << run_capture(c);
    cout << "System info written to " << fname << "\n";
}

void Tool::osi() {
    cout <<
"6) Application: DNS, HTTP/HTTPS, Email, FTP\n"
"5) Presentation: Data representation (HTML,DOC,JPEG,MP3)\n"
"4) Session: Inter host communication (TCP,SIP,RTP)\n"
"3) Transport: End-to-End (TCP,UDP,TLS)\n"
"2) Network: IP, ICMP, OSPF\n"
"1) Data Link: Ethernet, 802.11, ARP\n"
"0) Physical: Binary transmission (RJ45, DSL, Wi-Fi)\n";
}

void Tool::ohd() {
#if OS_WIN
    cout << "ASCII manual page not available via 'man' on Windows.\n";
#else
    run_system("man ascii");
#endif
}

void Tool::wdh() {
    string domain = getStr("Domain name please (e.g google.com):\n$: ");
    string save   = getStr("Save to disk? (y/n): ");
    string file   = getStr("Please pick a filename: ");
    bool to_disk = (!save.empty() && (save[0] == 'y' || save[0] == 'Y'));

#if OS_WIN
    string c1 = "whois " + domain;
    string c2 = "nslookup " + domain;
    string c3 = "nslookup " + domain;
#else
    string c1 = "whois " + domain;
    string c2 = "dig "   + domain;
    string c3 = "host "  + domain;
#endif
    if (to_disk) {
        ofstream out(file);
        if (!out) { cout << "Could not open file.\n"; return; }
        out << run_capture(c1);
        out << run_capture(c2);
        out << run_capture(c3);
        cout << "Results saved to " << file << "\n";
    } else {
        cout << run_capture(c1);
        cout << run_capture(c2);
        cout << run_capture(c3);
    }
}

void Tool::pchk() {
    int port = getInt("Port Please: ");
#if OS_WIN
    string out = run_capture("netstat -ano");
#else
    string out = run_capture("netstat -pnltu");
#endif
    string needle = ":" + to_string(port);
    istringstream iss(out);
    string line;
    while (getline(iss, line)) {
        if (line.find(needle) != string::npos) cout << line << "\n";
    }
}

void Tool::print_help() {
    for (auto& s : CMDS) cout << s << "\n";
}

void Tool::run() {
    while (true) {
        cout << "Hi, welcome to the console. Type 'help' for options.\n";
        string cmd = getStr("> ");
        if (cmd == "exit") break;
        else if (cmd == "help")   print_help();
        else if (cmd == "mdir")   mdir();
        else if (cmd == "read")   read_file();
        else if (cmd == "write")  write_file();
        else if (cmd == "append") append_file();
        else if (cmd == "sfile")  sfile();
        else if (cmd == "mkpasswd") mkpasswd();
        else if (cmd == "guess")  guess();
        else if (cmd == "calc")   calc();
        else if (cmd == "local")  local_info();
        else if (cmd == "osi")    osi();
        else if (cmd == "ohd")    ohd();
        else if (cmd == "wdh")    wdh();
        else if (cmd == "pchk")   pchk();
        else cout << "Unknown command.\n";
    }
}
// -------- main --------
int main() {
    Tool tool;
    tool.run();
    return 0;
};
