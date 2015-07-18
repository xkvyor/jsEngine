#include "parser.h"

NAMESPACE_BEGIN

Parser::Parser(Lexer* lex) : root_(NULL), lex_(lex)
{
	lex_->restart();
	root_ = program();
}

Parser::~Parser()
{
	deletePtr(root_);
}

Token Parser::match(std::string s)
{
	Token tok = lex_->get();
	if (tok.data_ != s)
	{
		std::stringstream ss;
		ss << "Expect [" << s << "], but get " << tok.toString();
		throw ParseError(ss.str());
	}
	return tok;
}

Token Parser::match(Token::Type type)
{
	Token tok = lex_->get();
	if (tok.type_ != type)
	{
		std::stringstream ss;
		ss << "Unexpected " << tok.toString();
		throw ParseError(ss.str());
	}
	return tok;
}

bool Parser::expect(std::string s)
{
	return (lex_->peek().data_ == s);
}

bool Parser::expect(Token::Type type)
{
	return (lex_->peek().type_ == type);
}

Program* Parser::program()
{
	Scope* s = new Scope(NULL);
	Position begin = lex_->peek().range_.begin_;
	auto stmts = topStatements(s);
	Position end = lex_->peek().range_.begin_;
	match(Token::Type::END_OF_FILE);
	auto ret = new Program(PositionRange(begin, end), stmts);
	ret->scope_ = s;
	return ret;
}

std::list<AST*>* Parser::topStatements(Scope* ps)
{
	auto ret = new std::list<AST*>();
	while (!expect(Token::Type::END_OF_FILE) && !expect("}"))
	{
		AST* tmp = topStatement(ps);
		if (tmp)
		{
			ret->push_back(tmp);
		}
	}
	return ret;
}

AST* Parser::topStatement(Scope* ps)
{
	if (expect("function"))
	{
		return namedFunction(ps);
	}
	else
	{
		return statement(ps);
	}
}

AST* Parser::namedFunction(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	Scope* s = new Scope(ps);

	match("function");
	auto name = identifier(ps);
	match("(");
	auto plist = parameterList(s);
	match(")");
	match("{");
	auto stmts = topStatements(s);
	match("}");

	Position end = lex_->peek().range_.begin_;

	auto ret = new Function(PositionRange(begin, end), name, plist, stmts);
	ret->scope_ = s;

	return ret;
}

std::list<Identifier*>* Parser::parameterList(Scope* ps)
{
	auto ret = new std::list<Identifier*>();
	if (expect(Token::Type::IDENTIFIER))
	{
		ret->push_back(identifier(ps));
		while (expect(","))
		{
			match(",");
			ret->push_back(identifier(ps));
		}
	}
	return ret;
}

void Parser::opteol()
{
	if (expect(";"))
	{
		match(";");
	}
}

AST* Parser::statement(Scope* ps)
{
	AST* ret = NULL;

	if (expect(";"))
	{
		ret = emptyStatement();
	}
	else if (expect("var"))
	{
		ret = varStatement(ps);
		opteol();
	}
	else if (expect("{"))
	{
		ret = block(ps);
		opteol();
	}
	else if (expect("if"))
	{
		ret = ifStatement(ps);
	}
	else if (expect("switch"))
	{
		ret = switchStatement(ps);
	}
	else if (expect("do"))
	{
		ret = doStatement(ps);
		opteol();
	}
	else if (expect("while"))
	{
		ret = whileStatement(ps);
	}
	else if (expect("for"))
	{
		ret = forStatement(ps);
	}
	else if (expect("with"))
	{
		ret = withStatement(ps);
	}
	else if (expect("continue"))
	{
		ret = continueStatement();
		opteol();
	}
	else if (expect("break"))
	{
		ret = breakStatement();
		opteol();
	}
	else if (expect("return"))
	{
		ret = returnStatement(ps);
		opteol();
	}
	else if (expect("try"))
	{
		ret = tryStatement(ps);
	}
	else if (expect("throw"))
	{
		ret = throwStatement(ps);
		opteol();
	}
	else
	{
		ret = expression(0, ps);
		opteol();
	}

	return ret;
}

AST* Parser::emptyStatement()
{
	Position begin = lex_->peek().range_.begin_;

	match(";");

	Position end = lex_->peek().range_.begin_;

	return new Empty(PositionRange(begin, end));
}

