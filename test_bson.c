#include "bson.h"
#include <assert.h>
#include <string.h>

void test_bson_decode(){

    char data[] = {0x08, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x00};
    bson_t* obj = bson_decode(data);

    assert(obj->data_len==8);
    for(int i=0;i<8;i++){
        assert(obj->data[i]==data[i]);
    }

}

void test_bson_get_val_string(){

    char data[] = {
        0x16, 0x00, 0x00, 0x00,                                // total document size
        0x02,                                                  // 0x02 = type String
        'h', 'e', 'l', 'l', 'o', 0x00,                         // field name
        0x06, 0x00, 0x00, 0x00, 'w', 'o', 'r', 'l','d', 0x00, // field value
        0x00 
    };
    bson_t* obj = bson_decode(data);

    int len;
    char* str = bson_get_val_string(obj, "hello", NULL, &len);
    assert(len==6);
    assert(strcmp(str,"world")==0);

}

void test_bson_get_vals(){

    /*
        {"hello":"world", "william":5, "age":1.234}
    */

    char data[] = {
      0x30, 0x00, 0x00, 0x00, 0x02, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x00, 0x06,
      0x00, 0x00, 0x00, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x00, 0x10, 0x77, 0x69,
      0x6c, 0x6c, 0x69, 0x61, 0x6d, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x61,
      0x67, 0x65, 0x00, 0x58, 0x39, 0xb4, 0xc8, 0x76, 0xbe, 0xf3, 0x3f, 0x00
    };

    bson_t* obj = bson_decode(data);

    int len;
    err_t err;

    char* str = bson_get_val_string(obj, "hello", NULL, &len);
    assert(strcmp(str,"world")==0);

    int val = bson_get_val_int32(obj, "william", NULL);
    assert(val==5);

    double dval = bson_get_val_double(obj, "age", NULL);
    assert(dval==1.234);

}


int main(int argc, char const *argv[])
{
    /* code */
    test_bson_decode();
    test_bson_get_val_string();
    test_bson_get_vals();
    return 0;
}

