#ifndef _AST_H_
#define _AST_H_

#include "common.h"
#include "lexer.h"

NAMESPACE_BEGIN

class Scope;

struct AST {
	enum Type {
		PROGRAM,
		FUNCTION,
		IDENTIFIER,
		EMPTY,
		VAR,
		DECLARATION,
		BLOCK,
		CONDITION,
		SWITCH,
		CASE,
		DOLOOP,
		LOOP,
		FORLOOP,
		FORINLOOP,
		RETURN,
		BREAK,
		CONTINUE,
		WITH,
		TRY,
		THROW,
		GROUP_EXPR,
		UNI_EXPR,
		BIN_EXPR,
		TRI_EXPR,
		CONSTRUCTOR,
		ARRAY_MEMBER,
		OBJECT_MEMBER,
		CALL,
		LITERAL_BOOL,
		LITERAL_NUMBER,
		LITERAL_STRING,
		LITERAL_NULL,
		KEYWORD,
		ARRAY,
		OBJECT,
		LITERAL_REGULAR,
	};

	Type type_;
	PositionRange range_;
	Scope* scope_;

	AST(Type type, PositionRange range):
		type_(type), range_(range), scope_(NULL)
	{}
	virtual ~AST()
	{}
};

class Identifier: public AST {
public:
	std::string name_;

public:
	Identifier(Token tok): AST(AST::Type::IDENTIFIER, tok.range_), name_(tok.data_)
	{}
	~Identifier()
	{}
};

class Program: public AST {
public:
	std::list<AST*>* stmts_;

public:
	Program(PositionRange range, std::list<AST*>* stmts):
		AST(AST::Type::PROGRAM, range), stmts_(stmts)
	{}
	~Program()
	{
		deletePtr(stmts_);
	}
};

class Function: public AST {
public:
	Identifier* id_;
	std::list<Identifier*>* args_;
	std::list<AST*>* stmts_;

public:
	Function(PositionRange range, Identifier* id,
		std::list<Identifier*>* args, std::list<AST*>* stmts):
		AST(AST::Type::FUNCTION, range), id_(id), args_(args), stmts_(stmts)
	{}
	~Function()
	{
		deletePtr(id_);
		deletePtr(args_);
		deletePtr(stmts_);
	}
};

class Empty: public AST {
public:
	Empty(PositionRange range): AST(AST::Type::EMPTY, range)
	{}
};

class Declaration: public AST {
public:
	Identifier* id_;
	AST* init_;

public:
	Declaration(PositionRange range, Identifier* id, AST* init):
		AST(AST::Type::DECLARATION, range), id_(id), init_(init)
	{}
	~Declaration()
	{
		deletePtr(id_);
		deletePtr(init_);
	}
};

class Var: public AST {
public:
	std::list<Declaration*>* vlist_;

public:
	Var(PositionRange range, std::list<Declaration*>* vlist):
		AST(AST::Type::VAR, range), vlist_(vlist)
	{}
	~Var()
	{
		deletePtr(vlist_);
	}
};

class Block: public AST {
public:
	std::list<AST*>* stmts_;

public:
	Block(PositionRange range, std::list<AST*>* stmts):
		AST(AST::Type::BLOCK, range), stmts_(stmts)
	{}
	~Block()
	{
		deletePtr(stmts_);
	}
};

class Condition: public AST {
public:
	AST* cond_;
	AST* yes_;
	AST* no_;

public:
	Condition(PositionRange range, AST* cond, AST* yes, AST* no):
		AST(AST::Type::CONDITION, range), cond_(cond), yes_(yes), no_(no)
	{}
	~Condition()
	{
		deletePtr(cond_);
		deletePtr(yes_);
		deletePtr(no_);
	}
};

class Switch: public AST {
public:
	AST* expr_;
	std::list<AST*>* branches_;

public:
	Switch(PositionRange range, AST* expr, std::list<AST*>* branches):
		AST(AST::Type::SWITCH, range), expr_(expr), branches_(branches)
	{}
	~Switch()
	{
		deletePtr(expr_);
		deletePtr(branches_);
	}
};

