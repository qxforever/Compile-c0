#pragma once
#include "ident.cpp"
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

}  // namespace std

struct instruction {
	std::string key, value;

	template <typename T1, typename T2> instruction(T1 u, T2 v) {
		this->key = std::to_string(u);
		this->value = std::to_string(v);
	}

	template <typename T> instruction(T u) { this->key = std::to_string(u); }
};

/**
 * 指令集
 * 在内部用 vector<pair<T1, T2>>  存储指令
 * 每次进入一个函数内部调用一下子
 **/
class Instructions {
private:
	std::vector<instruction> instructions;
	template <typename T1, typename T2> void _add(T1 u, T2 v = "") { instructions.push_back(instruction(u, v)); }

	template <typename T> void _add(T u) { instructions.push_back(instruction(u)); }
	IdentTable* p;

public:
	void push(uint64_t num) { _add(num); }

	void pop() { _add("Pop"); }

	void loca(uint32_t off) { _add("Loca", off); }

	void arga(uint32_t off) { _add("Arga", off); }

	void globa(uint32_t off) { _add("Globa", off); }

	void load() { _add("Load.64"); }

	void store() { _add("Store.64"); }

	void call() { _add("Ret"); }

	template <typename T> void custom(T u) { _add(u); }
};

int main() {
	Instructions instructions;
	instructions.load();
	instructions.globa(10);
}