#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bson.h"

enum bson_type_id {
    BSON_DOUBLE = 0x01,
    BSON_STRING = 0x02,
    // BSON_DOCUMENT = 0x03
    BSON_BINARY = 0x04,
    BSON_BOOLEAN = 0x08,
    BSON_NULL = 0x0A,
    BSON_INT32 = 0x10
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
 * Given a starting position in a raw BSON byte array, find the next element whose 
 * element name matches @key, and return a pointer to its position in the byte array.
 */
char* _bson_find_next_by_key(char* start_pos, char* key){
    
    char* arr_pos = start_pos;
    int end_of_doc = 0;
    int matched = -1;
    int str_len = 0;
    
    while(matched!=0 && !end_of_doc){
        enum bson_type_id elem_type = *arr_pos;
        matched = strcmp(arr_pos+1, key);
        
        /* 
         * Move the pointer to next document in byte array. The number of bytes to skip 
         * ahead is dependent on the element type, so we need specific skip-ahead logic 
         * for each  type.
        */

        // Skip e_name bytes.
        arr_pos += (strlen(arr_pos+1)+1);

        // Skip element value bytes (type-dependent).
        switch(elem_type){

            case BSON_DOUBLE:
                //double = 8 bytes
                arr_pos += 8;
                break;

            case BSON_STRING:
                // string  ::= int32 (byte*) "\x00"
                str_len |= arr_pos[0];
                str_len |= arr_pos[1]<<8;
                str_len |= arr_pos[2]<<16;
                str_len |= arr_pos[3]<<24;
                arr_pos += (4+str_len);
                break;

            case BSON_INT32:
                //int32 = 4 bytes
                arr_pos += 4;
                break;

            default:
                assert("bson type not supported yet!" == NULL);
        }
        
    } 

    if(matched!=0){
        return NULL;
    }

    return arr_pos; 
}

char* 
bson_get_val_string(bson_t* obj, char* key, err_t* err){

    //Start at beginning of BSON doc, past length header
    return NULL;

}
