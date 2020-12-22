#pragma once
#include "ident.cpp"
#include <string>
#include <utility>
#include <vector>
#include <iostream>

namespace std {
string to_string(string s) {
	return s;
}

string to_string(const char* s) {
	return string(s);
}

}  // namespace std

struct instruction {
	std::string key, value;

	template <typename T1, typename T2> instruction(T1 u, T2 v) {
		this->key = std::to_string(u);
		this->value = std::to_string(v);
	}

	template <typename T> instruction(T u) { this->key = std::to_string(u); }
};

std::ostream& operator<<(std::ostream& out, std::vector<instruction>& a) {
	int sz = 0;
	for (const auto& e : a) {
		out << "    " << sz << " : " << e.key;
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
	uint32_t id = 0, args = 0, retType = 0;

public:
	void push(uint64_t num) { _add("Push", num); }

	void push(double num) { _add("Push", *(uint64_t*)&num); }

	void pop() { _add("Pop"); }

	void loca(uint32_t off) { _add("LocA", off); }

	void arga(uint32_t off) { _add("ArgA", off); }

	void globa(uint32_t off) { _add("GlobA", off); }

	void load() { _add("Load64"); }

	void store() { _add("Store64"); }

	void ret() { _add("Ret"); }

	void call(uint32_t id) { _add("Call", id); }

	void callname(uint32_t id) { _add("CallName", id); }

	void stackalloc(uint32_t size) { _add("StackAlloc", size); }

	void br(int32_t off, int True = -1) {
		std::string s;
		if (True >= 0) s = True ? "True" : "False";
		_add("Br" + s, off);
	}

	template <typename T> void custom(T u) { _add(u); }

	friend std::ostream& operator<<(std::ostream& out, Instructions& ins) {
		out << "fn [" << ins.id << "] " << ins.p->getSize() << ' ' << ins.args << " -> " << (ins.retType > 0) << " {\n" << ins.instructions << "}\n";
		return out;
	}

	Instructions(IdentTable* p) { this->p = p; }
	~Instructions() { std::cout << *this; }
};

// int main() {
// 	IdentTable tb;
// 	tb.add("x", 1, 1);
// 	Instructions w = Instructions(&tb);
// 	w.load();
// 	w.loca(14);
// 	w.push(2.3);
// }