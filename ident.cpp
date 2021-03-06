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
#include <deque>

struct Global {
	int8_t isConst;
	int32_t size;
	std::string val;

	Global() {}
	Global(bool Const, int64_t sz, std::string v) {
		isConst = Const;
		size = sz;
		val = v;
	}
};

std::vector<Global> global;

struct Ident {
	bool isConst, isGlobal, isFunc, isStd;
	Token::type type, scope;
	int pos;
	std::deque<Token::type> params;
	std::string name;

	Ident(bool isConst, Token::type type, bool isGlobal, bool isFunc, int pos, std::string name) {
		this->isConst = isConst;
		this->type = type;
		this->scope = isGlobal ? Token::global : Token::local;
		this->isFunc = isFunc;
		this->pos = pos;
		this->name = name;
		this->isGlobal = isGlobal;
		this->isStd = false;
		if (isGlobal) {
			global.push_back(Global(isFunc ? 1 : 0, isFunc ? name.size() : 8, isFunc ? name : ""));
		}
	}

	void _add(Token::type x) { params.push_back(x); }

	void print() { std::cout << isConst << ' ' << type << ' ' << isGlobal << ' ' << pos << '\n'; }
};

class IdentTable {
private:
	std::unordered_map<std::string, int> table;	 // 存的变量的编号 idx, 在 ident[idx] 中找到当前标识符的所有位置
	std::deque<std::deque<Ident>> ident; // Todo
	std::stack<std::set<std::string>> newIdent;	 // 第 i 个 scope 里新建的变量, 全局变量下标为 0
	uint32_t cnt = 0, lastCnt = 0, globalCnt = 0, paramCnt = 0, funCnt = 1;
	bool funcBlock;	 // 是否刚刚进入函数

public:
	void newBlock() { newIdent.push(std::set<std::string>()); }
	void newFunc(bool ret = false) { newBlock(), cnt = lastCnt = 0, paramCnt = ret; }
	void exitBlock() {
		lastCnt = std::max(lastCnt, cnt);
		auto set = newIdent.top();
		newIdent.pop();
		int newSize = table.size();
		for (auto i : set) {
			int idx = table[i];
			if (ident[idx].back().scope == Token::local) cnt--;
			ident[idx].pop_back();
			if (ident[idx].empty()) {
				newSize = std::min(newSize, idx);
				table.erase(i);
			}
		}
		// ident.resize(table.size());
		while ((int)ident.size() > newSize) ident.pop_back();
		ASSERT(newSize == (int)table.size(), "check the correctness of IdentTable::exitBlock()");
	}

	uint32_t blockDep() { return newIdent.size(); }

	// 参数变量加入的时候 手动控制一下吧 没设计好
	Ident& add(std::string name, bool isConst, Token::type type, bool isFunc, bool isParam = false) {
		ASSERT(!newIdent.top().count(name), "redefine '" + name + '\'');
		newIdent.top().insert(name);
		if (!table.count(name)) {
			int sz = table.size();
			table[name] = sz;
			ident.push_back(std::deque<Ident>());
		}
		auto& it = isParam ? paramCnt : (isFunc ? funCnt : ((newIdent.size() == 1) ? globalCnt : cnt));
		it++;
		int val = 0;
		if (isParam) {
			val = paramCnt - 1;
		}
		else if (isFunc) {
			if (funCnt <= 9) val = funCnt - 1;
			else val = funCnt - 9; 
		}
		else if (newIdent.size() == 1) val = globalCnt + funCnt - 1;
		else val = cnt - 1;
		auto ret = Ident(isConst, type, newIdent.size() == 1, isFunc, val, name);
		ident[table[name]].push_back(ret);
		return ident[table[name]].back();
	}

	int _add_string() {
		globalCnt++;
		return globalCnt + funCnt - 1;
	}

	Ident& find(std::string name) {
		ASSERT(table.count(name) > 0, "\'" + name + "\' was not declared in this scope");
		int idx = table[name];
		return ident[idx].back();
	}

	uint32_t getSize() { return cnt; }
	uint32_t getFunId() { return funCnt + globalCnt - 1; }
	uint32_t getLastSize() { return lastCnt; }
	uint32_t getGlobalCnt() { return globalCnt; }
	IdentTable() { newBlock();}
};