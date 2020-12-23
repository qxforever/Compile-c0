#pragma once
#include "ident.cpp"
#include "instruction.cpp"
#include "tokenizer.cpp"
#include <cassert>
class Analyser {
private:
	Token::type curFuncType = Token::End;
	Tokenizer token;
	IdentTable table;
	Instructions& inst;
	std::vector<Instructions> insts;
	std::pair<Token::type, std::string> nextToken();
	void unReadToken();
	void statement();
	void expreStat();
	void declareStat();
	void constDeclareStat();
	void ifStat();
	void whileStat();
	void returnStat();
	void blockStat(int flag = 1);
	Token::type expression();
	Token::type factor();
	Token::type item();
	void program();
	void function();

public:
	void analyse();

	Analyser(Tokenizer tokenizer, IdentTable table, Instructions _inst) : token(tokenizer), table(table), inst(_inst) {
		insts.push_back(Instructions(&table, 0));
		inst = insts.back();
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
		return;
	else if (it.first == Token::End) {
		ASSERT(table.blockDep() == 1, "Unexpected EOF");
		exit(0);
	}
	else {
		expreStat();
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
	inst.pushAddress(it.scope, it.pos);

	auto nxt = nextToken();
	if (nxt.first == Token::assign) {
		auto rightValue = expression();
		ASSERT(rightValue == it.type, "mismatch assignment of variable");
		nxt = nextToken();
	}
	else
		inst.push(uint64_t(0));
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
	inst.pushAddress(it.scope, it.pos);
	auto nxt = nextToken();
	ASSERT(nxt.first == Token::assign, "const variable must be initialized");
	auto rightValue = expression();
	inst.store();
	ASSERT(it.type == rightValue, "mismatch assignment of variable");
	ASSERT(nextToken().first == Token::semicolon, "expected ';' at end of declaration");
}


void Analyser::ifStat() {
	auto rightVal = expression();
	ASSERT(Token::isNum(rightVal), "conditions can't be " + std::to_string(rightVal));

	inst.br(0, 0);
	uint32_t pos = inst.getSize() - 1;
	auto& jumpNext = inst.getLast();
	// pre block
	blockStat();
	//
	inst.br(0);
	auto& jumpEnd = inst.getLast();
	uint32_t _pos = inst.getSize() - 1;
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

	inst.br(0, 0);
	uint32_t pos = inst.getSize() - 1;
	auto& jump = inst.getLast();
	blockStat();
	inst.br(0, 0);
	auto& jumpBegin = inst.getLast();
	jump.key = std::to_string(inst.getSize() - pos - 1);
	jumpBegin.key = std::to_string(_pos - pos - 1);
}

// @Todo
void Analyser::returnStat() {
	// 需要知道当前函数类型 ?
	if (curFuncType == Token::Void) {
		auto nxt = nextToken();
		ASSERT(nxt.first == Token::semicolon, "expected ';' after return");
	}
	else if (curFuncType == Token::integer || curFuncType == Token::Double) {
		inst.arga(0);
		auto value = expression();
		ASSERT(value == curFuncType, "return type mismatch");
		inst.store();
	}
	else ERROR("illeagl return call");
	inst.ret();
}

/**
 * have not read '{'
 **/
void Analyser::blockStat(int flag) {
	ASSERT(nextToken().first == Token::leftBrace, "expected block statement after control stat or function");
	if (flag) table.newBlock();
	statement();
	auto it = nextToken();
	ASSERT(it.first == Token::rightBrace, "expected '}' at end of input");
	table.exitBlock();
}

Token::type Analyser::expression() {
	auto leftValue = item();
	if (leftValue == Token::Void || leftValue == Token::Bool) return leftValue;
	auto nxt = nextToken();
	// bool
	if (Token::isBoolOperator(nxt.first)) {
		auto rightValue = item();
		ASSERT(leftValue == rightValue, "compare in different type");
		// 比较一哈子
		inst.custom("Cmp", leftValue);
		switch (nxt.first) {
		case Token::equal:
			inst.custom("Not");
			break;
		case Token::notEqual:
			break;
		case Token::lower:	// 如果为真, 那么 a < b, 此时栈顶为 -1,
			inst.custom("Set", Token::lower);
			break;
		case Token::lowerEqual:	 // 如果为真, a <= b, 栈顶为 0 或 -1, 即 > 0 为假
			inst.custom("Set", Token::greater);
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

	while (nxt.first == Token::plus || nxt.first == Token::minus) {
		auto rightValue = item();
		ASSERT(leftValue == rightValue, "compare in different type");

		inst.custom(nxt.first);
		nxt = nextToken();
	}

	unReadToken();
	return leftValue;
}

Token::type Analyser::item() {
	auto leftValue = factor();
	auto nxt = nextToken();
	while (nxt.first == Token::mul || nxt.first == Token::div) {
		auto rightValue = item();
		ASSERT(leftValue == rightValue, "compare in different type");
		inst.custom(nxt.first);
		nxt = nextToken();
	}
	unReadToken();
	return leftValue;
}

Token::type Analyser::factor() {
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
			type = it.type;	 //
			inst.stackalloc(type != Token::Void); // 申请空间
			nxt = nextToken();
			ASSERT(nxt.first == Token::leftParen, "expected '(' but find " + nxt.second);
			nxt = nextToken();
			size_t cnt = 0;
			while (nxt.first != Token::rightParen) {
				ASSERT(cnt < it.params.size(), "too many params in " + it.name);
				Token::type type = Token::Void;
				if (nxt.first == Token::identify) {
					auto _it = table.find(nxt.second);
					type = _it.type;
					ASSERT(!_it.isFunc, _it.name + " function can't be param");
					inst.pushAddress(_it.scope, _it.pos);
					inst.load();
				}
				else if (nxt.first == Token::integer)
					inst.push(std::stoull(nxt.second)), type = nxt.first;
				else if (nxt.first == Token::Double)
					inst.push(std::stod(nxt.second)), type = nxt.first;
				else
					ERROR(nxt.second + " can't be param");
				ASSERT(type == it.params[cnt], nxt.second + " type mismatched with function param");
				cnt++;
			}
			ASSERT(cnt == it.params.size(), "too few params in " + it.name);
			inst.call(it.pos);
		}
		// isVariable
		else {
			type = it.type;	 // 不考虑类型转换的情况下
			inst.pushAddress(it.scope, it.pos);
			nxt = nextToken();
			// 赋值表达式
			if (nxt.first == Token::assign) {
				auto rightValue = expression();	 // 右值类型
				ASSERT(it.isConst == false, "assignment of read-only variable '" + it.name + "'");
				ASSERT(it.type == rightValue, "mismatch assignment of variable '" + it.name + "'");
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
		auto rightValue = expression();	 // 右值类型
		ASSERT(rightValue == Token::integer || rightValue == Token::Double, "Only integer and double can be negated");
		inst.custom("Neg", rightValue);
		type = rightValue;
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
	nxt = nextToken();
	if (nxt.first == Token::as) {
		auto next = nextToken();
		ASSERT(next.first == Token::intDecl || next.first == Token::doubleDecl, "expected 'int' or double after 'as'");
		char from = type == Token::integer ? 'I' : 'F';
		char to = next.first == Token::integer ? 'I' : 'F';
		if (from != to) inst.custom(std::string("") + from + "To" + to);
		type = nxt.first;
	}
	else
		unReadToken();
	return type;
}

// program -> decl_stmt* function*
void Analyser::program() {
	auto nxt = nextToken();
	while (nxt.first == Token::let || nxt.first == Token::Const || nxt.first == Token::fn) {
		if (nxt.first == Token::fn) function();
		else nxt.first == Token::let ? declareStat() : constDeclareStat();
		curFuncType = Token::End;
	}

	while (nxt.first == Token::fn) {
		function();
		nxt = nextToken();
	}
	ASSERT(nxt.first == Token::End, "Only declare statement or function appear in global");
	exit(0);
}

// function -> 'fn' IDENT '(' function_param_list? ')' '->' ty block_stmt
// function_param -> 'const'? IDENT ':' ty
// Done
void Analyser::function() {
	auto nxt = nextToken();
	ASSERT(nxt.first == Token::identify, "expected identifier after 'fn'");
	auto _func = table.add(nxt.second, 0, Token::Void, 1);

	insts.push_back(Instructions(&table, 0));  // 函数内部需要单独的 instructions
	inst = insts.back();
	table.newFunc();

	nxt = nextToken();	// should be '('
	ASSERT(nxt.first == Token::leftParen, "unexpected " + nxt.second + " after function identifier");

	nxt = nextToken();
	// , , , 其实是到 ) 结束的. 读取参数
	std::vector<std::tuple<std::string, Token::type, bool>> funParams;
	while (nxt.first == Token::identify || nxt.first == Token::Const) {
		bool isConst = 0;
		if (nxt.first == Token::Const)
			isConst = 1;
		else
			unReadToken();
		auto paramDeclare = [&] (bool isConst) {
			auto nxt = nextToken();
			ASSERT(nxt.first == Token::identify, "lazy to write");
			auto _nxt = nextToken();
			ASSERT(nxt.first == Token::colon, "lazy to write");
			auto __nxt = nextToken();
			ASSERT(nxt.first == Token::integer || nxt.first == Token::Double, "..");
			funParams.push_back(std::make_tuple(nxt.second, __nxt.first, isConst));
		};
		paramDeclare(isConst);
		nxt = nextToken();
		ASSERT(nxt.first == Token::comma || nxt.first == Token::rightParen, "unexpected " + nxt.second + " in function param list");  // , or ')'
		if (nxt.first == Token::rightParen) break;
	}
	ASSERT(nxt.first == Token::rightParen, "unexpected " + nxt.second + " in function param list");	 // ')'

	nxt = nextToken();	// ->
	ASSERT(nxt.first == Token::arrow, nxt.second + " in function " + _func.name);

	nxt = nextToken();	// type
	ASSERT(Token::isFuncType(nxt.first), "function type can not be " + nxt.second);
	auto type = Token::toVarType(nxt.first);
	_func.type = type;
	inst.setReturnType(type);
	if (_func.type != Token::Void) table.add("__ReturnValue.", 0, _func.type, 0);
	for (const auto e : funParams) {
		auto __it = table.add(std::get<0>(e), std::get<2>(e), std::get<1>(e), 0);
		_func._add(std::get<1>(e));
		__it.scope = Token::param;
	}
	inst.setParamCnt(funParams.size());
	uint32_t _tableSize = table.getSize();
	curFuncType = _func.type;
	blockStat(0);
	uint32_t __tableSize = table.getLastSize();

	assert(__tableSize > _tableSize);  // check correctness
	inst.setVarCnt(__tableSize - _tableSize);
}