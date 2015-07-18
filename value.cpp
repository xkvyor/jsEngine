#include "value.h"

NAMESPACE_BEGIN

void Value::setAttr(const std::string& key, ValuePtr v)
{
	std::cout << "set " << key << " = " << v->toString() << std::endl;
	attr_[key] = v;
}

ValuePtr Value::getAttr(const std::string& key)
{
	if (attr_.find(key) != attr_.end())
	{
		return attr_[key];
	}
	return Undefined::instance();
}

void Value::delAttr(const std::string& key)
{
	if (attr_.find(key) != attr_.end())
	{
		attr_.erase(key);
	}
}

std::vector<std::string> Value::getKeys()
{
	std::vector<std::string> ret;

	for (auto i : attr_)
	{
		ret.push_back(i.first);
	}

	std::sort(ret.begin(), ret.end());

	return ret;
}

NAMESPACE_END