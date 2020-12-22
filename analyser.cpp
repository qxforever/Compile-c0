#pragma once
#include "ident.cpp"
#include "instruction.cpp"
#include "tokenizer.cpp"
#include <cassert>
class Analyser {
private:
	Tokenizer token;
	IdentTable table;
	Instructions inst;
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
	Token::type expression();
	Token::type operatorExpr(Token::type&);

public:
	void analyse();

	Analyser(Tokenizer tokenizer, IdentTable table) : token(tokenizer), table(table), inst(&table, 0){}
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
		return;
	else if (it.first == Token::End) {
		ASSERT(table.blockDep() == 1, "Unexpected EOF");
		exit(0);
	}
	else {
		expression();
		it = nextToken();
		ASSERT(it.first == Token::semicolon, "expected ';' at end of expression statement");
	}
}

void Analyser::expreStat() {
	auto type = expression();
	if (Token::isNum(type)) inst.pop();
}

// let_decl_stmt -> 'let' IDENT ':' ty ('=' expr)? ';'
void Analyser::declareStat() {
	auto ident = nextToken();
	ASSERT(ident.first == Token::identify, "declaration no identifier");
	ASSERT(nextToken().first == Token::colon, "declaration no colon");
	auto type = nextToken();
	ASSERT(type.first == Token::intDecl || type.first == Token::doubleDecl, "declaration no type name");
	auto it = table.add(ident.second, 0, Token::toVarType(type.first), 0);
	inst.loca(it.pos);

	auto nxt = nextToken();
	if (nxt.first == Token::assign) {
		auto rightValue = expression();
		ASSERT(rightValue == it.type, "mismatch assignment of variable");
		nxt = nextToken();
	}
	else inst.push(uint64_t(0));
	inst.store();
	ASSERT(nxt.first == Token::semicolon, "expected ';' at end of declaration");
}

// const_decl_stmt -> 'const' IDENT ':' ty '=' expr ';'
void Analyser::constDeclareStat() {
	auto ident = nextToken();
	ASSERT(ident.first == Token::identify, "declaration no identifier");
	ASSERT(nextToken().first == Token::colon, "declaration no colon");
	auto type = nextToken();
	ASSERT(type.first == Token::integer || type.first == Token::Double, "declaration no type name");
	auto it = table.add(ident.second, true, Token::toVarType(type.first), false);
	inst.loca(it.pos);
	auto nxt = nextToken();
	ASSERT(nxt.first == Token::assign, "const variable must be initialized");
	auto rightValue = expression();
	inst.store();
	ASSERT(it.type == rightValue, "mismatch assignment of variable");
	ASSERT(nextToken().first == Token::semicolon, "expected ';' at end of declaration");
}

/**
 * @Todo First
 **/
void Analyser::ifStat() {
	auto rightVal = expression();
	ASSERT(Token::isNum(rightVal), "conditions can't be " + std::to_string(rightVal));
	
	inst.br(0, 0); uint32_t pos = inst.getSize() - 1;
	auto& jumpNext = inst.getLast();
	// pre block
	blockStat();
	// 
	inst.br(0);
	auto& jumpEnd = inst.getLast(); uint32_t _pos = inst.getSize() - 1;
	// jump next
	jumpNext.key = std::to_string(inst.getSize() - pos - 1);
	
	auto nxt = nextToken();
	if (nxt.first != Token::Else) {
		unReadToken();
		return;
	}
	else {
		nxt = nextToken();
		if (nxt.first == Token::If)
			ifStat();
		else
			unReadToken(), blockStat();
	}
	// jump to end 
	jumpEnd.key = std::to_string(inst.getSize() - _pos - 1);
}

void Analyser::whileStat() {
	uint32_t _pos = inst.getSize() - 1;
	auto rightVal = expression();
	ASSERT(Token::isNum(rightVal), "conditions can't be " + std::to_string(rightVal));
	
	inst.br(0, 0); uint32_t pos = inst.getSize() - 1;
	auto& jump = inst.getLast(); uint32_t pos = inst.getSize() - 1;
	blockStat();
	inst.br(0, 0);
	auto& jumpBegin = inst.getLast(); uint32_t pos = inst.getSize() - 1;
	jump.key = std::to_string(inst.getSize() - pos - 1);
	jumpBegin.key = std::to_string(_pos - pos - 1);
}


