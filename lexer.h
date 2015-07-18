#ifndef _LEXER_H_
#define _LEXER_H_

#include "common.h"

NAMESPACE_BEGIN

const std::set<std::string> KEYWORDS = {
	"abstract", "arguments", "boolean", "break", "byte",
	"case", "catch", "char", "class", "const", "continue",
	"debugger", "default", "delete", "do", "double", "else",
	"enum", "eval", "export", "extends", "false", "final",
	"finally", "float", "for", "function", "goto", "if",
	"implements", "import", "in", "instanceof", "int",
	"interface", "let", "long", "native", "new", "null",
	"package", "private", "protected", "public", "return",
	"short", "static", "super", "switch", "synchronized",
	"this", "throw", "throws", "transient", "true", "try",
	"typeof", "var", "void", "volatile", "while", "with",
	"yield"
};

struct Token {
	enum Type {
		// Symbol
		COMMA,
		COLON,
		QUESTION,
		SEMICOLON,
		DOT,
		LPAREN,
		RPAREN,
		LBRACKET,
		RBRACKET,
		LBRACE,
		RBRACE,

		// Operator
		OPERATOR,

		// Literal
		IDENTIFIER,
		NUMBER,
		STRING,
		REGULAR,

		// Keyword
		KEYWORD,

		END_OF_LINE,
		END_OF_FILE
	};

	Type type_;
	std::string data_;
	PositionRange range_;

	Token(Type type, const std::string& data, PositionRange range):
		type_(type), data_(data), range_(range)
	{}

	std::string toString()
	{
		std::stringstream ss;
		ss << "Token: [";
		if (data_.length() > 10)
		{
			ss << data_.substr(0, 7) << "...";
		}
		else
		{
			ss << data_;
		}
		ss << "] @ line: " << range_.begin_.line_
			<< ", col: " << range_.begin_.col_;
		return ss.str();
	}
};

static const std::unordered_map<std::string, Token::Type> TOKEN_MAP = {
	// Symbol
	{ ",", Token::Type::COMMA },
	{ ";", Token::Type::SEMICOLON },
	{ ":", Token::Type::COLON },
	{ "?", Token::Type::QUESTION },
	{ ".", Token::Type::DOT },
	{ "(", Token::Type::LPAREN },
	{ ")", Token::Type::RPAREN },
	{ "[", Token::Type::LBRACKET },
	{ "]", Token::Type::RBRACKET },
	{ "{", Token::Type::LBRACE },
	{ "}", Token::Type::RBRACE },

	// Arithmetic
	{ "+", Token::Type::OPERATOR },
	{ "-", Token::Type::OPERATOR },
	{ "*", Token::Type::OPERATOR },
	{ "/", Token::Type::OPERATOR },
	{ "%", Token::Type::OPERATOR },
	{ "++", Token::Type::OPERATOR },
	{ "--", Token::Type::OPERATOR },

	// Bitwise
	{ "&", Token::Type::OPERATOR },
	{ "|", Token::Type::OPERATOR },
	{ "~", Token::Type::OPERATOR },
	{ "^", Token::Type::OPERATOR },
	{ "<<", Token::Type::OPERATOR },
	{ ">>", Token::Type::OPERATOR },

	// Assign
	{ "=", Token::Type::OPERATOR },
	{ "+=", Token::Type::OPERATOR },
	{ "-=", Token::Type::OPERATOR },
	{ "*=", Token::Type::OPERATOR },
	{ "/=", Token::Type::OPERATOR },
	{ "%=", Token::Type::OPERATOR },
	{ "&=", Token::Type::OPERATOR },
	{ "|=", Token::Type::OPERATOR },
	{ "~=", Token::Type::OPERATOR },
	{ "^=", Token::Type::OPERATOR },
	{ "<<=", Token::Type::OPERATOR },
	{ ">>=", Token::Type::OPERATOR },

	// Relative
	{ ">", Token::Type::OPERATOR },
	{ ">=", Token::Type::OPERATOR },
	{ "<", Token::Type::OPERATOR },
	{ "<=", Token::Type::OPERATOR },
	{ "==", Token::Type::OPERATOR },
	{ "!=", Token::Type::OPERATOR },
	{ "===", Token::Type::OPERATOR },
	{ "!==", Token::Type::OPERATOR },

	// Condition
	{ "&&", Token::Type::OPERATOR },
	{ "||", Token::Type::OPERATOR },
	{ "!", Token::Type::OPERATOR },

};

class Lexer {
private:
	static const size_t BUFFER_SIZE = 4096;

private:
	std::list<Token> tokens_;
	std::list<Token>::iterator iter_;

	void process(const std::string& source);

public:
	Lexer(const std::string& source);
	~Lexer();

	void clear();
	void restart();

	Token get();
	Token peek();
	bool end();
};

NAMESPACE_END

#endif