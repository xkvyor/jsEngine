#include "vm.h"

NAMESPACE_BEGIN

VM::VM()
{}

VM::~VM()
{}

void VM::throwUnexpectSignal(ValuePtr v)
{
	std::stringstream ss;
	ss << "Unexpected control signal at "
			<< CAST(Signal, v)->pos_.toString();
	throw ExecError(ss.str());
}

void VM::exec(Program* prog)
{
	std::cout << "Execute a program" << std::endl;

	global_ = prog->scope_;

	loadBuiltin();

	for (auto i : *prog->stmts_)
	{
		ValuePtr ret = exec(i);
		if (ret->type_ == Value::Type::SIGNAL)
		{
			if (CAST(Signal, ret)->sigtype_ != Signal::Type::NORMAL)
			{
				throwUnexpectSignal(ret);
			}
		}
	}

	std::cout << "Execution finished" << std::endl;

	auto m = prog->scope_->getValueMap();
	for (auto i : m)
	{
		std::cout << "var: " << i.first
			<< " == " << i.second->toString() << std::endl;
	}
}

ValuePtr VM::exec(AST* code)
{
	EXEC(Var)
	EXEC(LiteralString)
	EXEC(LiteralNumber)
	EXEC(LiteralBool)
	EXEC(LiteralNull)
	EXEC(Identifier)
	EXEC(Function)
	EXEC(Block)
	EXEC(Condition)
	EXEC(Return)
	EXEC(BiExpression)
	EXEC(Break)
	EXEC(Continue)
	EXEC(GroupExpression)
	EXEC(Call)
	EXEC(ArrayMember)
	EXEC(ObjectMember)
	EXEC(Array)
	EXEC(Object)
	EXEC(Keyword)
	EXEC(Constructor)
	EXEC(Switch)
	EXEC(DoLoop)
	EXEC(Loop)
	EXEC(ForLoop)
	EXEC(ForInLoop)
	EXEC(With)
	EXEC(UniExpression)
	EXEC(BiExpression)
	EXEC(TriExpression)

	return Signal::sigNormal();
}

ValuePtr VM::exec(Var* v)
{
	ValuePtr ret = NULL;

	for (auto i : *v->vlist_)
	{
		Declaration* d = dynamic_cast<Declaration*>(i);
		if (d->init_)
		{
			ret = exec(d->init_);
		}
		else
		{
			ret = Undefined::instance();
		}
		v->scope_->setVar(d->id_->name_, ret);
		std::cout << "var " << d->id_->name_ << " = " << ret->toString() << std::endl;
	}

	return ret;
}

ValuePtr VM::exec(LiteralString* str)
{
	return ValuePtr(new StringValue(str->str_));
}

ValuePtr VM::exec(LiteralNumber* n)
{
	return ValuePtr(new Number(std::stoll(n->data_)));
}

ValuePtr VM::exec(LiteralBool* b)
{
	return ValuePtr(new Boolean(b->b_));
}

ValuePtr VM::exec(LiteralNull* n)
{
	return NullValue::instance();
}

ValuePtr VM::exec(Identifier* id)
{
	ValuePtr ret = id->scope_->getVar(id->name_);
	if (ret == NULL)
	{
		// std::stringstream ss;
		// ss << "Unknow identifier [" << id->name_ << "] at "
		// 	<< id->range_.toString();
		// throw ExecError(ss.str());
		return Undefined::instance();
	}
	return ret;
}

ValuePtr VM::exec(Function* f)
{
	return ValuePtr(new FunctionValue(f));
}

ValuePtr VM::exec(Block* b)
{
	for (auto i : *b->stmts_)
	{
		auto v = exec(i);
		if (v->type_ == Value::Type::SIGNAL)
		{
			return v;
		}
	}

	return Signal::sigNormal();
}

ValuePtr VM::exec(Condition* f)
{
	auto r = exec(f->cond_);

	bool check = r->toBool();

	if (check)
	{
		r = exec(f->yes_);
	}
	else
	{
		r = exec(f->no_);
	}

	if (r->type_ == Value::Type::SIGNAL)
	{
		return r;
	}

	return Signal::sigNormal();
}

