#include "tokenizer.cpp"

class Analyser{
private:
	Tokenizer token;
	void groupExpression();
public:
	
	void analyse();

	Analyser(Tokenizer tokenizer) {
		this->t = tokenizer;
	}
};

void Analyser::groupExpression(){
	auto next = token.nextToken();
	if (next.first == Token::leftParen) {

	}
}