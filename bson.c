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

typedef struct{
    enum bson_type_id type_id;
    char* e_name;
    char* data;
} bson_elem_t;


typedef struct{
    bson_elem_t curr_elem;
} bson_iter_t;


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

bson_t* bson_decode(char* buf){
    bson_t* obj = malloc(sizeof(bson_t));

    // Decode little endian int32 document length
    obj->data_len = decode_little_endian_int32(buf);

    if(obj->data_len < 4){
        assert("Invalid BSON document size." == NULL);
    }

    obj->data = (char*) malloc(obj->data_len);
    memcpy(obj->data, buf, obj->data_len);

    return obj;
    
}

char* 
bson_encode(bson_t* obj, int* len){
    char* data = (char*) malloc(obj->data_len);
    memcpy(data, obj->data, obj->data_len);
    *len = obj->data_len;
    return data;
}

int
_bson_element_size_bytes(bson_elem_t* element){

    // "\x<TYPE>" e_name
    int header_size = strlen(element->e_name)+2;
    int len;

    // Element data size (type-dependent).
    switch(element->type_id){

        case BSON_DOUBLE:
            //double = 8 bytes
            return header_size + 8;
            break;

        case BSON_STRING:
            // string  ::= int32 (byte*) "\x00"
            len = decode_little_endian_int32(element->data);
            return header_size + len + 4;
            break;

        case BSON_DOCUMENT:
        case BSON_ARRAY:
            len = decode_little_endian_int32(element->data);
            return header_size + len;
            break;

        case BSON_BINARY:
            len = decode_little_endian_int32(element->data);
            return header_size + len;
            break;

        case BSON_OBJECTID:
            // ObjectId: "\x07" e_name (byte*12)
            return header_size + 12;
            break;

        case BSON_BOOLEAN:
            return header_size + 1;
            break;

        case BSON_UTC:
            //UTC Timestamp: int64 = 8 bytes
            return header_size + 8;
            break;

        case BSON_NULL:
            break;

        case BSON_REGEX:
            //  Regex: "\x0B" e_name cstring cstring
            len = strlen(element->data)+1;
            return header_size + len + (strlen(element->data + len)+1);
            break;

        case BSON_JSCODE:
            len = decode_little_endian_int32(element->data);
            return header_size + 4 + len;
            break;

        case BSON_JSCODEWSCOPE:
            len = decode_little_endian_int32(element->data);
            return header_size + 4 + len;
            break;

        case BSON_INT32:
            //int32 = 4 bytes
            return header_size + 4;
            break;

        case BSON_TIMESTAMP:
            return header_size + 8;

        case BSON_INT64:
            return header_size + 8;

        case BSON_DEC128:
            return header_size + 16;

        case BSON_MINKEY:
        case BSON_MAXKEY:
            return header_size;
            break;

        default:
            return -1;
            break;
    }
    return -1;
}


/*
 * Given a BSON object, find the first element 
 * whose name matches @key, and return the element as a bson_elem_t object. If no element with given key name is found, returns NULL.
 */
bson_elem_t* 
_bson_find_by_key(bson_t* obj, char* key){
    
    char* arr_pos = obj->data + 4;
    int end_of_doc = 0;
    int matched = -1;
    int len = 0;
    
    while(!end_of_doc){

        bson_elem_t* element = (bson_elem_t*)malloc(sizeof(bson_elem_t));

        element->type_id = *arr_pos;
        
        char* e_name = arr_pos + 1;
        int e_name_len = strlen(e_name);
        element->e_name = (char*)malloc(e_name_len+1);
        memcpy(element->e_name, e_name, e_name_len+1); // replace with strcpy??


        //Copy data from BSON buffer into element object
        int elem_size = _bson_element_size_bytes(element);
        element->data = (char*) malloc(elem_size);
        memcpy(element->data, arr_pos + e_name_len + 1, elem_size);

        // Check if we found the key.
        matched = strcmp(element->e_name, key);

        if(matched==0){
            return element;
        }

        // Skip ahead to the next element.
        arr_pos += elem_size;

    } 

    return NULL; 
}

char*
_bson_element_to_string(bson_elem_t* element){
    return NULL;
}

char* 
bson_to_json(bson_t* obj, err_t* err){
    
    char* arr_pos = obj->data + 4;
    int end_of_doc = 0;

    while(!end_of_doc){
        bson_elem_t* element = (bson_elem_t*)malloc(sizeof(bson_elem_t));

        element->type_id = *arr_pos;
        
        char* e_name = arr_pos + 1;
        int e_name_len = strlen(e_name);
        element->e_name = (char*)malloc(e_name_len+1);
        memcpy(element->e_name, e_name, e_name_len+1);

        element->data = arr_pos + e_name_len + 1;

        // Implement this!
        /* _bson_element_to_string(element); */

        // Skip ahead to the next element.
        arr_pos += _bson_element_size_bytes(element);

    }

    return NULL;
}


char* 
bson_get_val_string(bson_t* obj, char* key, err_t* err, int* len){
    /*
     *   BSON Spec:
     *   string  ::= int32 (byte*) "\x00"
    */

    bson_elem_t* element = _bson_find_by_key(obj, key);

    // Check if found.
    if(element==NULL){
        strcpy(err->msg, "No key found.");
        return NULL;
    }

    // Check if correct BSON type.
    if(element->type_id!=BSON_STRING){
        strcpy(err->msg, "Unable to decode element as string.");
        return NULL;
    }

    int str_len = decode_little_endian_int32(element->data);

    char* out_str = (char*) malloc(str_len+1);
    memcpy(out_str, element->data + 4, str_len+1);
    *len = str_len;

    //Start at beginning of BSON doc, past length header
    return out_str;
}

double
bson_get_val_double(bson_t* obj, char* key, err_t* err){

    bson_elem_t* element = _bson_find_by_key(obj, key);

    // Check if found.
    if(element==NULL){
        strcpy(err->msg, "No key found.");
        return -1;
    }

    // Check if correct BSON type.
    if(element->type_id!=BSON_DOUBLE){
        strcpy(err->msg, "Unable to decode element as double.");
        return -1;
    }

    return decode_little_endian_double(element->data);
}

int
bson_get_val_int32(bson_t* obj, char* key, err_t* err){

    bson_elem_t* element = _bson_find_by_key(obj, key);

    // Check if found. 
    if(element==NULL){
        strcpy(err->msg, "No key found.");
        return -1;
    }

    // Check if correct BSON type. 
    if(element->type_id!=BSON_INT32){
        strcpy(err->msg, "Unable to decode element as int32.");
        return -1;
    }

    return decode_little_endian_int32(element->data);
}

bson_t*
bson_get_val_document(bson_t* obj, char* key, err_t* err){

    bson_elem_t* element = _bson_find_by_key(obj, key);

    // Check if found.
    if(element==NULL){
        strcpy(err->msg, "No key found.");
        return NULL;
    }

    // Check if correct BSON type. 
    if(element->type_id!=BSON_DOCUMENT){
        strcpy(err->msg, "Unable to decode element as document.");
        return NULL;
    }

    return bson_decode(element->data);
}

int
bson_get_val_boolean(bson_t* obj, char* key, err_t* err){

    bson_elem_t* element = _bson_find_by_key(obj, key);

    // Check if found. 
    if(element==NULL){
        strcpy(err->msg, "No key found.");
        return -1;
    }

    // Check if correct BSON type. 
    if(element->type_id!=BSON_BOOLEAN){
        strcpy(err->msg, "Unable to decode element as boolean.");
        return -1;
    }

    return (int)(*element->data);
}






