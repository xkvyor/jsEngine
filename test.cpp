#include <iostream>
#include <fstream>

#include "vm.h"

using namespace cl;
using namespace std;

void displayLexer(Lexer* lex)
{
	lex->restart();

	while (lex->peek().type_ != Token::Type::END_OF_FILE)
	{
		cout << lex->get().toString() << endl;
	}
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		return 1;
	}

	ifstream f(argv[1]);
	string source;
	char buf[4096];
	size_t rb;

	while ((rb = f.readsome(buf, 4096)) > 0)
	{
		source += string(buf, rb);
	}

	auto lex = new Lexer(source);

	// {
	// 	displayLexer(lex);
	// 	return 0;
	// }

	auto ps = new Parser(lex);

	auto vm = new VM();

	vm->exec(ps->getProgram());

	return 0;
}