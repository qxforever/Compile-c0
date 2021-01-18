#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cassert>
#include "common.cpp"

class Token{
public:
	enum type {
		fn, let, Const, While, If, Else, Return, Break, Continue,
		voidDecl, intDecl, doubleDecl, // 'voidDecl' means $\text{void}$ in input.  
		Void, integer, Double, Bool, identify, comment, 
		string, plus, minus, mul,
		div, equal, notEqual, lower,
		greater, lowerEqual, greaterEqual, as, assign,
		global, local, param,
		leftParen, // '('
		rightParen, // ')'
		leftBrace, // '{'
		rightBrace,// '}'
		arrow,
		comma, // ','
		colon, // ':'
		semicolon, // ';'
		End
	};

	static std::map<std::string, type> keyWordTable;
	
	static type toVarType(type sth) {
		assert(isFuncType(sth));
		if (sth == voidDecl) return Void;
		else return sth == intDecl ? integer : Double;
	}

	static bool isBinary(type sth) {
		return sth >= plus && sth <= as;
	}

	static bool isBoolOperator(type sth) {
		return sth >= equal && sth <= greaterEqual;
	}

	static bool isNum(type sth) {
		return sth >= integer && sth <= Bool;
	}

	static bool isFuncType(type sth) {
		return sth >= voidDecl && sth <= doubleDecl;
	}

	static bool isNumOperator(type sth) {
		return sth >= plus && sth <= div;
	}
};

std::map<std::string, Token::type> Token::keyWordTable = std::map<std::string, type>{
	{"fn", Token::fn}, {"let", Token::let}, {"const", Token::Const},
	{"as", Token::as}, {"while", Token::While},{"if", Token::If}, 
	{"else", Token::Else}, {"return" ,Token::Return}, {"break", Token::Break},
	{"continue", Token::Continue}, {"void", Token::voidDecl}, {"int", Token::intDecl}, {"double", Token::doubleDecl}
};

class Tokenizer{
private:
	std::string s;
	using pts = std::pair<Token::type, std::string>;
	std::vector<pts> token;
	int cur = 0;

	bool isSpace(char c){
		return isspace(c);
	}

	bool isAlpha(char c){
		return isalpha(c);
	}

