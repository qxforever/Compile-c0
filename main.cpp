#include "analyser.cpp"
#include "tokenizer.cpp"
#include "instruction.cpp"
#include "ident.cpp"

int main() {
	global.push_back(Global(1, 6, "_start"));
	freopen("test.in", "r", stdin);
	freopen("test.log", "w", stderr);
	std::string s;
	char c;
	while ((c = getchar()) != EOF)  s += c;
	Tokenizer tokenizer(s);
	tokenizer.show();
	IdentTable table;
	Analyser analyser(tokenizer, table);
	analyser.analyse();
#ifndef ONLINE_JUDGE
	std::ofstream out("test.out");
#else
	auto &out = std::cout;
#endif
	std::map<std::string, int> Table;
	std::ifstream fin("instruction.in");
	while (!fin.eof()) {
		std::string s, t;
		fin >> s >> t;
		int val;
		sscanf(s.c_str(), "%x", &val);
		Table[t] = val;
	}

	auto Write = [&](const void *p, int size) {
		out.write((char*)p, size);
	};
	
	uint32_t magic = 0x72303b3e, version = 0x00000001;
	Write(&magic, 4);
	Write(&version, 4);
	int sz = global.size();
	Write(&sz, 4);

	for (auto e : global) {
		Write(&e.isConst, 1);
		Write(&e.size, 4);
		if (e.isConst) {
			Write(e.val.c_str(), e.size);
		}
		else {
			int64_t a = 0;
			Write(&a, 8);
		}
	}
	auto &vec = analyser.getInst();
	sz = vec.size();
	Write(&sz, 4);
	for (auto ins : vec) {
		int a[5];
		std::vector<instruction> w;
		ins.get(a[0], a[1], a[2], a[3], a[4], w);
		for (int i = 0; i < 5; i++) Write(&a[i], 4);
		for (auto p : w) {
			for (auto &c : p.key) c = tolower(c);
			assert(Table.count(p.key) > 0);
			int8_t id = Table[p.key];
			Write(&id, 1);
			if (p.value != "") {
				if (p.value == "push") {
					uint64_t val = std::stoull(p.value);
					Write(&val, 8);
				}
				else {
					int32_t val = std::stoi(p.value);
					Write(&val, 4);
				}
			}
		}
	}
}