AST* Parser::varStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	match("var");

	auto decl = declare(ps);

	auto vlist = new std::list<Declaration*>();
	vlist->push_back(decl);

	while (expect(","))
	{
		match(",");
		decl = declare(ps);
		vlist->push_back(decl);
	}

	Position end = lex_->peek().range_.begin_;

	auto ret = new Var(PositionRange(begin, end), vlist);
	ret->scope_ = ps;

	return ret;
}

Declaration* Parser::declare(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	Identifier* id = identifier(ps);

	AST* init = NULL;
	if (expect("="))
	{
		match("=");
		init = expression(0, ps);
	}

	Position end = lex_->peek().range_.begin_;

	auto ret = new Declaration(PositionRange(begin, end), id, init);
	ret->scope_ = ps;

	return ret;
}

Block* Parser::block(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	Scope* s = new Scope(ps);
	match("{");
	auto stmts = statements(s);
	match("}");
	Position end = lex_->peek().range_.begin_;

	auto ret = new Block(PositionRange(begin, end), stmts);
	ret->scope_ = s;

	return ret;
}

std::list<AST*>* Parser::statements(Scope* ps)
{
	auto ret = new std::list<AST*>();
	while (!expect("}"))
	{
		ret->push_back(statement(ps));
	}
	return ret;
}

AST* Parser::ifStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	Scope* s = new Scope(ps);
	match("if");
	match("(");
	AST* cond = expression(s);
	match(")");
	AST* yes = statement(s);
	AST* no = NULL;
	if (expect("else"))
	{
		match("else");
		no = statement(s);
	}
	Position end = lex_->peek().range_.begin_;

	auto ret = new Condition(PositionRange(begin, end), cond, yes, no);
	ret->scope_ = s;

	return ret;
}

AST* Parser::switchStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	Scope* s = new Scope(ps);

	match("switch");
	match("(");
	AST* expr = expression(s);
	match(")");
	match("{");

	auto branches = new std::list<AST*>();

	while (!expect("}"))
	{
		if (expect("case"))
		{
			match("case");
			AST* v = expression(s);
			match(":");
			Position end = lex_->peek().range_.begin_;
			branches->push_back(new Case(PositionRange(begin, end), v));
		}
		else if (expect("default"))
		{
			match("default");
			match(":");
			Position end = lex_->peek().range_.begin_;
			branches->push_back(new Case(PositionRange(begin, end), NULL));
		}
		else
		{
			branches->push_back(statement(s));
		}
	}

	match("}");

	Position end = lex_->peek().range_.begin_;

	auto ret = new Switch(PositionRange(begin, end), expr, branches);
	ret->scope_ = s;

	return ret;
}

AST* Parser::doStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	match("do");
	AST* blk = block(ps);
	match("while");
	match("(");
	AST* cond = statement(ps);
	match(")");
	Position end = lex_->peek().range_.begin_;

	auto ret = new DoLoop(PositionRange(begin, end), blk, cond);
	ret->scope_ = ps;

	return ret;
}

AST* Parser::whileStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	Scope* s = new Scope(ps);
	match("while");
	match("(");
	AST* cond = expression(s);
	match(")");
	AST* body = statement(s);
	Position end = lex_->peek().range_.begin_;

	auto ret = new Loop(PositionRange(begin, end), cond, body);
	ret->scope_ = s;

	return ret;
}

AST* Parser::forStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	Scope* s = new Scope(ps);

	match("for");
	match("(");

	AST* init = NULL;

	if (expect("var"))
	{
		init = varStatement(s);

		if (expect(";"))
		{
			match(";");
			goto forloop;
		}
		else
		{
			auto in = match("in");

			if (dynamic_cast<Var*>(init)->vlist_->size() != 1)
			{
				std::stringstream ss;
				ss << "Unexpected token before " << in.toString();
				throw ParseError(ss.str());
			}

			goto forin;
		}
	}
	else if (expect(";"))
	{
		match(";");
		goto forloop;
	}
	else
	{
		init = forbegin(s);

		if (expect(";"))
		{
			match(";");
			goto forloop;
		}
		else
		{
			auto in = match("in");

			auto el = dynamic_cast<GroupExpression*>(init)->elist_;

			if (el->size() != 1 ||
				(*el->begin())->type_ != AST::Type::IDENTIFIER)
			{
				std::stringstream ss;
				ss << "Unexpected token before " << in.toString();
				throw ParseError(ss.str());
			}

			Identifier* id = new Identifier(*dynamic_cast<Identifier*>(*el->begin()));
			deletePtr(init);
			init = id;

			goto forin;
		}
	}

	forloop:
	{
		AST* cond = NULL;
		if (!expect(";"))
		{
			cond = expression(s);
		}
		match(";");
		AST* tail = NULL;
		if (!expect(")"))
		{
			tail = expression(s);
		}

		match(")");
		AST* body = statement(s);

		Position end = lex_->peek().range_.begin_;

		auto ret = new ForLoop(PositionRange(begin, end), init, cond, tail, body);
		ret->scope_ = s;

		return ret;
	}

	forin:
	{
		AST* expr = expression(s);
		match(")");
		AST* body = statement(s);

		Position end = lex_->peek().range_.begin_;

		auto ret = new ForInLoop(PositionRange(begin, end), init, expr, body);
		ret->scope_ = s;

		return ret;
	}
}