class Case: public AST {
public:
	AST* expr_;

public:
	Case(PositionRange range, AST* expr):
		AST(AST::Type::CASE, range), expr_(expr)
	{}
	~Case()
	{
		deletePtr(expr_);
	}
};

class DoLoop: public AST {
public:
	AST* blk_;
	AST* cond_;

public:
	DoLoop(PositionRange range, AST* blk, AST* cond):
		AST(AST::Type::DOLOOP, range), blk_(blk), cond_(cond)
	{}
	~DoLoop()
	{
		deletePtr(cond_);
		deletePtr(blk_);
	}
};

class Loop: public AST {
public:
	AST* cond_;
	AST* stmt_;

public:
	Loop(PositionRange range, AST* cond, AST* stmt):
		AST(AST::Type::LOOP, range), cond_(cond), stmt_(stmt)
	{}
	~Loop()
	{
		deletePtr(cond_);
		deletePtr(stmt_);
	}
};

class ForLoop: public AST {
public:
	AST* init_;
	AST* cond_;
	AST* iter_;
	AST* stmt_;

public:
	ForLoop(PositionRange range, AST* init, AST* cond, AST* iter, AST* stmt):
		AST(AST::Type::FORLOOP, range), init_(init), cond_(cond), iter_(iter), stmt_(stmt)
	{}
	~ForLoop()
	{
		deletePtr(init_);
		deletePtr(cond_);
		deletePtr(iter_);
		deletePtr(stmt_);
	}
};

class ForInLoop: public AST {
public:
	AST* key_;
	AST* target_;
	AST* stmt_;

public:
	ForInLoop(PositionRange range, AST* key, AST* target, AST* stmt):
		AST(AST::Type::FORINLOOP, range), key_(key), target_(target), stmt_(stmt)
	{}
	~ForInLoop()
	{
		deletePtr(key_);
		deletePtr(target_);
		deletePtr(stmt_);
	}
};

class Return: public AST {
public:
	AST* expr_;

public:
	Return(PositionRange range, AST* expr):
		AST(AST::Type::RETURN, range), expr_(expr)
	{}
	~Return()
	{
		deletePtr(expr_);
	}
};

class Break: public AST {
public:
	Break(PositionRange range):
		AST(AST::Type::BREAK, range)
	{}
};

class Continue: public AST {
public:
	Continue(PositionRange range):
		AST(AST::Type::CONTINUE, range)
	{}
};

class With: public AST {
public:
	AST* expr_;
	AST* stmt_;

public:
	With(PositionRange range, AST* expr, AST* stmt):
		AST(AST::Type::WITH, range), expr_(expr), stmt_(stmt)
	{}
	~With()
	{
		deletePtr(expr_);
		deletePtr(stmt_);
	}
};

class Try: public AST {
public:
	Block* tryblk_;
	std::list<std::pair<AST*, Block*>>* catches_;
	Block* finblk_;

public:
	Try(PositionRange range, Block* tryblk,
		std::list<std::pair<AST*, Block*>>* catches,
		Block* finblk):
		AST(AST::Type::TRY, range), tryblk_(tryblk),
		catches_(catches), finblk_(finblk)
	{}
	~Try()
	{
		deletePtr(tryblk_);
		deletePtr(catches_);
		deletePtr(finblk_);
	}
};

class Throw: public AST {
public:
	AST* expr_;

public:
	Throw(PositionRange range, AST* expr):
		AST(AST::Type::THROW, range), expr_(expr)
	{}
	~Throw()
	{
		deletePtr(expr_);
	}
};

class GroupExpression: public AST {
public:
	std::list<AST*>* elist_;

public:
	GroupExpression(PositionRange range, std::list<AST*>* exprlist):
		AST(AST::Type::GROUP_EXPR, range), elist_(exprlist)
	{}
	~GroupExpression()
	{
		deletePtr(elist_);
	}
};

class UniExpression: public AST {
public:
	std::string op_;
	AST* expr_;
	bool pre_;

public:
	UniExpression(PositionRange range, Token op, AST* expr):
		AST(AST::Type::UNI_EXPR, range),
		op_(op.data_), expr_(expr), pre_(true)
	{}
	UniExpression(PositionRange range, AST* expr, Token op):
		AST(AST::Type::UNI_EXPR, range),
		op_(op.data_), expr_(expr), pre_(false)
	{}
	~UniExpression()
	{
		deletePtr(expr_);
	}
};

