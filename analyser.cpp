#include "ident.cpp"
#include "tokenizer.cpp"
class Analyser {
private:
	Tokenizer token;
	IdentTable table;
	std::pair<Token::type, std::string> nextToken();
	void unReadToken();
	void statement();
	void expreStat();
	void declareStat();
	void constDeclareStat();
	void ifStat();
	void whileStat();
	void returnStat();
	void blockStat();
	void expression();

public:
	void analyse();

	void set(IdentTable table) {
		this->table = table;
	}

	Analyser(Tokenizer tokenizer) {
		this->token = tokenizer;
	}
};

std::pair<Token::type, std::string> Analyser::nextToken() {
	return token.nextToken();
}

void Analyser::unReadToken() {
	return token.unRead();
}

void Analyser::statement() {
	auto it = nextToken();
	if (it.first == Token::let)
		declareStat();
	else if (it.first == Token::Const)
		constDeclareStat();
	else if (it.first == Token::If)
		ifStat();
	else if (it.first == Token::While)
		whileStat();
	else if (it.first == Token::Return)
		returnStat();
	else if (it.first == Token::leftBrace)
		unReadToken(), blockStat();
	else if (it.first == Token::semicolon)
		return ;
	else {
		expression();
		it = nextToken();
		ASSERT(it.first == Token::semicolon, "expected ';' at end of expression statement"); 
	}
}

void Analyser::expreStat() {
	// @TODO
}

void Analyser::declareStat() {
	auto ident = nextToken();
	ASSERT(ident.first == Token::identify, "declaration no identifier");
	ASSERT(nextToken().first == Token::colon, "declaration no colon");
	auto type = nextToken();
	ASSERT(type.first == Token::integer || type.first == Token::Double, "declaration no type name");
	table.add(ident.second, 0, type.first == Token::Double);
	
	auto nxt = nextToken();
	if (nxt.first == Token::equal) {
		expression();
		// TODOexpr
		nxt = nextToken();
	}
	ASSERT(nxt.first == Token::semicolon, "expected ';' at end of declaration");
}

void Analyser::constDeclareStat() {
	auto ident = nextToken();
	ASSERT(ident.first == Token::identify, "declaration no identifier");
	ASSERT(nextToken().first == Token::colon, "declaration no colon");
	auto type = nextToken();
	ASSERT(type.first == Token::integer || type.first == Token::Double, "declaration no type name");
	table.add(ident.second, 1, type.first == Token::Double);
	
	auto nxt = nextToken();
	ASSERT(nxt.first == Token::equal, "const variable must be initialized"); 
	expression();
	// TODOexpr
	nxt = nextToken();
	ASSERT(nxt.first == Token::semicolon, "expected ';' at end of declaration");
}

void Analyser::ifStat() {
	expression();
	// TODOexpr
	blockStat();
	auto nxt = nextToken();
	if (next.first != Token::Else) {
		unReadToken();
		return ;
	}
	else {
		nxt = nextToken();
		if (nxt.first == Token::If)
			ifStat();
		else unReadToken(), blockStat();
	}
}

void Analyser::whileStat() {
	expression();
	// TODOexpr
	blockStat();
}

/**
 * have not read '{'
 **/
void Analyser::blockStat() {
	ASSERT(nextToken().first == Token::leftBrace, "expected block statement after control stat or function");
	table.newBlock();
	statement();
	auto it = nextToken();
	ASSERT(it.first == Token::rightBrace, "expected '}' at end of input");
	table.exitBlock();
}
