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
static HashMap* load_vocab_json(const char* filename)
{
    printf("Loading vocab from :%s\n",filename);

    char* json_data = read_file_contents(filename);
    if(!json_data)
    {

        return NULL;
    }
    HashMap* vocab = hash_map_create(65536);
    if(!vocab)
    {
        free(json_data);
        return NULL;
    }
    const char* ptr = json_data;
    char key[256];
    int value;
    //skipping opening braces
    while(*ptr && *ptr !='}')
    {
        ptr++;
    }
    if(*ptr == '{')
        ptr++;
    //skipping white spaces
    while(*ptr && *ptr!='}')
    {
        while(*ptr &&(*ptr == ' '||*ptr =='\t' || *ptr == '\n' || *ptr == '\r'))
        {
            ptr++;
        }
        if(*ptr != '"')
        {
            ptr++;
            continue;
        }
        ptr++;
        //skip opening quote
        size_t key_len = 0;
        while(*ptr && *ptr != '"'){
            if(key_len < sizeof(key)-1)
            {
                key[key_len++] = *ptr;
            }
            ptr++;
        }
        key[key_len] = '\0';
        ptr++;

        while(*ptr && *ptr == ':')
            ptr++;
        if(*ptr == ':')
        {
            ptr++;
        }
        while(*ptr&&(*ptr == ' '||*ptr == '\t'||*ptr == '\n'|| *ptr == '\r'))
            ptr++;
        value = 0;
        int sign = 1;
        if(*ptr == '-')
        {
            sign = -1;
            ptr++;
        }
        while(*ptr >= '0'&& *ptr <= '9' )
        {
            value = value * 10 + (*ptr - '0');
            ptr++;
        }
        value*=sign;

        hash_map_insert(vocab,key,value);

        while(*ptr && * ptr != ',' && *ptr != '}')
        {
            ptr++;
        }
        if(*ptr == ',')
        {
            ptr++;
        }
    }
    free(json_data);
    printf("Loaded %zu entries from vocab\n",hash_map_size(vocab));
    return vocab;
}

static MergeTable* load_merges_txt(const char* filename)
{
    printf("Loading merges from:%s\n",filename);

    char* file_data = read_file_contents(filename);
    if(!file_data)
    {
        return NULL;
    }
    MergeTable* merges = merge_table_create(1000);
    if(!merges)
    {
        free(file_data);
        return NULL;
    }
    const char* ptr = file_data;
    char line[512];
    int line_number = 0;
    
    while(*ptr)
    {
        const char* line_start = ptr;
        while(*ptr && *ptr != '\n')
        {
            ptr++;
        }
        size_t line_len = ptr - line_start;
        if(line_len > 0 && line_len<sizeof(line)-1);
        {
            memcpy(line,line_start,line_len);
            line[line_len] = '\0';
            if(line_len > 0 && line[line_len-1]=='\r')
            {
                line[line_len - 1] = '\0';
            }
            if(line[0]!= '\0' && line[0]!= '#')
            {
                char* token1 = line;
                char* token2 = strchr(line,' ');
                if(token2)
                {
                    *token2 = '\0';
                    token2++;

                    while(token2 == ' ')
                    {
                        token2++;
                    }
                    if(*token2!='\0')
                    {
                        int rank = line_number-1;
                        if(line[0]!='#')
                        {
                            merge_table_add_rule(merges,token1,token2,rank);
                        }
                    }
                }
            }
        }
        if(*ptr == '\n')
        {
            ptr++;
        }
        line_number++;
    }
    free(file_data);
    printf("Loaded %zu merge rules\n",merge_table_size(merges));
    return merges;
}
static HashMap* build_inverse_vocab(HashMap* vocab)
{
    HashMap* inverse = hash_map_create(65536);
    if(!inverse)
    {
        return NULL;
    }

    typedef struct{
        char* key;
        int value;
    }Entry;
    Entry* entries = NULL;
    size_t entry_count = 0;

}