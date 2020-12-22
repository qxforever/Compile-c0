#include "analyser.cpp"
#include "common.h"
#include "tokenizer.cpp"
#include "instruction.cpp"
#include "ident.cpp"

int main() {
	std::string s;
	char c;
	while ((c = getchar()) != EOF)  s += c;
	Tokenizer tokenizer(s);
	IdentTable table;
	Analyser analyser(tokenizer, table);
}