#include "analyser.cpp"
#include "tokenizer.cpp"
#include "instruction.cpp"
#include "ident.cpp"

int main() {
	freopen("test.in", "r", stdin);
	freopen("test.log", "w", stderr);
	freopen("test.out", "w", stdout);
	std::string s;
	char c;
	while ((c = getchar()) != EOF)  s += c;
	Tokenizer tokenizer(s);
	tokenizer.show();
	IdentTable table;
	Analyser analyser(tokenizer, table);
	analyser.analyse();
}