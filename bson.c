#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bson.h"

enum bson_type_id {
    BSON_DOUBLE = 0x01,
    BSON_STRING = 0x02,
    BSON_DOCUMENT = 0x03,
    BSON_ARRAY = 0x04,
    BSON_BINARY = 0x05,
    BSON_OBJECTID = 0x07,
    BSON_BOOLEAN = 0x08,
    BSON_UTC = 0x09,
    BSON_NULL = 0x0A,
    BSON_REGEX = 0x0B,
    BSON_JSCODE = 0x0D,
    BSON_JSCODEWSCOPE = 0x0F,
    BSON_INT32 = 0x10,
    BSON_TIMESTAMP = 0x11,
    BSON_INT64 = 0x12,
    BSON_DEC128 = 0x13,
    BSON_MINKEY = 0xFF,
    BSON_MAXKEY = 0x7F
};

bson_t* bson_decode(char* buf){
    bson_t* obj = malloc(sizeof(bson_t));

    // Decode little endian int32 document length
    obj->data_len = 0;
    obj->data_len  |= buf[0];
    obj->data_len  |= buf[1]<<8;
    obj->data_len  |= buf[2]<<16;
    obj->data_len  |= buf[3]<<24;

    if(obj->data_len < 4){
        assert("Invalid BSON document size." == NULL);
    }

    obj->data = (char*) malloc(obj->data_len);
    memcpy(obj->data, buf, obj->data_len);

    return obj;
    
}

char* bson_encode(bson_t* obj, int* len){
    char* data = (char*) malloc(obj->data_len);
    memcpy(data, obj->data, obj->data_len);
    *len = obj->data_len;
    return data;
}

/* 
 * Given a buffer pointed to by buf, decode the first 4 bytes as a little endian 32-bit
 * integer, and return that value as an int.
 */
int decode_little_endian_int32(char* buf){
    int val = 0;
    memcpy(&val, buf, 4);
    return val;
}

/* 
 * Given a buffer pointed to by buf, decode the first 8 bytes as a little endian 64-bit
 * floating point, and return that value as a double.
 */
double decode_little_endian_double(char* buf){
    double val = 0;
    memcpy(&val, buf, 8);
    return val;
}

/*
 * Given a starting position, pointing to an element in a raw BSON byte array, find the next element 
 * whose element name matches @key, and return a pointer to the element value's position in the byte 
 * array. If no element with given key name is found, returns NULL.
 */
char* _bson_find_next_by_key(char* start_pos, char* key){
    
    char* arr_pos = start_pos;
    int end_of_doc = 0;
    int matched = -1;
    int len = 0;
    
    while(!end_of_doc){
        enum bson_type_id elem_type = *arr_pos;

        //Check e_name
        matched = strcmp(arr_pos+1, key);

        if(matched==0){
            return arr_pos;
        }
        
        /* 
         * Move the pointer to the next document in byte array. The number of bytes to skip 
         * ahead is dependent on the element type, so we need specific skip-ahead logic 
         * for each  type.
        */    

        // Skip element type byte.
        arr_pos+=1;

        // Skip e_name bytes.
        int e_name_len = strlen(arr_pos);
        arr_pos += (e_name_len+1);

        // Skip element value bytes (type-dependent).
        switch(elem_type){

            case BSON_DOUBLE:
                //double = 8 bytes
                arr_pos += 8;
                break;

            case BSON_STRING:
                // string  ::= int32 (byte*) "\x00"
                len = decode_little_endian_int32(arr_pos);
                arr_pos += (4+len);
                break;

            case BSON_DOCUMENT:
            case BSON_ARRAY:
                len = decode_little_endian_int32(arr_pos);
                arr_pos += len;
                break;

            case BSON_BINARY:
                len = decode_little_endian_int32(arr_pos);
                arr_pos += len;
                break;

            case BSON_OBJECTID:
                // ObjectId: "\x07" e_name (byte*12)
                arr_pos += 12;
                break;

            case BSON_BOOLEAN:
                arr_pos += 1;
                break;

            case BSON_UTC:
                //UTC Timestamp: int64 = 8 bytes
                arr_pos += 8;
                break;

            case BSON_NULL:
                break;

            case BSON_REGEX:
                //  Regex: "\x0B" e_name cstring cstring
                arr_pos += (strlen(arr_pos)+1);
                arr_pos += (strlen(arr_pos)+1);
                break;

            case BSON_JSCODE:
                len = decode_little_endian_int32(arr_pos);
                arr_pos += (4+len);
                break;

            case BSON_JSCODEWSCOPE:
                len = decode_little_endian_int32(arr_pos);
                arr_pos += (4+len);
                break;

            case BSON_INT32:
                //int32 = 4 bytes
                arr_pos += 4;
                break;

            case BSON_TIMESTAMP:
                arr_pos += 8;

            case BSON_INT64:
                arr_pos += 8;

            case BSON_DEC128:
                arr_pos += 16;

            case BSON_MINKEY:
            case BSON_MAXKEY:
                break;

            default:
                assert("BSON type unknown!" == NULL);
        }
    } 

    if(matched!=0){
        return NULL;
    }

    return arr_pos; 
}


