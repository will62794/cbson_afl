#include "bson.h"

void test_bson_decode(){

    char data[] = {0x08, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x00};

    bson_t* obj = bson_decode(data);

}

void test_bson_get_val_string(){

    char data[] = {
        0x16, 0x00, 0x00, 0x00,                                // total document size
        0x02,                                                  // 0x02 = type String
        'h', 'e', 'l', 'l', 'o', 0x00,                         // field name
        0x06, 0x00, 0x00, 0x00, 'w', 'o', 'r', 'l', 'd', 0x00, // field value
        0x00 
    };

    char* str = bson_get_val_string()

}


int main(int argc, char const *argv[])
{
    /* code */
    test_bson_decode();
    return 0;
}

