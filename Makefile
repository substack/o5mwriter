all: test

test-code: test/test.c o5mwriter.h
	@g++ -I. test/test.c -o test/test

test: test-code
	@test/test.sh
