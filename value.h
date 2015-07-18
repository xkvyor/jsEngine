#ifndef _VALUE_H_
#define _VALUE_H_

#include "common.h"
#include "ast.h"

NAMESPACE_BEGIN

class Undefined;
class Value;

typedef std::shared_ptr<Value> ValuePtr;

class Value {
public:
	enum Type {
		UNDEFINED,
		NULLVAL,
		BOOL,
		NUMBER,
		STRING,
		OBJECT,
		FUNCTION,
		SIGNAL
	};

	Type type_;
	std::unordered_map<std::string, ValuePtr> attr_;

	Value(Type type): type_(type)
	{}
	virtual ~Value()
	{}

	void setAttr(const std::string& key, ValuePtr v);
	ValuePtr getAttr(const std::string& key);
	void delAttr(const std::string& key);
	std::vector<std::string> getKeys();

	virtual std::string toString() = 0;
	virtual bool toBool() = 0;
	virtual std::string typeof() = 0;
};

class Undefined: public Value {
private:
	Undefined(): Value(Value::Type::UNDEFINED)
	{}
public:
	static ValuePtr instance()
	{
		static ValuePtr inst(new Undefined());
		return inst;
	}
	std::string toString()
	{
		return "undefined";
	}
	bool toBool()
	{
		return false;
	}
	std::string typeof()
	{
		return "undefined";
	}
};

class Boolean: public Value {
public:
	bool b_;

public:
	Boolean(bool b): Value(Value::Type::BOOL), b_(b)
	{}
	std::string toString()
	{
		return b_ ? "true" : "false";
	}
	bool toBool()
	{
		return b_;
	}
	std::string typeof()
	{
		return b_ ? "true" : "false";
	}
};

class NotaNumber: public Value {
private:
	NotaNumber(): Value(Value::Type::NUMBER)
	{}
public:
	static ValuePtr instance()
	{
		static ValuePtr inst(new NotaNumber());
		return inst;
	}
	std::string toString()
	{
		return "NaN";
	}
	bool toBool()
	{
		return false;
	}
	std::string typeof()
	{
		return "number";
	}
};

class Number: public Value {
public:
	double num_;

public:
	Number(double n): Value(Value::Type::NUMBER), num_(n)
	{}
	Number(int64_t n): Value(Value::Type::NUMBER), num_(double(n))
	{}
	std::string toString()
	{
		return std::to_string(num_);
	}
	bool toBool()
	{
		return num_ != 0.0;
	}
	std::string typeof()
	{
		return "number";
	}
};

class StringValue: public Value {
public:
	std::string str_;

public:
	StringValue(const std::string& str): Value(Value::Type::STRING), str_(str)
	{}
	std::string toString()
	{
		return str_;
	}
	bool toBool()
	{
		return str_ != "";
	}
	std::string typeof()
	{
		return "string";
	}
};

class ObjectValue: public Value {
public:
	ObjectValue(): Value(Value::Type::OBJECT)
	{}
	std::string toString()
	{
		return "[object Object]";
	}
	bool toBool()
	{
		return true;
	}
	std::string typeof()
	{
		return "object";
	}
};

class NullValue: public Value {
private:
	NullValue(): Value(Value::Type::NULLVAL)
	{}
public:
	static ValuePtr instance()
	{
		static ValuePtr inst(new NullValue());
		return inst;
	}
	std::string toString()
	{
		return "";
	}
	bool toBool()
	{
		return false;
	}
	std::string typeof()
	{
		return "object";
	}
};

class FunctionValue: public Value {
public:
	Function* code_;

public:
	FunctionValue(Function* code): Value(Value::Type::FUNCTION), code_(code)
	{}
	std::string toString()
	{
		return "function";
	}
	bool toBool()
	{
		return true;
	}
	std::string typeof()
	{
		return "function";
	}
};

class Signal: public Value {
public:
	enum Type {
		BREAK,
		CONTINUE,
		NORMAL,
		RETURN
	};
	Type sigtype_;
	Position pos_;
	ValuePtr val_;

private:
	Signal(Type sigtype): Value(Value::Type::SIGNAL), sigtype_(sigtype), val_(nullptr)
	{}

public:
	~Signal()
	{}
	std::string toString()
	{
		return "[built-in]";
	}
	bool toBool()
	{
		return false;
	}

	static ValuePtr sigBreak(Break* b)
	{
		static ValuePtr brk(new Signal(Type::BREAK));
		dynamic_cast<Signal*>(brk.get())->pos_ = b->range_.begin_;
		return brk;
	}

	static ValuePtr sigContinue(Continue* c)
	{
		static ValuePtr con(new Signal(Type::CONTINUE));
		dynamic_cast<Signal*>(con.get())->pos_ = c->range_.begin_;
		return con;
	}

	static ValuePtr sigNormal()
	{
		static ValuePtr nor(new Signal(Type::NORMAL));
		return nor;
	}

	static ValuePtr sigReturn(ValuePtr val)
	{
		Signal* inst = new Signal(Type::RETURN);
		inst->val_ = val;
		static ValuePtr ret(inst);
		return ret;
	}

	std::string typeof()
	{
		return "built-in";
	}
};

class Scope {
private:
	std::unordered_map<std::string, ValuePtr> vars_;
	Scope* parent_;

public:
	Scope(Scope* p): parent_(p)
	{}
	~Scope()
	{}

	ValuePtr getVar(const std::string& name)
	{
		Scope* cur = this;
		while (cur)
		{
			auto r = cur->vars_.find(name);
			if (r != cur->vars_.end())
			{
				return r->second;
			}
			cur = cur->parent_;
		}
		return nullptr;
	}

	void setVar(const std::string& name, ValuePtr val)
	{
		Scope* cur = this;
		while (cur)
		{
			auto r = cur->vars_.find(name);
			if (r != cur->vars_.end())
			{
				r->second = val;
				return;
			}
			cur = cur->parent_;
		}
		vars_[name] = val;
	}

	void delVar(const std::string& name)
	{
		Scope* cur = this;
		while (cur)
		{
			auto r = cur->vars_.find(name);
			if (r != cur->vars_.end())
			{
				cur->vars_.erase(name);
				return;
			}
			cur = cur->parent_;
		}
	}

	inline Scope* getParent() { return parent_; }
	inline std::unordered_map<std::string, ValuePtr>& getValueMap() { return vars_; }
};

NAMESPACE_END

#endif