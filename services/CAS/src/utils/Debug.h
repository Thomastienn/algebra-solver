#pragma once
#include <iostream>
#include <functional>
#include <sstream>
#include "../core/parser/Nodes.h"


using namespace std;
template<typename A, typename B> ostream& operator<<(ostream &os, const pair<A, B> &p) { return os << '(' << p.first << ", " << p.second << ')'; }
template<typename T_container, typename T = typename enable_if<!is_same<T_container, string>::value, typename T_container::value_type>::type> ostream& operator<<(ostream &os, const T_container &v) { os << '{'; string sep; for (const T &x : v) os << sep << x, sep = ", "; return os << '}'; }
inline void dbg_out() { cerr << endl; }
template<typename Head, typename... Tail> void dbg_out(Head H, Tail... T) { cerr << ' ' << H; dbg_out(T...); }
#define dbg(...) cerr << "(" << #__VA_ARGS__ << "):", dbg_out(__VA_ARGS__)
// #define dbg(...)

#define STEP(cls, fn) {#cls "::" #fn, [&]{ return cls::fn(node); }}

namespace Color {
    static const std::string GREEN = "\033[32m";
    static const std::string RED   = "\033[31m";
    static const std::string RESET = "\033[0m";
};

namespace Table {
    struct Step {
        const std::string name;
        bool result;
        std::string nodeStrAfter;
        std::function<bool()> func;

        Step(const std::string n, std::function<bool()> f) : name(n), func(std::move(f)), result(false), nodeStrAfter(""){}
    };
    struct Col { 
        std::string title;
        size_t width; 
    };
};


class Debug {
private:
    static std::string padRight(const std::string &s, size_t width);
public:
    static void executeSteps(std::unique_ptr<ASTNode>& node, bool debug, std::vector<Table::Step>& steps);
};
