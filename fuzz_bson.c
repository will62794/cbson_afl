#include <stdlib.h>
#include "bson.h"

int main(int argc, char const *argv[])
{
    
    // Read all bytes from stdin
    FILE *fp = fopen("/dev/stdin","rb");
    char* bson_bytes;
    fseek(fp, 0, SEEK_END);
    long num_bytes = ftell(fp);
    rewind(fp);


    bson_bytes = (char *) malloc((num_bytes+1));
    fread(bson_bytes, num_bytes, 1, fp);
    fclose(fp);

    bson_t* obj = bson_decode(bson_bytes);
    

    return 0;
}