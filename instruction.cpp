#pragma once
#include "ident.cpp"
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace std {
string to_string(string s) {
	return s;
}

string to_string(const char* s) {
	return string(s);
}

string to_string(Token::type s) {
	if (s == Token::plus) return "Add";
	else if (s == Token::minus)
		return "Sub";
	else if (s == Token::mul)
		return "Mul";
	else if (s == Token::div)
		return "Div";
	else if (s == Token::integer)
		return "I";
	else if (s == Token::Double)
		return "F";
	else if (s == Token::lower)
		return "Lt";
	else if (s == Token::greater)
		return "Gt";
	else if (s == Token::global)
		return "GlobA";
	else if (s == Token::local)
		return "LocA";
	else if (s == Token::param)
		return "ArgA";
	std::cerr << s << '\n';
	assert(0);
	return "";
}

}  // namespace std

struct instruction {
	std::string key, value;

	template <typename T1, typename T2> instruction(T1 u, T2 v) {
		this->key = std::to_string(u);
		this->value = std::to_string(v);
#ifdef debug
		std::cerr << "key = " << key << " value = " << value << '\n';
#endif
	}

	template <typename T> instruction(T u) {
		this->key = std::to_string(u);
#ifdef debug
		std::cerr << "key = " << key << " value = " << value << '\n';
#endif
	}
};

std::ostream& operator<<(std::ostream& out, std::vector<instruction>& a) {
	int sz = 0;
	for (const auto& e : a) {
		out << "    " << sz++ << ": " << e.key;
		if (e.value != "") out << '(' + e.value + ')';
		out << '\n';
	}
	return out;
}

/**
 * 指令集
 * 在内部用 vector<pair<T1, T2>>  存储指令
 * 每次进入一个函数内部调用一下子
 **/
class Instructions {
private:
	std::vector<instruction> instructions;
	template <typename T1, typename T2> void _add(T1 u, T2 v) { instructions.push_back(instruction(u, v)); }
	template <typename T> void _add(T u) { instructions.push_back(instruction(u)); }
	IdentTable* p;
	std::string name;
	uint32_t id = 0, varCnt = 0, paramCnt = 0, noOut;
	Token::type retType;
	static std::ofstream out;
public:
	void push(uint64_t num) { _add("Push", num); }

	void push(double num) { _add("Push", *(uint64_t*)&num); }

	void pushAddress(Token::type u, uint32_t off) { _add(u, off); }

	void pop() { _add("Pop"); }

	void loca(uint32_t off) { _add("LocA", off); }

	void arga(uint32_t off) { _add("ArgA", off); }

	void globa(uint32_t off) { _add("GlobA", off); }

	void load() { _add("Load64"); }

	void store() { _add("Store64"); }

	void ret() { _add("Ret"); res = 1; }

	void call(uint32_t id) { _add("Call", id); }

	void callname(uint32_t id) { _add("CallName", id); }

	void stackalloc(uint32_t size) { _add("StackAlloc", size); }

	void br(int32_t off, int True = -1) {
		std::string s;
		if (True >= 0) s = True ? "True" : "False";
		_add("Br" + s, off);
	}

	template <typename T> void custom(T u) { _add(u); }

	template <typename T> void custom(T u, Token::type v) { _add(std::to_string(u) + std::to_string(v)); }

	uint32_t getSize() { return instructions.size(); }

	instruction& getLast() { return instructions.back(); }

	friend std::ostream& operator<<(std::ostream& out, Instructions& ins) {
		out << "fn [" << ins.id << "] " << ins.varCnt << ' ' << ins.paramCnt << " -> " << (ins.retType != Token::Void) << " {\n" << ins.instructions << "}\n\n";
		return out;
	}

	Instructions() {
		noOut = 1;
		assert(0);
	};
	Instructions(IdentTable* p, int flag = 1) {
		this->p = p;
		noOut = flag;
		instructions.reserve(10010);
	}

#ifndef ONLINE_JUDGE
	~Instructions() {
		if (!noOut) out << *this;
	}
#endif

	void setVarCnt(uint32_t x) { varCnt = x; };
	void setParamCnt(uint32_t x) { paramCnt = x; };
	void setId(uint32_t x) { id = x; }
	void setReturnType(Token::type x) { retType = x; }
	void setNoOut(uint32_t x) { noOut = x; }
	void setName(std::string s) { name = s; }

	bool res = false;
	void get(int& a1, int& a2, int& a3, int& a4, int& a5, std::vector<instruction> &w) { 
		a1 = id, a2 = retType != Token::Void, a3 = paramCnt, a4 = varCnt, a5 = (int)instructions.size();
		w = instructions;
	}
};
#ifndef ONLINE_JUDGE
std::ofstream Instructions::out("debugger.out");
#endif