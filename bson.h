#include <stdio.h>

typedef struct{
    char* data;
    int data_len;
} bson_t;

typedef struct{
    char* msg;
    int err_set;
} err_t;


/* 
 * Serializes a raw data buffer into a BSON object. Returns NULL on failure.
 */
bson_t* bson_decode(char* buf, int len, err_t* err);


/* 
 * Takes a BSON object and returns a buffer containing the raw BSON object bytes. Sets @len to
 * the size of the returned buffer. Memory for this buffer is owned by the callee.
 */
char* bson_encode(bson_t* obj, int* len);


/*
 * Takes a BSON object and returns its representation as a JSON string. Returns NULL and will set @err
 * if any elements are unable to converted to a string representation.
 */
char* bson_to_json(bson_t* obj, err_t* err);

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

int
bson_get_val_null(bson_t* obj, char* key, err_t* err);

long
bson_get_val_utc(bson_t* obj, char* key, err_t* err);

long
bson_get_val_int64(bson_t* obj, char* key, err_t* err);

