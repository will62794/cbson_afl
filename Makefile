test:
	gcc -g bson.c test_bson.c -o test_bson

fuzz:
	afl-clang bson.c fuzz_bson.c -o fuzz_bson;
	gcc -O0 -g bson.c fuzz_bson.c -o fuzz_bson_debug

fuzzrun:
	afl-fuzz -i testcases -o findings -- ./fuzz_bson
