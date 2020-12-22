#pragma once
#include "common.cpp"
#include <cstdlib>
#include <iostream>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

struct Ident {
	bool isConst;
	bool isFloat;
	bool isGlobal;
	int pos;

	Ident(bool isConst, bool isFloat, bool isGlobal, int pos) {
		this->isConst = isConst;
		this->isFloat = isFloat;
		this->isGlobal = isGlobal;
		this->pos = pos;
	}

	void print() { std::cout << isConst << ' ' << isFloat << ' ' << isGlobal << ' ' << pos << '\n'; }
};

class IdentTable {
private:
	std::unordered_map<std::string, int> table;	 // 存的变量的编号 idx, 在 ident[idx] 中找到当前标识符的所有位置
	std::vector<std::stack<Ident>> ident;
	std::stack<std::set<std::string>> newIdent;	 // 第 i 个 scope 里新建的变量, 全局变量下标为 0
	int cnt = 0;

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
		ASSERT(newSize == table.size(), "check the correctness of IdentTable::exitBlock()");
	}

	void add(std::string name, bool isConst, bool isFloat) {
		ASSERT(!newIdent.top().count(name), "redefine '" + name + '\'');
		newIdent.top().insert(name);
		if (!table.count(name)) {
			int sz = table.size();
			table[name] = sz;
			std::cout << name << '\n';
			ident.push_back(std::stack<Ident>());
		}
		ident[table[name]].push(Ident(isConst, isFloat, newIdent.size() == 1, cnt++));
	}

	Ident find(std::string name) {
		ASSERT(table.count(name) > 0, "\'" + name + "\' was not declared in this scope");
		int idx = table[name];
		return ident[idx].top();
	}

	IdentTable() { newBlock(); }
};

int main() {
	IdentTable table;
	table.add("x", 0, 0);
	table.add("x", 1, 1);
}