ValuePtr VM::exec(GroupExpression* g)
{
	ValuePtr ret = nullptr;

	for (auto i : *g->elist_)
	{
		ret = exec(i);

		if (ret->type_ == Value::Type::SIGNAL)
		{
			throwUnexpectSignal(ret);
		}
	}

	return ret;
}

ValuePtr VM::exec(Break* f)
{
	return Signal::sigBreak(f);
}

ValuePtr VM::exec(Continue* c)
{
	return Signal::sigContinue(c);
}

ValuePtr VM::exec(Return* r)
{
	if (r->expr_)
	{
		return Signal::sigReturn(exec(r->expr_));
	}
	else
	{
		return Signal::sigReturn(NullValue::instance());
	}
}

ValuePtr VM::exec(ArrayMember* a)
{
	ValuePtr attr = exec(a->attr_);
	std::string key = attr->toString();

	ValuePtr ref = exec(a->base_);

	if (ref->type_ == Value::Type::UNDEFINED
		|| ref->type_ == Value::Type::NULLVAL)
	{
		std::stringstream ss;
		ss << "Can not get attr [" << key << "] for " << ref->toString()
			<< " at " << a->range_.toString();
		throw ExecError(ss.str());
	}

	return ref->getAttr(key);
}

ValuePtr VM::exec(Call* c)
{
	ValuePtr fv = exec(c->func_);

	if (CAST(FunctionValue, fv) == nullptr)
	{
		std::stringstream ss;
		ss << "Only function can be invoked at " << c->range_.toString();
		throw ExecError(ss.str());
	}

	auto func = CAST(FunctionValue, fv)->code_;
	auto rarg = c->args_->begin();

	ValuePtr arguments(new ObjectValue);
	ValuePtr me(new ObjectValue);
	int i = 0;

	for (auto arg : *func->args_)
	{
		if (rarg != c->args_->end())
		{
			auto v = exec(*rarg++);
			func->scope_->setVar(arg->name_, v);
			arguments->setAttr(std::to_string(i++), v);
		}
	}

	func->scope_->getValueMap()["arguments"] = arguments;
	func->scope_->getValueMap()["this"] = me;

	for (auto stmt : *func->stmts_)
	{
		auto ret = exec(stmt);
		if (ret->type_ == Value::Type::SIGNAL)
		{
			if (CAST(Signal, ret)->sigtype_ == Signal::Type::RETURN)
			{
				return CAST(Signal, ret)->val_;
			}
			else if (CAST(Signal, ret)->sigtype_ == Signal::Type::NORMAL)
			{
				continue;
			}
			else
			{
				throwUnexpectSignal(ret);
			}
		}
	}

	return NullValue::instance();
}

ValuePtr VM::exec(ObjectMember* o)
{
	auto key = dynamic_cast<Identifier*>(o->attr_)->name_;

	ValuePtr ref = exec(o->base_);

	if (ref->type_ == Value::Type::UNDEFINED
		|| ref->type_ == Value::Type::NULLVAL)
	{
		std::stringstream ss;
		ss << "Can not get attr [" << key << "] for " << ref->toString()
			<< " at " << o->range_.toString();
		throw ExecError(ss.str());
	}

	return ref->getAttr(key);
}

ValuePtr VM::exec(Array* arr)
{
	ValuePtr ret(new ObjectValue());
	int i = 0;

	for (auto e : *arr->elem_)
	{
		ret->setAttr(std::to_string(i++), exec(e));
	}

	return ret;
}

ValuePtr VM::exec(Object* obj)
{
	ValuePtr ret(new ObjectValue());

	for (auto p : *obj->kv_)
	{
		ret->setAttr(exec(p.first)->toString(), exec(p.second));
	}

	return ret;
}

ValuePtr VM::exec(Keyword* kw)
{
	return kw->scope_->getVar(kw->data_);
}