AST* Parser::returnStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;
	match("return");
	AST* ret = NULL;
	if (!(expect(";") || expect("}")
		|| lex_->peek().range_.begin_.line_ > begin.line_))
	{
		ret = expression(ps);
	}
	Position end = lex_->peek().range_.begin_;

	auto rval = new Return(PositionRange(begin, end), ret);
	rval->scope_ = ps;

	return rval;
}

AST* Parser::breakStatement()
{
	Position begin = lex_->peek().range_.begin_;
	match("break");
	Position end = lex_->peek().range_.begin_;
	return new Break(PositionRange(begin, end));
}

AST* Parser::continueStatement()
{
	Position begin = lex_->peek().range_.begin_;
	match("continue");
	Position end = lex_->peek().range_.begin_;
	return new Continue(PositionRange(begin, end));
}

AST* Parser::withStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	Scope* s = new Scope(ps);
	match("with");
	match("(");
	AST* expr = expression(s);
	match(")");
	AST* stmt = statement(s);
	Position end = lex_->peek().range_.begin_;

	auto ret = new With(PositionRange(begin, end), expr, stmt);
	ret->scope_ = s;

	return ret;
}

AST* Parser::throwStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;
	match("throw");
	AST* expr = expression(ps);
	Position end = lex_->peek().range_.begin_;
	auto ret = new Throw(PositionRange(begin, end), expr);
	ret->scope_ = ps;
	return ret;
}

AST* Parser::tryStatement(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	match("try");
	auto tryblk = block(ps);

	auto catches = new std::list<std::pair<AST*, Block*>>();
	while (expect("catch"))
	{
		match("catch");
		match("(");
		Scope* s = new Scope(ps);
		AST* expr = expression(s);
		match(")");
		auto blk = block(s);
		blk->scope_ = s;
		catches->push_back(std::make_pair(expr, blk));
	}

	Block* finblk = NULL;
	if (expect("finally"))
	{
		match("finally");
		finblk = block(ps);
	}

	Position end = lex_->peek().range_.begin_;

	return new Try(PositionRange(begin, end), tryblk, catches, finblk);
}

AST* Parser::expression(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	auto exprlist = new std::list<AST*>();
	exprlist->push_back(expression(0, ps));

	while (expect(","))
	{
		match(",");
		exprlist->push_back(expression(0, ps));
	}

	Position end = lex_->peek().range_.begin_;

	auto ret = new GroupExpression(PositionRange(begin, end), exprlist);
	ret->scope_ = ps;

	return ret;
}

AST* Parser::expression(int pri, Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	if (pri > 11)
	{
		if (expect("delete")
			|| expect("++")
			|| expect("--"))
		{
			Token op = lex_->get();
			AST* expr = leftExpression(ps);
			Position end = lex_->peek().range_.begin_;
			auto ret = new UniExpression(PositionRange(begin, end), op, expr);
			ret->scope_ = ps;
			return ret;
		}
		else if (expect("void")
			|| expect("typeof")
			|| expect("+")
			|| expect("-")
			|| expect("~")
			|| expect("!"))
		{
			Token op = lex_->get();
			AST* expr = expression(pri, ps);
			Position end = lex_->peek().range_.begin_;
			auto ret = new UniExpression(PositionRange(begin, end), op, expr);
			ret->scope_ = ps;
			return ret;
		}
		else
		{
			AST* expr = leftExpression(ps);
			if (expect("++") || expect("--"))
			{
				Token op = lex_->get();
				Position end = lex_->peek().range_.begin_;
				auto ret = new UniExpression(PositionRange(begin, end), expr, op);
				ret->scope_ = ps;
				return ret;
			}
			return expr;
		}
	}

	AST* left = expression(pri+1, ps);

	if (!expectOperator(pri))
	{
		return left;
	}

	Token op = lex_->get();

	if (pri == 1)
	{
		AST* first = expression(pri, ps);
		match(":");
		AST* second = expression(pri, ps);
		Position end = lex_->peek().range_.begin_;
		auto ret = new TriExpression(PositionRange(begin, end), left, first, second);
		ret->scope_ = ps;
		return ret;
	}

	AST* right = expression(pri, ps);

	Position end = lex_->peek().range_.begin_;

	AST* ret = new BiExpression(PositionRange(begin, end), left, op, right);
	ret->scope_ = ps;

	while (expectOperator(pri))
	{
		op = lex_->get();
		right = expression(pri+1, ps);
		ret = new BiExpression(PositionRange(begin, end), left, op, right);
		ret->scope_ = ps;
	}

	return ret;
}

