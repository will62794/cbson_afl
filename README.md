# C BSON

My own toy implementation of BSON in C. Intended mainly as a programming/design exercise, as well as to demonstrate a simple usage of AFL (American Fuzzy Lop). Roughly based on the MongoDB C Driver. Provides an interface to decode/encode BSON objects and a simple way to retrieve values from a BSON object by element name/key. Depending on how much more work I've put into it, only certain BSON types are supported. 

To compile the AFL fuzzing harness, simply run

`make fuzz`

And to spin up the AFL fuzzer, do

`make fuzzrun`

Note, you will need to have AFL installed in order to run the above targets. You can get it here [http://lcamtuf.coredump.cx/afl/]. Happy fuzzing!