ValuePtr VM::exec(Constructor* c)
{
	auto called = c->ctor_;

	ValuePtr fv = exec(called->func_);

	if (CAST(FunctionValue, fv) == nullptr)
	{
		std::stringstream ss;
		ss << "Only function can be invoked at " << called->range_.toString();
		throw ExecError(ss.str());
	}

	auto func = CAST(FunctionValue, fv)->code_;
	auto rarg = called->args_->begin();

	ValuePtr arguments(new ObjectValue);
	ValuePtr me(new ObjectValue);
	int i = 0;

	for (auto arg : *func->args_)
	{
		if (rarg != called->args_->end())
		{
			auto v = exec(*rarg++);
			func->scope_->setVar(arg->name_, v);
			arguments->setAttr(std::to_string(i++), v);
		}
	}

	func->scope_->getValueMap()["arguments"] = arguments;
	func->scope_->getValueMap()["this"] = me;

	for (auto stmt : *func->stmts_)
	{
		auto ret = exec(stmt);
		if (ret->type_ == Value::Type::SIGNAL)
		{
			if (CAST(Signal, ret)->sigtype_ == Signal::Type::RETURN)
			{
				break;
			}
			else if (CAST(Signal, ret)->sigtype_ == Signal::Type::NORMAL)
			{
				continue;
			}
			else
			{
				throwUnexpectSignal(ret);
			}
		}
	}

	return me;
}

ValuePtr VM::exec(Switch* sw)
{
	auto val = exec(sw->expr_);

	enum {
		EXECUTE,
		IGNORE
	} state = IGNORE;

	for (auto stmt : *sw->branches_)
	{
		if (stmt->type_ == AST::Type::CASE)
		{
			if (dynamic_cast<Case*>(stmt)->expr_ == NULL
				|| eq(exec(dynamic_cast<Case*>(stmt)->expr_), val))
			{
				state = EXECUTE;
			}
			else
			{
				state = IGNORE;
			}
		}
		else if (state == EXECUTE)
		{
			auto ret = exec(stmt);

			if (ret->type_ == Value::Type::SIGNAL)
			{
				if (CAST(Signal, ret)->sigtype_ == Signal::Type::BREAK)
				{
					break;
				}
				else
				{
					return ret;
				}
			}
		}
	}

	return Signal::sigNormal();
}

ValuePtr VM::exec(DoLoop* dl)
{
	ValuePtr check = nullptr;
	do
	{
		auto ret = exec(dl->blk_);
		if (ret->type_ == Value::Type::SIGNAL)
		{
			if (CAST(Signal, ret)->sigtype_ == Signal::Type::RETURN)
			{
				return ret;
			}
			else if (CAST(Signal, ret)->sigtype_ == Signal::Type::BREAK)
			{
				break;
			}
		}

		check = exec(dl->cond_);
	} while (check->toBool());

	return Signal::sigNormal();
}

ValuePtr VM::exec(Loop* lp)
{
	while (exec(lp->cond_)->toBool())
	{
		auto ret = exec(lp->stmt_);
		if (ret->type_ == Value::Type::SIGNAL)
		{
			if (CAST(Signal, ret)->sigtype_ == Signal::Type::RETURN)
			{
				return ret;
			}
			else if (CAST(Signal, ret)->sigtype_ == Signal::Type::BREAK)
			{
				break;
			}
		}
	}

	return Signal::sigNormal();
}

ValuePtr VM::exec(ForLoop* fl)
{
	exec(fl->init_);

	while (exec(fl->cond_)->toBool())
	{
		auto ret = exec(fl->stmt_);
		if (ret->type_ == Value::Type::SIGNAL)
		{
			if (CAST(Signal, ret)->sigtype_ == Signal::Type::RETURN)
			{
				return ret;
			}
			else if (CAST(Signal, ret)->sigtype_ == Signal::Type::BREAK)
			{
				break;
			}
		}

		exec(fl->iter_);
	}

	return Signal::sigNormal(); 
}