bool Parser::expectOperator(int pri)
{
	return (PRIORITY.find(lex_->peek().data_) != PRIORITY.end()
		&& PRIORITY.find(lex_->peek().data_)->second == pri);
}

AST* Parser::leftExpression(Scope* ps)
{
	if (expect("new"))
	{
		return constructor(ps);
	}
	else
	{
		return callExpression(ps);
	}
}

AST* Parser::constructor(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	match("new");

	AST* ctor = callExpression(ps);

	if (dynamic_cast<Call*>(ctor) == NULL)
	{
		std::stringstream ss;
		ss << "Initializer is not a function before " << lex_->peek().toString();
		throw ParseError(ss.str());
	}

	Position end = lex_->peek().range_.begin_;

	auto ret = new Constructor(PositionRange(begin, end), dynamic_cast<Call*>(ctor));
	ret->scope_ = ps;

	return ret;
}

AST* Parser::callExpression(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	AST* expr = primary(ps);

	for (;;)
	{
		if (expect("."))
		{
			match(".");
			Identifier* mem = identifier(ps);
			Position end = lex_->peek().range_.begin_;
			expr = new ObjectMember(PositionRange(begin, end), expr, mem);
			expr->scope_ = ps;
		}
		else if (expect("("))
		{
			auto args = arglist(ps);
			Position end = lex_->peek().range_.begin_;
			expr = new Call(PositionRange(begin, end), expr, args);
			expr->scope_ = ps;
		}
		else if (expect("["))
		{
			match("[");
			AST* key = expression(0, ps);
			match("]");
			Position end = lex_->peek().range_.begin_;
			expr = new ArrayMember(PositionRange(begin, end), expr, key);
			expr->scope_ = ps;
		}
		else
		{
			break;
		}
	}

	return expr;
}

std::list<AST*>* Parser::arglist(Scope* ps)
{
	auto ret = new std::list<AST*>();

	if (expect("("))
	{
		match("(");
		if (expect(")"))
		{
			match(")");
		}
		else
		{
			for (;;)
			{
				ret->push_back(expression(0, ps));
				if (expect(","))
				{
					match(",");
				}
				if (expect(")"))
				{
					match(")");
					break;
				}
			}
		}
	}

	return ret;
}

Identifier* Parser::identifier(Scope* s)
{
	auto id = match(Token::Type::IDENTIFIER);
	auto ret = new Identifier(id);
	ret->scope_ = s;
	return ret;
}

AST* Parser::primary(Scope* ps)
{
	if (expect("("))
	{
		match("(");
		AST* expr = expression(0, ps);
		match(")");
		return expr;
	}
	else if (expect(Token::Type::IDENTIFIER))
	{
		return identifier(ps);
	}
	else if (expect("true") || expect("false"))
	{
		return new LiteralBool(lex_->get());
	}
	else if (expect("null"))
	{
		return new LiteralNull(lex_->get());
	}
	else if (expect(Token::Type::STRING))
	{
		return new LiteralString(lex_->get());
	}
	else if (expect(Token::Type::NUMBER))
	{
		return new LiteralNumber(lex_->get());
	}
	else if (expect("this") || expect("arguments"))
	{
		auto ret = new Keyword(lex_->get());
		ret->scope_ = ps;
		return ret;
	}
	else if (expect("["))
	{
		return literalArray(ps);
	}
	else if (expect("{"))
	{
		return literalObject(ps);
	}
	else if (expect("function"))
	{
		return function(ps);
	}
	else if (expect(Token::Type::REGULAR))
	{
		return new LiteralRegular(lex_->get());
	}

	// throw exception?
	std::stringstream ss;
	ss << "Can not parse primary-expression, " << lex_->peek().toString();
	throw ParseError(ss.str());
}

