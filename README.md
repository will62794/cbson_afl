# C BSON with AFL Fuzzing Demo

My own implementation of BSON in C. Intended mainly as a programming/design exercise, as well as to demonstrate a simple usage of AFL (American Fuzzy Lop). Roughly based on the MongoDB C Driver. Provides an interface to decode/encode BSON objects and a simple way to retrieve values from a BSON object by element name/key. At this point, only the following basic BSON element types are supported for key value retrieval and BSON->JSON conversion: {double, string, document, int32, boolean}. Strings are treated as ASCII. UTF-8 decoding is not supported yet.

To compile the AFL fuzzing harness, simply run

`make fuzz`

And to spin up the AFL fuzzer, do

`make fuzzrun`

Note, you will need to have AFL installed in order to run the above targets. You can get it here [http://lcamtuf.coredump.cx/afl/]. Happy fuzzing!