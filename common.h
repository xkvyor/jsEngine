#ifndef _COMMON_H_
#define _COMMON_H_

#define NAMESPACE_BEGIN namespace cl {
#define NAMESPACE_END }

#include <string>
#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <sstream>
#include <exception>
#include <iostream>

NAMESPACE_BEGIN

template<typename T>
inline void deletePtr(T& p)
{
	delete p;
	p = NULL;
}

template<typename T>
inline void deleteArray(T& p)
{
	delete [] p;
	p = NULL;
}

struct Position {
	int line_, col_;
	Position(): line_(0), col_(0)
	{}
	Position(int l, int c): line_(l), col_(c)
	{}
	std::string toString()
	{
		std::stringstream ss;
		ss << line_ << ":" << col_;
		return ss.str();
	}
};

struct PositionRange {
	Position begin_, end_;
	PositionRange(const Position& begin, const Position& end):
		begin_(begin), end_(end)
	{}
	std::string toString()
	{
		std::stringstream ss;
		ss << begin_.toString() << "-" << end_.toString();
		return ss.str();
	}
};

NAMESPACE_END

#endif