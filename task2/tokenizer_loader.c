#include "tokenizer_loader.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

static char* read_file_contents(const char* filename){
    FILE* file = fopen(filename,"rb");
    if(!file)
    {
        fprintf(stderr,"Error: Could not open file: %s\n",filename);
        return NULL;
    }
    fseek(file,0,SEEK_END);
    long file_size = ftell(file);
    fseek(file,0,SEEK_SET);

    char* buffer = malloc(file_size+1);
    if(!buffer){
        fclose(file);
        fprintf(stderr, "ERROR: Failed to allocate memory for file\n");
        return NULL;
    }
    size_t bytes_read  = fread(buffer,1,file_size,file);
    if(bytes_read != (size_t)file_size)
    {
        free(buffer);
        fclose(file);
        fprintf(stderr,"Errorr: Failed to read Entire file\n");
        return NULL;
    }
    buffer[file_size] = '\0';
    fclose(file);

    return buffer;
}