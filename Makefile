CXX = g++
CXXFLAGS = -std=c++11 -g -pg -Wall

all: test

value.o:
	$(CXX) $(CXXFLAGS) -c value.cpp -o $@

lexer.o:
	$(CXX) $(CXXFLAGS) -c lexer.cpp -o $@

parser.o:
	$(CXX) $(CXXFLAGS) -c parser.cpp -o $@

vm.o:
	$(CXX) $(CXXFLAGS) -c vm.cpp -o $@

test: value.o lexer.o parser.o vm.o
	$(CXX) $(CXXFLAGS) value.o lexer.o parser.o vm.o test.cpp -o $@

clean:
	rm -f *.o
	rm -f test