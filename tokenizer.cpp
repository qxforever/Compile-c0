#include <string>
#include <vector>
#include <map>
#include <iostream>

class Token{
public:
    enum type {
        fn, let, Const, as, While, If, Else, Return, Break, Continue,
        integer, identify,
        string, plus, minus, mul,
        div, assign, equal, notEqual, lower,
        greater, lowerEqual, greaterEqual,
        leftParen, rightParen, leftBrace, rightBrace,
        arrow, comma, colon, semicolon
    };

    static std::map<std::string, type> keyWordTable;

};

std::map<std::string, Token::type> Token::keyWordTable = std::map<std::string, type>{
    {"fn", Token::fn}, {"let", Token::let}, {"const", Token::Const},
    {"as", Token::as}, {"while", Token::While},{"if", Token::If}, 
	{"else", Token::Else}, {"return" ,Token::Return}, {"break", Token::Break},
	{"continue", Token::Continue}
};

class Tokenizer{
private:
    std::string s;
    using pts = std::pair<Token::type, std::string>;
    std::vector<pts> token;

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

            if (isAlpha(s[i])) {
                while (isAlpha(s[j]) || isDigit(s[j])) {
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
                while (isDigit(s[j])) {
                    tmp += s[j];
                    j++;
                }
                token.push_back({Token::integer, tmp});
                i = j - 1;
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
					std::cerr << "Unexpected operator !";
					exit(3);
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
			else {
				switch(s[i]) {
					case '+' :
						token.push_back({Token::plus, "+"});break;
					case '*' :
						token.push_back({Token::mul, "*"});break;
					case '/' :
						token.push_back({Token::div, "/"});break;
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
						std::cerr << "Unexpected character " << s[i] ;
						exit(3); 
				}
			}
        }
    }

	void show() {
		for(const auto i : token) {
			std::cout << i.first << " " << i.second  << '\n';
		}
	}

    Tokenizer(std::string s){
		s += '\n';
        this->s = s;
    }

    
};

int main(){
	// Token test
	using namespace std;
	freopen("test.in", "r", stdin);
	std::string s;
	char c = getchar();
	while (c != EOF) {
		s += c;
		c = getchar();
	}

	cout << "read\n" << s << endl;
	cout << "end read\n" << endl;
	Tokenizer ana = Tokenizer(s);
	ana.work();
	ana.show();
}