	bool isDigit(char c){
		return isdigit(c);
	}

public:
	void work() {
		int len = s.length();
		for (int i = 0; i < len; i++) {
			if (isspace(s[i])) continue;
			std::string tmp = "";
			int j = i;

			if (isAlpha(s[i]) || s[i] == '_') {
				while (isAlpha(s[j]) || isDigit(s[j]) || s[j] == '_') {
					tmp += s[j];
					j++;
				}

				if (Token::keyWordTable.count(tmp)) {
					token.push_back({Token::keyWordTable[tmp], tmp});
				}
				else {
					token.push_back({Token::identify, tmp});
				}
				i = j - 1;
			}
			else if (isDigit(s[i])) {
				// Integer
				while (isDigit(s[j])) {
					tmp += s[j++];
				}
				// Double 
				if (s[j] == '.') {
					tmp += s[j++];
					int cnt = 0;
					while (isdigit(s[j])) tmp += s[j++], cnt++;
					ASSERT(cnt > 0, "at least 1 digit after '.'");
					if (s[j] == 'e' || s[j] == 'E') {
						tmp += s[j++];
						if (s[j] == '+' || s[j] == '-') tmp += s[j++];
						cnt = 0;
						while (isdigit(s[j])) tmp += s[j++], cnt++;
						ASSERT(cnt > 0, "at least 1 digit after 'e'");
					}
					token.push_back({Token::Double, tmp});
				}
				else token.push_back({Token::integer, tmp});
				i = j - 1;
			}
			else if (s[i] == '\"') {
				int j = i;
				std::vector<int> tmp;
				for (; j < len; j++) {
					if (s[j] == '\\') tmp.push_back(-1);
					else tmp.push_back(s[j]);
					if (s[j] == '\"' && tmp.size() > 1 && tmp[tmp.size() - 2] != -1) break;
					if (tmp.size() >= 2 && tmp[tmp.size() - 2] == -1) {
						int c = tmp.back();
						if (!(c == 'n' || c == 't' || c == 'r' || c == -1 || c == '\'' || c == '\"')) {
							std::cerr << "Invalid escape sequence \n";
							exit(3);
						}
						tmp.resize(tmp.size() - 2);
						switch (c){
							case 'n':
								tmp.push_back('\n');break;
							case 't':
								tmp.push_back('\t');break;
							case 'r':
								tmp.push_back('\r');break;
							case '\'':
								tmp.push_back('\'');break;
							case '\"':
								tmp.push_back('\"');break;
							case -1:
								tmp.push_back('\\');break;
							default:
								ERROR("Invalid escape sequence");
							break;
						}
					}
				}
				if (j == len) {
					ERROR("\" does not match");
				}
				std::string ss;
				for (int i = 1; i < (int)tmp.size() - 1; i++) ss += (char)tmp[i];
				token.push_back({Token::string, ss});
				i = j;
			}
			else if (s[i] == '\'') {
				int j = i;
				std::vector<int> tmp;
				for (; j < len; j++) {
					if (s[j] == '\\') tmp.push_back(-1);
					else tmp.push_back(s[j]);
					if (s[j] == '\'' && tmp.size() > 1 && tmp[tmp.size() - 2] != -1) break;
					if (tmp.size() >= 2 && tmp[tmp.size() - 2] == -1) {
						int c = tmp.back();
						if (!(c == 'n' || c == 't' || c == 'r' || c == -1 || c == '\'' || c == '\"')) {
							std::cerr << "Invalid escape sequence \n";
							exit(3);
						}
						tmp.resize(tmp.size() - 2);
						switch (c){
							case 'n':
								tmp.push_back('\n');break;
							case 't':
								tmp.push_back('\t');break;
							case 'r':
								tmp.push_back('\r');break;
							case '\'':
								tmp.push_back('\'');break;
							case '\"':
								tmp.push_back('\"');break;
							case -1:
								tmp.push_back('\\');break;
							default:
								ERROR("Invalid escape sequence");
							break;
						}
					}
				}
				if (j == len) {
					ERROR("\" does not match");
				}
				std::string ss;
				for (int i = 1; i < (int)tmp.size() - 1; i++) ss += (char)tmp[i];
				ASSERT(ss.size() == 1, "Invalid char varible");
				token.push_back({Token::integer, std::to_string(int(ss[0]))});
				i = j;
			}
			else if (s[i] == '=') {
				if (s[i + 1] == '=') {
					token.push_back({Token::equal, "=="});
					i++;
				}
				else token.push_back({Token::assign, "="});
			}
			else if (s[i] == '!') {
				if(s[i + 1] == '=') token.push_back({Token::notEqual, "!="}), i++;
				else {
					ERROR("Unexpected operator !\n");
				}
			}
			else if (s[i] == '<') {
				if (s[i + 1] == '=') {
					token.push_back({Token::lowerEqual, "<="});
					i++;
				}
				else token.push_back({Token::lower, "<"});
			}
			else if (s[i] == '>') {
				if (s[i + 1] == '=') {
					token.push_back({Token::greaterEqual, ">="});
					i++;
				}
				else token.push_back({Token::greater, ">"});
			}
			else if (s[i] == '-') {
				if (s[i + 1] == '>') {
					token.push_back({Token::arrow, "->"});
					i++;
				}
				else token.push_back({Token::minus, "-"});
			}
			else if (s[i] == '/') {
				if (s[i + 1] == '/') {
					int j = i + 2;
					while (j < len && s[j] != '\n') j++;
					i = j;
				}
				else token.push_back({Token::div, "/"});
			}
			else {
				switch(s[i]) {
					case '+' :
						token.push_back({Token::plus, "+"});break;
					case '*' :
						token.push_back({Token::mul, "*"});break;
					// case '/' :
					// 	token.push_back({Token::div, "/"});break;
					case '(' :
						token.push_back({Token::leftParen, "("});break;
					case ')' :
						token.push_back({Token::rightParen, ")"});break;
					case '{' :
						token.push_back({Token::leftBrace, "{"});break;
					case '}' :
						token.push_back({Token::rightBrace, "}"});break;
					case ',' :
						token.push_back({Token::comma, ","});break;
					case ':':
						token.push_back({Token::colon, ":"});break;
					case ';':
						token.push_back({Token::semicolon, ";"});break;
					default:
						std::cerr << "Unexpected character " << s[i] << '\n';
						exit(3); 
				}
			}
		}
		token.push_back({Token::End, "End"});
	}

	void show() {
		for(const auto &i : token) {
			std::cerr << i.first << " " << i.second  << '\n';
		}
	}

	pts nextToken(){
		ASSERT(cur < (int)token.size(), "should terminate after end of file");
		return token[cur++];
	}

	pts& preToken() {
		return token[cur - 1];
	}

	void unRead(){
		cur--;
		ASSERT(cur >= 0, "current token pos will be -1");
	}

	Tokenizer(std::string s){
		s += '\n';
		this->s = s;
		work();
	}

	Tokenizer(){}
};