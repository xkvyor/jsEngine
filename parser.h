#ifndef _PARSER_H_
#define _PARSER_H_

#include "value.h"
#include "ast.h"

NAMESPACE_BEGIN

static const std::unordered_map<std::string, int> PRIORITY = {
	// Symbol
	{ "?", 1 },

	// Arithmetic
	{ "+", 10 },
	{ "-", 10 },
	{ "*", 11 },
	{ "/", 11 },
	{ "%", 11 },
	{ "++", 15 },
	{ "--", 15 },

	// Bitwise
	{ "&", 6 },
	{ "|", 4 },
	{ "~", 15 },
	{ "^", 5 },
	{ "<<", 9 },
	{ ">>", 9 },

	// Assign
	{ "=", 0 },
	{ "+=", 0 },
	{ "-=", 0 },
	{ "*=", 0 },
	{ "/=", 0 },
	{ "%=", 0 },
	{ "&=", 0 },
	{ "|=", 0 },
	{ "~=", 0 },
	{ "^=", 0 },
	{ "<<=", 0 },
	{ ">>=", 0 },

	// Relative
	{ ">", 8 },
	{ ">=", 8 },
	{ "<", 8 },
	{ "<=", 8 },
	{ "instanceof", 8 },
	{ "in", 8 },
	{ "==", 7 },
	{ "!=", 7 },
	{ "===", 7 },
	{ "!==", 7 },
	{ "&&", 3 },
	{ "||", 2 },
	{ "!", 15 },

};

class ParseError: public std::exception
{
private:
	std::string msg_;

public:
	ParseError(std::string msg): msg_(msg)
	{}
	virtual const char* what() const throw()
	{
		return msg_.c_str();
	}
};

class Parser {
private:
	Program* root_;
	Lexer* lex_;

	Token match(std::string s);
	Token match(Token::Type type);
	bool expect(std::string s);
	bool expect(Token::Type type);
	bool expectOperator(int pri);

	std::list<AST*>* topStatements(Scope* ps);
	AST* topStatement(Scope* ps);
	std::list<AST*>* statements(Scope* ps);
	AST* statement(Scope* ps);
	void opteol();

	Program* program();
	AST* ifStatement(Scope* s);
	AST* switchStatement(Scope* s);
	AST* doStatement(Scope* s);
	AST* whileStatement(Scope* s);
	AST* forStatement(Scope* s);
	AST* returnStatement(Scope* s);
	AST* continueStatement();
	AST* withStatement(Scope* s);
	AST* breakStatement();
	AST* throwStatement(Scope* s);
	AST* tryStatement(Scope* s);
	AST* expression(Scope* s);
	AST* expression(int pri, Scope* s);
	AST* emptyStatement();
	AST* namedFunction(Scope* s);
	AST* varStatement(Scope* s);
	Declaration* declare(Scope* s);
	Block* block(Scope* s);
	Identifier* identifier(Scope* s);
	std::list<Identifier*>* parameterList(Scope* s);
	AST* leftExpression(Scope* s);
	AST* constructor(Scope* s);
	AST* callExpression(Scope* s);
	std::list<AST*>* arglist(Scope* s);
	AST* primary(Scope* s);
	Function* function(Scope* s);
	Array* literalArray(Scope* s);
	Object* literalObject(Scope* s);
	AST* forbegin(Scope* s);
	AST* forbegin(int pri, Scope* s);

public:
	Parser(Lexer* lex);
	~Parser();

	inline Program* getProgram() { return root_; }
};

NAMESPACE_END

#endif