default:
	gcc -h

test:
	gcc bson.c test_bson.c -o test_bson