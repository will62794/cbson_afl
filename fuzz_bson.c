#include <stdlib.h>
#include "bson.h"

// Read all bytes from stdin and set @len to number of bytes read
char* read_stdin(long* len){
    FILE *fp = fopen("/dev/stdin","rb");
    fseek(fp, 0, SEEK_END);
    *len = ftell(fp);
    rewind(fp);

    char* buf = (char *) malloc((*len+1));
    fread(buf, *len, 1, fp);
    fclose(fp);

    return buf;
}

int main(int argc, char const *argv[])
{
    char* bson_bytes;
    long num_bytes;
    err_t err;

    bson_bytes = read_stdin(&num_bytes);

    bson_t* obj = bson_decode(bson_bytes, (int)num_bytes, &err);
    if(obj==NULL){
        return 0;
    }

    char* json_str = bson_to_json(obj, &err);

    return 0;
}