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


/*
    Given a pointer to the data of a BSON element (not including header and type_id), determine its total size 
    in bytes.
*/
int
_bson_element_data_size(char* element_data, enum bson_type_id type_id){
    int len;

    // Element data size (type-dependent).
    switch(type_id){

        case BSON_DOUBLE:
            //double = 8 bytes
            return 8;
            break;

        case BSON_STRING:
            // string  ::= int32 (byte*) "\x00"
            len = decode_little_endian_int32(element_data);
            return len + 4;
            break;

        case BSON_DOCUMENT:
        case BSON_ARRAY:
            len = decode_little_endian_int32(element_data);
            return len;
            break;

        case BSON_BINARY:
            len = decode_little_endian_int32(element_data);
            return len;
            break;

        case BSON_OBJECTID:
            // ObjectId: "\x07" e_name (byte*12)
            return 12;
            break;

        case BSON_BOOLEAN:
            return 1;
            break;

        case BSON_UTC:
            //UTC Timestamp: int64 = 8 bytes
            return 8;
            break;

        case BSON_NULL:
            break;

        case BSON_REGEX:
            //  Regex: "\x0B" e_name cstring cstring
            len = strlen(element_data)+1;
            return len + (strlen(element_data + len)+1);
            break;

        case BSON_JSCODE:
            len = decode_little_endian_int32(element_data);
            return 4 + len;
            break;

        case BSON_JSCODEWSCOPE:
            len = decode_little_endian_int32(element_data);
            return 4 + len;
            break;

        case BSON_INT32:
            //int32 = 4 bytes
            return 4;
            break;

        case BSON_TIMESTAMP:
            return 8;
            break;

        case BSON_INT64:
            return 8;
            break;

        case BSON_DEC128:
            return 16;
            break;

        case BSON_MINKEY:
        case BSON_MAXKEY:
            return 0;
            break;

        default:
            return -1;
            break;
    }
    return -1;
}

/* 
 * Given a pointer to an element in a BSON byte array, parse the element bytes into a bson_elem_t struct. Sets @len
   to the total length of the raw element in bytes (useful for jumping to next element in BSON document).
*/
bson_elem_t*
_bson_parse_element(char* elem_pos, int* len){
    bson_elem_t* element = (bson_elem_t*)malloc(sizeof(bson_elem_t));

    // Parse BSON type id.
    element->type_id = *elem_pos;
    
    // Parse e_name.
    char* e_name = elem_pos + 1;
    int e_name_len = strlen(e_name);
    element->e_name = (char*)malloc(e_name_len+1);
    strcpy(element->e_name, e_name);

    // Parse element data.
    char* elem_data = e_name + e_name_len + 1;
    int elem_size = _bson_element_data_size(elem_data, element->type_id);
    element->data = (char*) malloc(elem_size);
    memcpy(element->data, elem_data, elem_size); 
    

    // Set total element size in bytes
    *len = 1 + e_name_len + 1 + elem_size;   

    return element;
}

/*
 * Given a BSON object, find the first element 
 * whose name matches @key, and return the element as a bson_elem_t object. If no element with given key name is found, returns NULL.
 */
bson_elem_t* 
_bson_find_by_key(bson_t* obj, char* key){
    
    char* arr_base_pos = obj->data + 4;
    int arr_offset = 0;
    int matched = -1;
    int len = 0;

    while(arr_offset < obj->data_len){

        int elem_size;
        bson_elem_t* element = _bson_parse_element(arr_base_pos + arr_offset, &elem_size);

        // Check if we found the key.
        matched = strcmp(element->e_name, key);

        if(matched==0){
            return element;
        }

        // Skip ahead to the next element.
        arr_offset += elem_size;
    } 

    return NULL; 
}

char*
_bson_element_data_to_string(bson_elem_t* element){
    char* out_str;
    char* strval;
    double dval;
    int ival;
    err_t err;

    switch(element->type_id){
        case BSON_DOUBLE:
            dval = decode_little_endian_double(element->data);
            asprintf(&out_str, "%f", dval);
            break;

        case BSON_STRING:
            strval = element->data+4;
            asprintf(&out_str, "\"%s\"", strval);
            break;

        case BSON_DOCUMENT:
            out_str = bson_to_json(bson_decode(element->data), &err);
            break;

        case BSON_INT32:
            ival = decode_little_endian_int32(element->data);
            asprintf(&out_str, "%d", ival);
            break;

        case BSON_BOOLEAN:
            if(*element->data){
                asprintf(&out_str, "%s", "true");
            }
            else{
                asprintf(&out_str, "%s", "false");
            }
            break;

        default:
            assert("BSON type unsupported!"==NULL);

    }

    return out_str;
}

char* 
bson_to_json(bson_t* obj, err_t* err){
    
    char* arr_base_pos = obj->data + 4;
    int arr_offset = 0;
    char* json_str = (char*)malloc(1);
    int curr_str_size;
    int end_of_doc = 0;
    int elem_size;

    // document ::= int32 e_list "\x00"
    while(5 + arr_offset < obj->data_len){
        bson_elem_t* element = _bson_parse_element(arr_base_pos + arr_offset, &elem_size);

        char* elem_str;
        char* fmt = ",\"%s\":%s";
        char* data_str = _bson_element_data_to_string(element); 
        asprintf(&elem_str, fmt, element->e_name, data_str);
        
        //Append string
        json_str = realloc(json_str, strlen(json_str)+strlen(elem_str)+1);
        strcat(json_str, elem_str);

        // Skip ahead to the next element.
        arr_offset += elem_size;
    }

    // Quick hack to get rid of leading command and add curly brace at same time :)
    json_str[0]='{';

    // Append final curly brace.
    json_str = realloc(json_str, strlen(json_str)+2);
    strcat(json_str, "}");
    
    return json_str;
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






