src = main.cpp
exe = calculator
all: $(exe)

$(exe): $(src)
	g++ main.cpp -std=c++11 -g -o $@

test: $(exe)
	@./test.sh
