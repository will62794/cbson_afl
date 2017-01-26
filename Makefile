test:
	gcc -g bson.c test_bson.c -o test_bson

fuzz:
	afl-clang bson.c fuzz_bson.c -o bson_fuzzer

fuzzdebug:
	gcc -O0 -g bson.c fuzz_bson.c -o bson_fuzzer_debug

fuzzrun:
	afl-fuzz -i testcases -o findings -- ./bson_fuzzer
