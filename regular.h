#ifndef _REGULAR_H_
#define _REGULAR_H_

#include "common.h"

NAMESPACE_BEGIN

using std::string;

class RegularExpression {
private:
	string pattern_;

public:
	RegularExpression(const char* pattern);
	RegularExpression(const string& pattern);
};

NAMESPACE_END

#endif