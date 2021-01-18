#include "analyser.cpp"
#include "tokenizer.cpp"
#include "instruction.cpp"
#include "ident.cpp"
#include <iostream>
#include <ctime>

#ifdef ONLINE_JUDGE
#define _byteswap_uint64(x) (x)
#endif

uint32_t little2big(uint32_t le) {
#ifdef ONLINE_JUDGE
	return le;
#endif
    return ((le & 0xff) << 24) 
            | ((le & 0xff00) << 8) 
            | ((le & 0xff0000) >> 8)
            | ((le >> 24) & 0xff);
}

int main(int argc, char** argv) {
	global.push_back(Global(1, 6, "_start"));
#ifndef ONLINE_JUDGE
	freopen("test.in", "r", stdin);
#endif
	std::string s;
	char c;
	while ((c = getchar()) != EOF)  s += c;
#ifdef ONLINE_JUDGE
	std::cerr << s << '\n';
#endif
	Tokenizer tokenizer(s);
#ifdef debug
	freopen("test.log", "w", stderr);
	tokenizer.show();
#endif
// init std Function
	IdentTable table;
	table.add("getint", 1, Token::integer, 1);
	table.add("getdouble", 1, Token::Double, 1);
	table.add("getchar", 1, Token::integer, 1);
	table.add("putint", 1, Token::Void, 1);
	auto &fun1 = table.find("putint");
	fun1.params.push_back(Token::integer);
	table.add("putdouble", 1, Token::Void, 1);
	auto &fun2 = table.find("putdouble");
	fun2.params.push_back(Token::Double);
	table.add("putchar", 1, Token::Void, 1);
	auto &fun3 = table.find("putchar");
	fun3.params.push_back(Token::integer);
	table.add("putstr", 1, Token::Void, 1);
	auto &fun4 = table.find("putstr");
	fun4.params.push_back(Token::integer);
	table.add("putln", 1, Token::Void, 1);

// init string 

	int sz = 0;
	std::map<std::string, int> map;
	auto nxt = tokenizer.nextToken();
	while (nxt.first != Token::End) {
		if (nxt.first == Token::string) {
			if (map.count(nxt.second) == 0) {
				int pos = table._add_string();
				map[nxt.second] = pos;
				global.push_back(Global(1, nxt.second.size(), nxt.second));
			}
			tokenizer.preToken().first = Token::integer;
			tokenizer.preToken().second = std::to_string(map[nxt.second]);
			nxt = tokenizer.preToken();
		}
		sz++;
		nxt = tokenizer.nextToken();
	}
	while (sz >= 0) tokenizer.unRead(), sz--;

#ifdef debug
	tokenizer.show();
#endif

	Analyser analyser(tokenizer, table);
	analyser.analyse();
#ifndef ONLINE_JUDGE
	std::ofstream out("test.out", std::ios::out | std::ios::binary);
#else
	std::ofstream out(argv[1], std::ios::out | std::ios::binary);
#endif
	std::map<std::string, int> Table;
	std::ifstream fin("instruction.io");
	while (!fin.eof() && clock() < CLOCKS_PER_SEC * 5) {
		std::string s, t;
		fin >> s >> t;
		int val;
		sscanf(s.c_str(), "%x", &val);
		Table[t] = val;
	}

	auto Write = [&](const void *p, int size) {
		out.write((char*)p, size);
	};

	uint32_t magic = little2big(0x72303b3e), version = little2big(0x00000001);
	Write(&magic, 4);
	Write(&version, 4);

	sz = little2big((int32_t)global.size());
	Write(&sz, 4);
	for (auto e : global) {
		int _sz = little2big(e.size);
		// std::cout << e.size << '\n';
		Write(&e.isConst, 1);
		Write(&_sz, 4);
		if (e.isConst) {
			Write(e.val.c_str(), e.size);
		}
		else {
			int64_t a = 0;
			Write(&a, 8);
		}
	}
	auto &vec = analyser.getInst();
	sz = little2big((uint32_t)vec.size());
	Write(&sz, 4);
	for (auto ins : vec) {
		int a[5];
		std::vector<instruction> w;
		ins.get(a[0], a[1], a[2], a[3], a[4], w);
		for (int i = 0; i < 5; i++) a[i] = little2big(a[i]), Write(&a[i], 4);
		for (auto p : w) {
			for (auto &c : p.key) c = tolower(c);
			std::cout << p.key << '\n';
			assert(Table.count(p.key) > 0);
			int8_t id = Table[p.key];
			Write(&id, 1);
			if (p.value != "") {
				if (p.key == "push") {
					uint64_t val = std::stoull(p.value);
					val = _byteswap_uint64(val);
					Write(&val, 8);
				}
				else {
					int32_t val = std::stoi(p.value);
					val = little2big(val);
					Write(&val, 4);
				}
			}
		}
	}
	return 0;
}
