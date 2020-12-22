#pragma once
#include "common.cpp"
#include "tokenizer.cpp"
#include <cstdlib>
#include <iostream>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

struct Ident {
	bool isConst, isGlobal, isFunc;
	Token::type type;
	int pos;
	std::vector<int> params;
	std::string name;

	Ident(bool isConst, Token::type type, bool isGlobal, bool isFunc, int pos, std::string name) {
		this->isConst = isConst;
		this->type = type;
		this->isGlobal = isGlobal;
		this->isFunc = isFunc;
		this->pos = pos;
		this->name = name;
	}

	void add(int x) { params.push_back(x); }

	void print() { std::cout << isConst << ' ' << type << ' ' << isGlobal << ' ' << pos << '\n'; }
};

class IdentTable {
private:
	std::unordered_map<std::string, int> table;	 // 存的变量的编号 idx, 在 ident[idx] 中找到当前标识符的所有位置
	std::vector<std::stack<Ident>> ident;
	std::stack<std::set<std::string>> newIdent;	 // 第 i 个 scope 里新建的变量, 全局变量下标为 0
	uint32_t cnt = 0;

public:
	void newBlock() { newIdent.push(std::set<std::string>()); }

	void exitBlock() {
		auto set = newIdent.top();
		newIdent.pop();
		int newSize = table.size();
		for (auto i : set) {
			int idx = table[i];
			ident[idx].pop();
			if (ident[idx].empty()) {
				newSize = std::min(newSize, idx + 1);
				table.erase(i);
			}
		}
		ident.resize(table.size());
		ASSERT(newSize == (int)table.size(), "check the correctness of IdentTable::exitBlock()");
	}

	Ident& add(std::string name, bool isConst, Token::type type, bool isFunc) {
		ASSERT(!newIdent.top().count(name), "redefine '" + name + '\'');
		newIdent.top().insert(name);
		if (!table.count(name)) {
			int sz = table.size();
			table[name] = sz;
			std::cout << name << '\n';
			ident.push_back(std::stack<Ident>());
		}
		auto ret = Ident(isConst, type, newIdent.size() == 1, isFunc, cnt++, name);
		ident[table[name]].push(ret);
		return ident[table[name]].top();
	}

	Ident& find(std::string name) {
		ASSERT(table.count(name) > 0, "\'" + name + "\' was not declared in this scope");
		int idx = table[name];
		return ident[idx].top();
	}

	uint32_t getSize() { return cnt; }

	IdentTable() { newBlock(); }
};

// int main() {
// 	IdentTable table;
// 	table.add("x", 0, 0);
// 	table.add("x", 1, 1);
// }