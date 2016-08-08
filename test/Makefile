all: test

test-code: test.c ../o5mwriter.h
	@g++ -I ../ test.c -o test

test: test-code
	@./test.sh
