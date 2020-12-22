#include "analyser.cpp"
#include "common.h"
#include "tokenizer.cpp"

int main(int argc, char **argv){
	std::string s = argv[1];

	Tokenizer tokenizer = Tokenizer(s);
	tokenizer.work();
	
	Analyser analyser = Analyser(tokenizer);
	analyser.work();
}