ValuePtr VM::exec(ForInLoop* fi)
{
	std::string i;

	exec(fi->key_);

	if (fi->key_->type_ == AST::Type::VAR)
	{
		auto var = dynamic_cast<Var*>(fi->key_);
		i = (*var->vlist_->begin())->id_->name_;
	}
	else if (fi->key_->type_ == AST::Type::IDENTIFIER)
	{
		i = dynamic_cast<Identifier*>(fi->key_)->name_;
	}
	else
	{
		std::stringstream ss;
		ss << "Unexpected token in for-loop at "
			<< fi->key_->range_.toString();
		throw ExecError(ss.str());
	}

	ValuePtr obj = exec(fi->target_);

	if (obj->type_ == Value::Type::SIGNAL)
	{
		std::stringstream ss;
		ss << "Illegal for-loop at " << fi->target_->range_.toString();
		throw ExecError(ss.str());
	}

	if (obj->type_ == Value::Type::STRING)
	{
		auto s = CAST(StringValue, obj)->str_;

		for (auto c : s)
		{
			std::string tmp(1, c);
			auto v = ValuePtr(new StringValue(tmp));
			fi->scope_->setVar(i, v);
			auto ret = exec(fi->stmt_);
			if (ret->type_ == Value::Type::SIGNAL)
			{
				if (CAST(Signal, ret)->sigtype_ == Signal::Type::RETURN)
				{
					return ret;
				}
				else if (CAST(Signal, ret)->sigtype_ == Signal::Type::BREAK)
				{
					break;
				}
			}
		}

		return Signal::sigNormal();
	}

	auto keys = obj->getKeys();

	for (auto key : keys)
	{
		fi->scope_->setVar(i, obj->getAttr(key));
		auto ret = exec(fi->stmt_);
		if (ret->type_ == Value::Type::SIGNAL)
		{
			if (CAST(Signal, ret)->sigtype_ == Signal::Type::RETURN)
			{
				return ret;
			}
			else if (CAST(Signal, ret)->sigtype_ == Signal::Type::BREAK)
			{
				break;
			}
		}
	}

	return Signal::sigNormal();
}

ValuePtr VM::exec(With* w)
{
	exec(w->expr_);
	return exec(w->stmt_);
}