class BiExpression: public AST {
public:
	AST* left_;
	std::string op_;
	AST* right_;

public:
	BiExpression(PositionRange range, AST* left, Token op, AST* right):
		AST(AST::Type::BIN_EXPR, range), left_(left), op_(op.data_), right_(right)
	{}
	~BiExpression()
	{
		deletePtr(left_);
		deletePtr(right_);
	}
};

class TriExpression: public AST {
public:
	AST* cond_;
	AST* yes_;
	AST* no_;

public:
	TriExpression(PositionRange range, AST* cond, AST* yes, AST* no):
		AST(AST::Type::TRI_EXPR, range), cond_(cond), yes_(yes), no_(no)
	{}
	~TriExpression()
	{
		deletePtr(cond_);
		deletePtr(yes_);
		deletePtr(no_);
	}
};

class Call;

class Constructor: public AST {
public:
	Call* ctor_;

public:
	Constructor(PositionRange range, Call* ctor):
		AST(AST::Type::CONSTRUCTOR, range), ctor_(ctor)
	{}
	~Constructor()
	{
		deletePtr(ctor_);
	}
};

class ArrayMember: public AST {
public:
	AST* base_;
	AST* attr_;

public:
	ArrayMember(PositionRange range, AST* base, AST* attr):
		AST(AST::Type::ARRAY_MEMBER, range), base_(base), attr_(attr)
	{}
	~ArrayMember()
	{
		deletePtr(base_);
		deletePtr(attr_);
	}
};

class ObjectMember: public AST {
public:
	AST* base_;
	AST* attr_;

public:
	ObjectMember(PositionRange range, AST* base, AST* attr):
		AST(AST::Type::OBJECT_MEMBER, range), base_(base), attr_(attr)
	{}
	~ObjectMember()
	{
		deletePtr(base_);
		deletePtr(attr_);
	}
};

class Call: public AST {
public:
	AST* func_;
	std::list<AST*>* args_;

public:
	Call(PositionRange range, AST* func, std::list<AST*>* args):
		AST(AST::Type::CALL, range), func_(func), args_(args)
	{}
	~Call()
	{
		deletePtr(func_);
		deletePtr(args_);
	}
};

class LiteralBool: public AST {
public:
	bool b_;

public:
	LiteralBool(Token b): AST(AST::Type::LITERAL_BOOL, b.range_), b_(b.data_ == "true")
	{}
};

class LiteralNumber: public AST {
public:
	std::string data_;

public:
	LiteralNumber(Token n): AST(AST::Type::LITERAL_NUMBER, n.range_), data_(n.data_)
	{}
};

class LiteralString: public AST {
public:
	std::string str_;

public:
	LiteralString(Token s): AST(AST::Type::LITERAL_STRING, s.range_),
		str_(s.data_.substr(1, s.data_.length()-2))
	{}
};

class LiteralNull: public AST {
public:
	LiteralNull(Token n): AST(AST::Type::LITERAL_NULL, n.range_)
	{}
};

class Keyword: public AST {
public:
	std::string data_;

public:
	Keyword(Token n): AST(AST::Type::KEYWORD, n.range_), data_(n.data_)
	{}
};

class Array: public AST {
public:
	std::list<AST*>* elem_;

public:
	Array(PositionRange range, std::list<AST*>* elem):
		AST(AST::Type::ARRAY, range), elem_(elem)
	{}
	~Array()
	{
		deletePtr(elem_);
	}
};

class Object: public AST {
public:
	std::list<std::pair<AST*, AST*>>* kv_;

public:
	Object(PositionRange range, std::list<std::pair<AST*, AST*>>* kv):
		AST(AST::Type::OBJECT, range), kv_(kv)
	{}
	~Object()
	{
		deletePtr(kv_);
	}
};

class LiteralRegular: public AST {
public:
	std::string re_;

public:
	LiteralRegular(Token s): AST(AST::Type::LITERAL_REGULAR, s.range_),
		re_(s.data_)
	{}
};

NAMESPACE_END

#endif