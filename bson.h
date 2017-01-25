#include <stdio.h>

typedef struct{
    char* data;
    int data_len;
} bson_t;

typedef struct{
    char msg[1024];
    int err_set;
} err_t;


/* 
 * Serializes a raw data buffer into a BSON object 
 */
bson_t* bson_decode(char* buf);

/* 
 * Takes a BSON object and returns a buffer containing the raw BSON object bytes. Sets @len to
 * the size of the returned buffer. Memory for this buffer is owned by the callee.
 */
char* bson_encode(bson_t* obj, int* len);


char* 
bson_get_val_string(bson_t* obj, char* key, err_t* err, int* len);

double 
bson_get_val_double(bson_t* obj, char* key, err_t* err);

int
bson_get_val_int32(bson_t* obj, char* key, err_t* err);

bson_t*
bson_get_val_document(bson_t* obj, char* key, err_t* err);

int
bson_get_val_boolean(bson_t* obj, char* key, err_t* err);