ValuePtr VM::exec(UniExpression* u)
{
	if (u->pre_ && u->op_ == "delete")
	{
		if (u->expr_->type_ == AST::Type::IDENTIFIER)
		{
			u->scope_->delVar(dynamic_cast<Identifier*>(u->expr_)->name_);
			return ValuePtr(new Boolean(true));
		}
		else if (u->expr_->type_ == AST::Type::ARRAY_MEMBER)
		{
			ValuePtr attr = exec(dynamic_cast<ArrayMember*>(u->expr_)->attr_);
			std::string key = attr->toString();

			ValuePtr ref = exec(dynamic_cast<ArrayMember*>(u->expr_)->base_);

			ref->delAttr(key);
			return ValuePtr(new Boolean(true));
		}
		else if (u->expr_->type_ == AST::Type::OBJECT_MEMBER)
		{
			std::string key = dynamic_cast<Identifier*>(dynamic_cast<ObjectMember*>(u->expr_)->attr_)->name_;

			ValuePtr ref = exec(dynamic_cast<ObjectMember*>(u->expr_)->base_);

			ref->delAttr(key);
			return ValuePtr(new Boolean(true));
		}
		else
		{
			return ValuePtr(new Boolean(false));
		}
	}

	auto v = exec(u->expr_);

	if (u->pre_)
	{
		if (u->op_ == "++")
		{
			if (v->type_ == Value::Type::NUMBER)
			{
				if (CAST(Number, v) != nullptr)
				{
					++(CAST(Number, v)->num_);
					return v;
				}
			}

			return NotaNumber::instance();
		}

		if (u->op_ == "--")
		{
			if (v->type_ == Value::Type::NUMBER)
			{
				if (CAST(Number, v) != nullptr)
				{
					--(CAST(Number, v)->num_);
					return v;
				}
			}

			return NotaNumber::instance();
		}

		if (u->op_ == "+")
		{
			return v;
		}

		if (u->op_ == "-")
		{
			if (v->type_ == Value::Type::NUMBER)
			{
				if (CAST(Number, v) != nullptr)
				{
					ValuePtr ret(new Number(-CAST(Number, v)->num_));
					return ret;
				}
			}

			return NotaNumber::instance();
		}

		if (u->op_ == "~")
		{
			int64_t n = 0;

			if (v->type_ == Value::Type::NUMBER)
			{
				if (CAST(Number, v) != nullptr)
				{
					n = int64_t(CAST(Number, v)->num_);
				}
			}

			return ValuePtr(new Number(~n));
		}

		if (u->op_ == "!")
		{
			return ValuePtr(new Boolean(!v->toBool()));
		}

		if (u->op_ == "void")
		{
			return v;
		}

		if (u->op_ == "typeof")
		{
			std::string t = v->typeof();
			return ValuePtr(new StringValue(t));
		}
	}
	else
	{
		if (u->op_ == "++")
		{
			if (v->type_ == Value::Type::NUMBER)
			{
				if (CAST(Number, v) != nullptr)
				{
					ValuePtr ret(new Number(CAST(Number, v)->num_));
					++(CAST(Number, v)->num_);
					return ret;
				}
			}

			return NotaNumber::instance();
		}

		if (u->op_ == "--")
		{
			if (v->type_ == Value::Type::NUMBER)
			{
				if (CAST(Number, v) != nullptr)
				{
					ValuePtr ret(new Number(CAST(Number, v)->num_));
					--(CAST(Number, v)->num_);
					return ret;
				}
			}

			return NotaNumber::instance();
		}
	}

	std::stringstream ss;
	ss << "Can not execute unary-expression at " << u->range_.toString();
	throw ExecError(ss.str());
}

ValuePtr VM::assign(AST* left, ValuePtr v)
{
	if (left->type_ == AST::Type::IDENTIFIER)
	{
		if (left->scope_->getVar(dynamic_cast<Identifier*>(left)->name_) == nullptr)
		{
			global_->setVar(dynamic_cast<Identifier*>(left)->name_, v);
		}
		else
		{
			left->scope_->setVar(dynamic_cast<Identifier*>(left)->name_, v);
		}

		std::cout << "assign " << dynamic_cast<Identifier*>(left)->name_
				<< " = " << v->toString() << std::endl;

		return v;
	}
	else if (left->type_ == AST::Type::ARRAY_MEMBER)
	{
		ValuePtr attr = exec(dynamic_cast<ArrayMember*>(left)->attr_);
		std::string key = attr->toString();

		ValuePtr ref = exec(dynamic_cast<ArrayMember*>(left)->base_);

		if (ref->type_ == Value::Type::UNDEFINED
			|| ref->type_ == Value::Type::NULLVAL)
		{
			std::stringstream ss;
			ss << "Can not set attr [" << key << "] for " << ref->toString()
				<< " at " << left->range_.toString();
			throw ExecError(ss.str());
		}

		ref->setAttr(key, v);
		return v;
	}
	else if (left->type_ == AST::Type::OBJECT_MEMBER)
	{
		std::string key = dynamic_cast<Identifier*>(dynamic_cast<ObjectMember*>(left)->attr_)->name_;
		ValuePtr ref = exec(dynamic_cast<ObjectMember*>(left)->base_);

		if (ref->type_ == Value::Type::UNDEFINED
			|| ref->type_ == Value::Type::NULLVAL)
		{
			std::stringstream ss;
			ss << "Can not set attr [" << key << "] for " << ref->toString()
				<< " at " << left->range_.toString();
			throw ExecError(ss.str());
		}

		ref->setAttr(key, v);
		return v;
	}
	else
	{
		std::stringstream ss;
		ss << "Invalid left value in assignment at "
			<< left->range_.toString();
		throw ExecError(ss.str());
	}
}

