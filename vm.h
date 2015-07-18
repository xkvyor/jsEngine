#ifndef _VM_H_
#define _VM_H_

#include "parser.h"

NAMESPACE_BEGIN

class ExecError: public std::exception
{
private:
	std::string msg_;

public:
	ExecError(std::string msg): msg_(msg)
	{}
	virtual const char* what() const throw()
	{
		return msg_.c_str();
	}
};

#define CAST(type, ptr) (std::dynamic_pointer_cast<type>(ptr))
#define EXEC_DECL(type) ValuePtr exec(type* code);
#define EXEC(type) {if (dynamic_cast<type*>(code)) {\
				return exec(dynamic_cast<type*>(code));}}
#define BOP_DECL(func) ValuePtr func(ValuePtr left, ValuePtr right);

class VM {
private:
	Scope* global_;

	void throwUnexpectSignal(ValuePtr sig);

	EXEC_DECL(AST)
	EXEC_DECL(Var)
	EXEC_DECL(LiteralString)
	EXEC_DECL(LiteralNumber)
	EXEC_DECL(LiteralBool)
	EXEC_DECL(LiteralNull)
	EXEC_DECL(Identifier)
	EXEC_DECL(Function)
	EXEC_DECL(Block)
	EXEC_DECL(Condition)
	EXEC_DECL(Return)
	EXEC_DECL(Break)
	EXEC_DECL(Continue)
	EXEC_DECL(GroupExpression)
	EXEC_DECL(Call)
	EXEC_DECL(ArrayMember)
	EXEC_DECL(ObjectMember)
	EXEC_DECL(Array)
	EXEC_DECL(Object)
	EXEC_DECL(Keyword)
	EXEC_DECL(Constructor)
	EXEC_DECL(Switch)
	EXEC_DECL(DoLoop)
	EXEC_DECL(Loop)
	EXEC_DECL(ForLoop)
	EXEC_DECL(ForInLoop)
	EXEC_DECL(With)
	EXEC_DECL(UniExpression)
	EXEC_DECL(BiExpression)
	EXEC_DECL(TriExpression)

	ValuePtr assign(AST* left, ValuePtr rval);

	BOP_DECL(plus)
	BOP_DECL(minus)
	BOP_DECL(mul)
	BOP_DECL(div)
	BOP_DECL(mod)
	BOP_DECL(band)
	BOP_DECL(bor)
	BOP_DECL(bxor)
	BOP_DECL(ls)
	BOP_DECL(le)
	BOP_DECL(gt)
	BOP_DECL(ge)
	BOP_DECL(eq)
	BOP_DECL(neq)
	BOP_DECL(teq)
	BOP_DECL(nteq)
	BOP_DECL(lshift)
	BOP_DECL(rshift)

	ValuePtr rev(ValuePtr v);

	void loadBuiltin();

public:
	VM();
	~VM();

	void exec(Program* prog);
};

NAMESPACE_END

#endif