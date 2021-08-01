src = main.cpp
exe = calculator
all: $(exe)
	./$< "123"
	./$< "   "
	./$< "123 + 456"
	./$< "123 + 5 * 2"

$(exe): $(src)
	g++ main.cpp -std=c++11 -o $@