ValuePtr VM::plus(ValuePtr left, ValuePtr right)
{
	if (CAST(NotaNumber, left) || CAST(NotaNumber, right))
	{
		return NotaNumber::instance();
	}
	else if (left->type_ == Value::Type::NUMBER
			&& right->type_ == Value::Type::NUMBER)
	{
		return ValuePtr(new Number(CAST(Number, left)->num_
								+ CAST(Number, right)->num_));
	}
	else
	{
		auto s = left->toString() + right->toString();
		return ValuePtr(new StringValue(s));
	}
}

ValuePtr VM::minus(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(CAST(Number, left)->num_
								- CAST(Number, right)->num_));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::mul(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(CAST(Number, left)->num_
								* CAST(Number, right)->num_));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::div(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(CAST(Number, left)->num_
								/ CAST(Number, right)->num_));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::mod(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(int64_t(CAST(Number, left)->num_)
								% int64_t(CAST(Number, right)->num_)));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::band(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(int64_t(CAST(Number, left)->num_)
								& int64_t(CAST(Number, right)->num_)));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::bor(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(int64_t(CAST(Number, left)->num_)
								| int64_t(CAST(Number, right)->num_)));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::bxor(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(int64_t(CAST(Number, left)->num_)
								^ int64_t(CAST(Number, right)->num_)));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::rev(ValuePtr v)
{
	if (v->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, v) == nullptr)
	{
		return ValuePtr(new Number(~int64_t(CAST(Number, v)->num_)));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::lshift(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(int64_t(CAST(Number, left)->num_)
								<< int64_t(CAST(Number, right)->num_)));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::rshift(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Number(int64_t(CAST(Number, left)->num_)
								>> int64_t(CAST(Number, right)->num_)));
	}
	else
	{
		return NotaNumber::instance();
	}
}

ValuePtr VM::eq(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Boolean(CAST(Number, left)->num_
								== CAST(Number, right)->num_));
	}
	else
	{
		return ValuePtr(new Boolean(left->toString() == right->toString()));
	}
}

ValuePtr VM::neq(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Boolean(CAST(Number, left)->num_
								!= CAST(Number, right)->num_));
	}
	else
	{
		return ValuePtr(new Boolean(left->toString() != right->toString()));
	}
}

ValuePtr VM::ls(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Boolean(CAST(Number, left)->num_
								< CAST(Number, right)->num_));
	}
	else
	{
		return ValuePtr(new Boolean(left->toString() < right->toString()));
	}
}

ValuePtr VM::le(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Boolean(CAST(Number, left)->num_
								<= CAST(Number, right)->num_));
	}
	else
	{
		return ValuePtr(new Boolean(left->toString() <= right->toString()));
	}
}

ValuePtr VM::gt(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Boolean(CAST(Number, left)->num_
								> CAST(Number, right)->num_));
	}
	else
	{
		return ValuePtr(new Boolean(left->toString() > right->toString()));
	}
}

ValuePtr VM::ge(ValuePtr left, ValuePtr right)
{
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Boolean(CAST(Number, left)->num_
								>= CAST(Number, right)->num_));
	}
	else
	{
		return ValuePtr(new Boolean(left->toString() >= right->toString()));
	}
}

ValuePtr VM::teq(ValuePtr left, ValuePtr right)
{
	if (left->type_ != right->type_)
	{
		return ValuePtr(new Boolean(false));
	}
	if (left->type_ == Value::Type::NUMBER
		&& right->type_ == Value::Type::NUMBER
		&& CAST(NotaNumber, left) == nullptr
		&& CAST(NotaNumber, right) == nullptr)
	{
		return ValuePtr(new Boolean(CAST(Number, left)->num_
								== CAST(Number, right)->num_));
	}
	else
	{
		return ValuePtr(new Boolean(left->toString() == right->toString()));
	}
}

