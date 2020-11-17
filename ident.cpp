#include <unordered_map>
#include <string>
#include <stack>
#include <vector>
#include <set>
#include "common.cpp"

using pos = int;
#define type std::string

int allocPos();

struct Ident{
	bool isConst;
	bool isFloat;
	int pos;

	Ident(bool isConst, bool isFloat){
		this->isConst = isConst;
		this->isFloat = isFloat;
		this->pos = allocPos();
	}
};

class IdentTable{
private:
	int curBlock = 0;
	std::unordered_map<std::string, int> table;
	std::vector<std::stack<Ident>> ident;
	std::stack<std::set<std::string>> newIdent;
public:
	void newBlock(){
		newIdent.push(std::set<std::string>());
	}

	void exitBlock(){
		auto set = newIdent.top();
		newIdent.pop();
		int newSize = table.size();
		for(auto i : set) {
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

	void add(std::string name, bool isConst, bool isFloat){
		ASSERT(!newIdent.top().count(name), "redefinition " + name);
		newIdent.top().insert(name);
		if (!table.count(name)) {
			int sz = table.size();
			table[name] = sz;
			ident.push_back(std::stack<Ident>());
		}
		ident[table[name]].push(Ident(isConst, isFloat));
	}

	Ident find(std::string name){
		ASSERT(!table.count(name), name + "was not declared in this scope");
		int idx = table[name];
		return ident[idx].top();
	}
};