//需要先写函数
void Analyser::returnStat() {
	// auto type = expression();

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

Token::type Analyser::expression() {
	auto nxt = nextToken();
	Token::type type = Token::Void;
	// group_expr -> '(' expr ')'
	if (nxt.first == Token::leftParen) {
		type = expression();
		nxt = nextToken();
		ASSERT(nxt.first == Token::rightParen, "parentheses mismatched");
	}
	// ident_expr -> IDENT
	else if (nxt.first == Token::identify) {
		auto it = table.find(nxt.second);
		// isFunction
		if (it.isFunc) {
			type = it.type;	 // @Todo 函数调用
		}
		// isVariable
		else {
			type = it.type;	 // 不考虑类型转换的情况下
			if (it.isGlobal)
				inst.globa(it.pos);	 // Global
			else
				inst.loca(it.pos);	// local
			nxt = nextToken();
			// 赋值表达式
			if (nxt.first == Token::assign) {
				auto right = expression();	// 右值类型
				ASSERT(it.isConst == false, "assignment of read-only variable \'" + it.name + "\'");
				ASSERT(it.type == right, "mismatch assignment of variable \'" + it.name + '\'');
				inst.store();
				return Token::Void;
			}
			// 现在不知道是啥也
			else {
				inst.load();  // load
				unReadToken();
			}
		}
	}
	// negate_expr -> '-' expr
	else if (nxt.first == Token::minus) {
		auto right = expression();	// 右值类型
		ASSERT(right == Token::integer || right == Token::Double, "Only integer and double can be negated");
		inst.custom("NegI");
		return right;
	}
	// 字面量表达式 uint
	else if (nxt.first == Token::integer) {
		inst.push(std::stoull(nxt.second));
		type = Token::integer;
	}
	// double
	else if (nxt.first == Token::Double) {
		inst.push(std::stod(nxt.second));
		type = Token::Double;
	}
	// Not a expression
	else {
		unReadToken();
		return Token::Void;
	}

	if (type != Token::Void) {
		operatorExpr(type);
	}
	return type;
}

Token::type Analyser::operatorExpr(Token::type& leftVal) {
	auto nxt = nextToken();
	Token::type type;
	if (!Token::isBinary(nxt.first)) {
		unReadToken();
		return Token::Void;
	}
	// as_expr -> expr 'as' ty 类型转换表达式
	if (nxt.first == Token::as) {
		auto next = nextToken();
		ASSERT(next.first == Token::intDecl || next.first == Token::doubleDecl, "expected 'int' or double after 'as'");
		char from = leftVal == Token::integer ? 'I' : 'F';
		char to = next.first == Token::integer ? 'I' : 'F';
		if (from != to) inst.custom(std::string("") + from + "To" + to);
		leftVal = next.first;
		return operatorExpr(leftVal);
	}
	// 运算符表达式
	else {
		// Bool 运算符 '==', '<=' etc
		if (Token::isBoolOperator(nxt.first)) {
			ASSERT(leftVal == Token::integer || leftVal == Token::Double, "value is not comparable");
			auto rightVal = expression();
			ASSERT(rightVal == Token::integer || rightVal == Token::Double, "value is not comparable");
			ASSERT(leftVal == rightVal, "values of different type are not comparable");

			// 比较一哈子
			inst.custom("Cmp", leftVal);
			switch (nxt.first) {
			case Token::equal:
				inst.custom("Not");
				break;
			case Token::notEqual:
				break;
			case Token::lower:	// 如果为真, 那么 a < b, 此时栈顶为 -1,
				inst.custom("Set" + Token::lower);
				break;
			case Token::lowerEqual:	 // 如果为真, a <= b, 栈顶为 0 或 -1, 即 > 0 为假
				inst.custom("Set" + Token::greater);
				inst.custom("Not");
				break;
			case Token::greater:
				inst.custom("Set", Token::greater);
				break;
			case Token::greaterEqual:
				inst.custom("Set", Token::lower);
				inst.custom("Not");
				break;
			default:
				assert(0);
				break;
			}
			// Bool 不能继续参与运算 直接返回了
			return Token::Bool;
		}

		// + - * / @Todo 明天干
		else {

			return leftVal;
		}
	}
	assert(0);
	return type;
}