ValuePtr VM::nteq(ValuePtr left, ValuePtr right)
{
	auto r = teq(left, right);
	return ValuePtr(new Boolean(!CAST(Boolean, r)->b_));
}

ValuePtr VM::exec(BiExpression* bi)
{
	if (bi->op_ == "&&")
	{
		auto b = exec(bi->left_)->toBool() && exec(bi->right_)->toBool();
		return ValuePtr(new Boolean(b));
	}

	if (bi->op_ == "||")
	{
		auto b = exec(bi->left_)->toBool() || exec(bi->right_)->toBool();
		return ValuePtr(new Boolean(b));
	}

	ValuePtr rval = exec(bi->right_);

	if (bi->op_ == "=")
	{
		return assign(bi->left_, rval);
	}

	ValuePtr lval = exec(bi->left_);

	if (bi->op_ == "+=")
	{
		return assign(bi->left_, plus(lval, rval));
	}

	if (bi->op_ == "-=")
	{
		return assign(bi->left_, minus(lval, rval));
	}

	if (bi->op_ == "*=")
	{
		return assign(bi->left_, mul(lval, rval));
	}

	if (bi->op_ == "/=")
	{
		return assign(bi->left_, div(lval, rval));
	}

	if (bi->op_ == "%=")
	{
		return assign(bi->left_, mod(lval, rval));
	}

	if (bi->op_ == "&=")
	{
		return assign(bi->left_, band(lval, rval));
	}

	if (bi->op_ == "|=")
	{
		return assign(bi->left_, bor(lval, rval));
	}

	if (bi->op_ == "~=")
	{
		return assign(bi->left_, rev(rval));
	}

	if (bi->op_ == "^=")
	{
		return assign(bi->left_, bxor(lval, rval));
	}

	if (bi->op_ == "<<=")
	{
		return assign(bi->left_, lshift(lval, rval));
	}

	if (bi->op_ == ">>=")
	{
		return assign(bi->left_, rshift(lval, rval));
	}

	if (bi->op_ == "+")
	{
		return plus(lval, rval);
	}

	if (bi->op_ == "-")
	{
		return minus(lval, rval);
	}

	if (bi->op_ == "*")
	{
		return mul(lval, rval);
	}

	if (bi->op_ == "/")
	{
		return div(lval, rval);
	}

	if (bi->op_ == "%")
	{
		return mod(lval, rval);
	}

	if (bi->op_ == "&")
	{
		return band(lval, rval);
	}

	if (bi->op_ == "|")
	{
		return bor(lval, rval);
	}

	if (bi->op_ == "^")
	{
		return bxor(lval, rval);
	}

	if (bi->op_ == "<<")
	{
		return lshift(lval, rval);
	}

	if (bi->op_ == ">>")
	{
		return rshift(lval, rval);
	}

	if (bi->op_ == "<")
	{
		return ls(lval, rval);
	}

	if (bi->op_ == "<=")
	{
		return le(lval, rval);
	}

	if (bi->op_ == ">")
	{
		return gt(lval, rval);
	}

	if (bi->op_ == ">=")
	{
		return ge(lval, rval);
	}

	if (bi->op_ == "==")
	{
		return eq(lval, rval);
	}

	if (bi->op_ == "!=")
	{
		return neq(lval, rval);
	}

	if (bi->op_ == "===")
	{
		return teq(lval, rval);
	}

	if (bi->op_ == "!==")
	{
		return nteq(lval, rval);
	}

	std::stringstream ss;
	ss << "Can not execute binary-expression at " << bi->range_.toString();
	throw ExecError(ss.str());
}

ValuePtr VM::exec(TriExpression* tri)
{
	bool check = exec(tri->cond_)->toBool();
	if (check)
	{
		return exec(tri->yes_);
	}
	else
	{
		return exec(tri->no_);
	}
}

// Try
// Throw
// LiteralRegular

void VM::loadBuiltin()
{
	global_->setVar("undefined", Undefined::instance());
	// global_->setVar("window", ValuePtr(new ObjectValue()));
}

NAMESPACE_END