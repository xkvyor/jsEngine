#include "lexer.h"

NAMESPACE_BEGIN

static bool isDigit(char c, int base = 10)
{
	if (base <= 10)
	{
		return c >= '0' && c <= '0' + base - 1;
	}
	else
	{
		return (c >= '0' && c <= '9')
			|| (c >= 'a' && c < 'a' + base - 10)
			|| (c >= 'A' && c < 'A' + base - 10);
	}
}

static bool isUpperLetter(char c)
{
	return c >= 'A' && c <= 'Z';
}

static bool isLowerLetter(char c)
{
	return c >= 'a' && c <= 'z';
}

static bool isLetter(char c)
{
	return isUpperLetter(c) || isLowerLetter(c);
}

static bool isIdentifierFirst(char c)
{
	return isLetter(c) || c == '_' || c == '$';
}

static bool isIdentifier(char c)
{
	return isLetter(c) || isDigit(c) || c == '_' || c == '$';
}

Lexer::Lexer(const std::string& source)
{
	restart();
	process(source);
}

Lexer::~Lexer()
{}

static std::string escape(const std::string& s)
{
	std::string ret;
	int n = s.length();
	ret.resize(n, '\0');
	for (int i = 0, p = 0; i < n; ++i, ++p)
	{
		if (i+1 < n && s[i] == '\\')
		{
			switch (s[i+1])
			{
				case '\'':
					ret[p] = '\'';
					++i;
					break;
				case '\"':
					ret[p] = '\"';
					++i;
					break;
				case '\\':
					ret[p] = '\\';
					++i;
					break;
				case 'n':
					ret[p] = '\n';
					++i;
					break;
				case 'r':
					ret[p] = '\r';
					++i;
					break;
				case 't':
					ret[p] = '\t';
					++i;
					break;
				case 'b':
					ret[p] = '\b';
					++i;
					break;
				case 'f':
					ret[p] = '\f';
					++i;
					break;
			}
		}
		else
		{
			ret[p] = s[i];
		}
	}
	return ret;
}

void Lexer::process(const std::string& source)
{
	int cur = 0;
	int forward = cur;
	int end = source.length();

	int line = 1;
	int col = 1;

	while (cur < end)
	{
		char c = source[cur];
		Position begin(line, col);
		forward = cur + 1;
		++col;

		if (isIdentifierFirst(c))
		{
			while (forward < end && isIdentifier(source[forward]))
			{
				++forward;
				++col;
			}

			Position end(line, col);
			std::string data = source.substr(cur, forward-cur);
			Token::Type type = Token::Type::IDENTIFIER;

			if (KEYWORDS.find(data) != KEYWORDS.end())
			{
				type = Token::Type::KEYWORD;
			}

			tokens_.push_back(Token(type, data, PositionRange(begin, end)));
		}
		else if (c == '"' || c == '\'')
		{
			while (forward < end && source[forward] != c)
			{
				if (source[forward] == '\n')
				{
					++line;
					col = 1;
				}
				else
				{
					++col;
				}
				if (source[forward] == '\\')
				{
					++forward;
				}
				++forward;
			}

			Position end(line, col);
			std::string data = escape(source.substr(cur, forward-cur+1));
			++forward;

			Token::Type type = Token::Type::STRING;

			tokens_.push_back(Token(type, data, PositionRange(begin, end)));
		}
		else if (c == '/' && source[forward] != '/' && source[forward] != '*'
			&& (tokens_.size() == 0 || (
				tokens_.rbegin()->type_ != Token::Type::IDENTIFIER
				&& tokens_.rbegin()->type_ != Token::Type::NUMBER
				&& tokens_.rbegin()->type_ != Token::Type::STRING
				&& tokens_.rbegin()->type_ != Token::Type::REGULAR
				&& tokens_.rbegin()->type_ != Token::Type::KEYWORD
				&& tokens_.rbegin()->data_ != ")")))
		{
			while (forward < end && source[forward] != c)
			{
				if (source[forward] == '\n')
				{
					++line;
					col = 1;
				}
				else
				{
					++col;
				}
				if (source[forward] == '\\')
				{
					++forward;
				}
				++forward;
			}

			Position end(line, col);
			std::string data = source.substr(cur, forward-cur+1);
			++forward;

			while (isLetter(source[forward]))
			{
				++forward;
			}

			Token::Type type = Token::Type::REGULAR;

			tokens_.push_back(Token(type, data, PositionRange(begin, end)));
		}
		else if (c == '/' && source[forward] == '/')
		{
			while (forward < end && source[forward] != '\n')
			{
				++forward;
				++col;
			}
		}
		else if (c == '/' && source[forward] == '*')
		{
			++forward;
			++col;
			while (forward < end)
			{
				if (source[forward] == '\n')
				{
					++line;
					col = 1;
				}
				else if (source[forward] == '*')
				{
					if (forward + 1 < end && source[forward+1] == '/')
					{
						forward += 2;
						break;
					}
				}
				++forward;
			}
		}
		else if (isDigit(c))
		{
			int base = 10;

			if (c == '0')
			{
				switch (source[forward])
				{
					case 'X':
					case 'x':
						base = 16;
						break;
					case 'B':
					case 'b':
						base = 2;
						break;
					case 'O':
					case 'o':
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
						base = 8;
						break;
					default:
						--forward;
				}
				++forward;
			}

			while (isDigit(source[forward], base))
			{
				++forward;
			}

			if (source[forward] == '.')
			{
				++forward;
				while (isDigit(source[forward], base))
				{
					++forward;
				}
			}

			if (source[forward] == 'e' || source[forward] == 'E')
			{
				++forward;
				if (source[forward] == '+' || source[forward] == '-')
				{
					while (isDigit(source[forward], base))
					{
						++forward;
					}
				}
			}

			Position end(line, col);
			std::string data = source.substr(cur, forward-cur);

			Token::Type type = Token::Type::NUMBER;

			tokens_.push_back(Token(type, data, PositionRange(begin, end)));
		}
		else if (c == '\n')
		{
			++line;
			col = 1;
		}
		else if (TOKEN_MAP.find(source.substr(cur, 1)) != TOKEN_MAP.end())
		{
			while (forward < end &&
				TOKEN_MAP.find(source.substr(cur, forward - cur + 1)) != TOKEN_MAP.end())
			{
				++forward;
				++col;
			}

			Position end(line, col);
			std::string data = source.substr(cur, forward-cur);
			Token::Type type = TOKEN_MAP.find(data)->second;

			tokens_.push_back(Token(type, data, PositionRange(begin, end)));
		}

		cur = forward;
	}

	tokens_.push_back(Token(Token::Type::END_OF_FILE, "",
		PositionRange(Position(line, col), Position(line, col))));
}

void Lexer::restart()
{
	iter_ = tokens_.begin();
}

void Lexer::clear()
{
	tokens_.clear();
	restart();
}

Token Lexer::get()
{
	return *iter_++;
}

Token Lexer::peek()
{
	return *iter_;
}

bool Lexer::end()
{
	return iter_ == tokens_.end();
}

NAMESPACE_END