char* 
bson_get_val_string(bson_t* obj, char* key, err_t* err, int* len){
    /*
     *   BSON Spec:
     *   string  ::= int32 (byte*) "\x00"
    */

    char* elem_pos = _bson_find_next_by_key(obj->data + 4, key);

    // Check if found.
    if(elem_pos==NULL){
        strcpy(err->msg, "No key found.");
        return NULL;
    }

    // Check if correct BSON type.
    if(elem_pos[0]!=BSON_STRING){
        strcpy(err->msg, "Unable to decode element as string.");
        return NULL;
    }

    // Skip e_name bytes
    elem_pos += (strlen(elem_pos)+1);

    int str_len = decode_little_endian_int32(elem_pos);

    // Move forward to raw string bytes
    elem_pos+=4;

    char* out_str = (char*) malloc(str_len+1);
    memcpy(out_str, elem_pos, str_len+1);
    *len = str_len;

    //Start at beginning of BSON doc, past length header
    return out_str;
}

double
bson_get_val_double(bson_t* obj, char* key, err_t* err){

    char* elem_pos = _bson_find_next_by_key(obj->data + 4, key);

    // Check if found.
    if(elem_pos==NULL){
        strcpy(err->msg, "No key found.");
        return -1;
    }

    // Check if correct BSON type.
    if(elem_pos[0]!=BSON_DOUBLE){
        strcpy(err->msg, "Unable to decode element as double.");
        return -1;
    }


    // Skip e_name bytes
    elem_pos += (strlen(elem_pos)+1);

    return decode_little_endian_double(elem_pos);
}

int
bson_get_val_int32(bson_t* obj, char* key, err_t* err){

    char* elem_pos = _bson_find_next_by_key(obj->data + 4, key);

    // Check if found. 
    if(elem_pos==NULL){
        strcpy(err->msg, "No key found.");
        return -1;
    }

    // Check if correct BSON type. 
    if(elem_pos[0]!=BSON_INT32){
        strcpy(err->msg, "Unable to decode element as int32.");
        return -1;
    }

    // Skip e_name bytes
    elem_pos += (strlen(elem_pos)+1);

    return decode_little_endian_int32(elem_pos);
}

bson_t*
bson_get_val_document(bson_t* obj, char* key, err_t* err){

    char* elem_pos = _bson_find_next_by_key(obj->data + 4, key);

    // Check if found.
    if(elem_pos==NULL){
        strcpy(err->msg, "No key found.");
        return NULL;
    }

    // Check if correct BSON type. 
    if(elem_pos[0]!=BSON_DOCUMENT){
        strcpy(err->msg, "Unable to decode element as document.");
        return NULL;
    }

    // Skip e_name bytes
    elem_pos += (strlen(elem_pos)+1);
    err->err_set = 0;

    return bson_decode(elem_pos);
}

int
bson_get_val_boolean(bson_t* obj, char* key, err_t* err){

    char* elem_pos = _bson_find_next_by_key(obj->data + 4, key);

    // Check if found. 
    if(elem_pos==NULL){
        strcpy(err->msg, "No key found.");
        return -1;
    }

    // Check if correct BSON type. 
    if(elem_pos[0]!=BSON_BOOLEAN){
        strcpy(err->msg, "Unable to decode element as boolean.");
        return -1;
    }

    // Skip e_name bytes
    elem_pos += (strlen(elem_pos)+1);

    return (int)(*elem_pos);
}