Function* Parser::function(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	match("function");

	Scope* s = new Scope(ps);

	Identifier* name = NULL;
	if (expect(Token::Type::IDENTIFIER))
	{
		name = identifier(ps);
	}
	match("(");
	auto plist = parameterList(s);
	match(")");
	match("{");
	auto stmts = topStatements(s);
	match("}");

	Position end = lex_->peek().range_.begin_;

	auto ret = new Function(PositionRange(begin, end), name, plist, stmts);
	ret->scope_ = s;

	return ret;
}

Array* Parser::literalArray(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	match("[");

	auto elem = new std::list<AST*>();

	if (!expect("]"))
	{
		for (;;)
		{
			elem->push_back(expression(0, ps));
			if (expect(","))
			{
				match(",");
				if (expect("]"))
				{
					match("]");
					break;
				}
			}
			else
			{
				match("]");
				break;
			}
		}
	}
	else
	{
		match("]");
	}

	Position end = lex_->peek().range_.begin_;

	return new Array(PositionRange(begin, end), elem);
}

Object* Parser::literalObject(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	match("{");

	auto kv = new std::list<std::pair<AST*, AST*>>();

	if (expect("}"))
	{
		match("}");
	}
	else
	{
		for (;;)
		{
			AST* key = primary(ps);

			match(":");

			AST* val = expression(0, ps);

			kv->push_back(std::make_pair(key, val));

			if (expect("}"))
			{
				match("}");
				break;
			}
			else
			{
				match(",");
				if (expect("}"))
				{
					match("}");
					break;
				}
			}
		}
	}

	Position end = lex_->peek().range_.begin_;

	return new Object(PositionRange(begin, end), kv);
}

AST* Parser::forbegin(int pri, Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	if (pri > 11)
	{
		if (expect("delete")
			|| expect("++")
			|| expect("--"))
		{
			Token op = lex_->get();
			AST* expr = leftExpression(ps);
			Position end = lex_->peek().range_.begin_;
			auto ret = new UniExpression(PositionRange(begin, end), op, expr);
			ret->scope_ = ps;
			return ret;
		}
		else if (expect("void")
			|| expect("typeof")
			|| expect("+")
			|| expect("-")
			|| expect("~")
			|| expect("!"))
		{
			Token op = lex_->get();
			AST* expr = expression(pri, ps);
			Position end = lex_->peek().range_.begin_;
			auto ret = new UniExpression(PositionRange(begin, end), op, expr);
			ret->scope_ = ps;
			return ret;
		}
		else
		{
			AST* expr = leftExpression(ps);
			if (expect("++") || expect("--"))
			{
				Token op = lex_->get();
				Position end = lex_->peek().range_.begin_;
				auto ret = new UniExpression(PositionRange(begin, end), expr, op);
				ret->scope_ = ps;
			}
			return expr;
		}
	}

	AST* left = forbegin(pri+1, ps);

	if (!expectOperator(pri) || expect("in"))
	{
		return left;
	}

	Token op = lex_->get();

	if (pri == 1)
	{
		AST* first = forbegin(pri, ps);
		match(":");
		AST* second = forbegin(pri, ps);
		Position end = lex_->peek().range_.begin_;
		auto ret = new TriExpression(PositionRange(begin, end), left, first, second);
		ret->scope_ = ps;
	}

	AST* right = forbegin(pri, ps);

	Position end = lex_->peek().range_.begin_;

	AST* ret = new BiExpression(PositionRange(begin, end), left, op, right);
	ret->scope_ = ps;

	while (expectOperator(pri))
	{
		op = lex_->get();
		right = forbegin(pri+1, ps);
		ret = new BiExpression(PositionRange(begin, end), left, op, right);
		ret->scope_ = ps;
	}

	return ret;
}

AST* Parser::forbegin(Scope* ps)
{
	Position begin = lex_->peek().range_.begin_;

	auto exprlist = new std::list<AST*>();
	exprlist->push_back(forbegin(0, ps));

	while (expect(","))
	{
		match(",");
		exprlist->push_back(forbegin(0, ps));
	}

	Position end = lex_->peek().range_.begin_;

	auto ret = new GroupExpression(PositionRange(begin, end), exprlist);
	ret->scope_ = ps;

	return ret;
}

NAMESPACE_END