default:
	gcc -h

test:
	gcc -g bson.c test_bson.c -o test_bson

fuzz:
	afl-clang bson.c fuzz_bson.c -o bson_fuzzer

fuzzrun:
	afl-fuzz -i testcases -o findings -- ./bson_fuzzer
