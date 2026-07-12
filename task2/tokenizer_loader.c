#include "tokenizer_loader.h"
#include<stdio.h>
#include"hashmap.h"
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
        if(line_len > 0 && line_len<sizeof(line)-1)
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

                    while(*token2 == ' ')
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

    size_t capacity = hash_map_get_capacity(vocab);

    for(size_t i = 0; i < capacity;i++)
    {
        HashNode* current = hash_map_get_bucket(vocab,i);
        while(current)
        {
            int id = hash_map_get_value(current);
            char key_str[32];
            snprintf(key_str,sizeof(key_str),"%d",id);
            hash_map_insert(inverse,key_str,id);

            current = hash_map_get_next(current);

        }
    }
    return inverse;
}


TokenizerData* tokenizer_init(const char* vocab_path,const char* merges_path)
{
    printf("Initializing Tokenizer Loader\n");

    TokenizerData* data = malloc(sizeof(TokenizerData));
    if(!data)
    {
        fprintf(stderr,"Error: Failed to allocate tokenizer data\n");
        return NULL;
    }

    data->vocab = load_vocab_json(vocab_path);

    if(!data->vocab)
    {
        fprintf(stderr,"Error: Failed to laod vocabulary\n");
        free(data);
        return NULL;
    }

    data->merges = load_merges_txt(merges_path);
    if(!data->merges)
    {
        fprintf(stderr,"Error: Failed to laod merge rules\n");
        free(data);
        return NULL;
    }

    data->id_to_token = hash_map_create(65536);
    if(!data->id_to_token)
    {
        fprintf(stderr,"Error: Failed to build inverse vocabulary for decoding\n");
    }
    else
    {
        
    }
    printf("Tokenizer loaded\n");

    return data;
}
bool tokenizer_lookup_id(const TokenizerData* data,const char* token,int* out_id)
{
    if(!data||!data->vocab||!token)
    {
        return false;
    }
    return hash_map_get(data->vocab,token,out_id);
}
// tokenizer_loader.h
bool tokenizer_lookup_token(const TokenizerData* data, int id, const char** out_token)
{
    return false;
}
int tokenizer_get_merge_rank(const TokenizerData* data, const char* t1,const char* t2)
{
    if(!data||!data->merges||!t1||!t2)
    {
        return -1;
    }
    return merge_table_get_rank(data->merges,t1,t2);
}
size_t tokenizer_vocab_size(const TokenizerData* data)
{
    if(!data||data->vocab)
    {
        return 0;
    }
    return hash_map_size(data->vocab);
}
void tokenizer_free(TokenizerData* data)
{
    if(!data)
    {
        return ;
    }
    hash_map_destroy(data->vocab);
    merge_table_destroy(data->merges);
    hash_map_destroy(data->id_to_token);
    free(data);
}

void tokenizer_print_stats(const TokenizerData* data) {
    if (!data) {
        printf("Tokenizer data: NULL\n");
        return;
    }
    
    printf("\n TOKENIZER DATA STATISTICS \n");
    printf("Vocabulary Size: %zu\n", tokenizer_vocab_size(data));
    printf("Merge Rules: %zu\n", merge_table_size(data->merges));
    printf("Inverse Vocab: %s\n", data->id_to_token ? "Built" : "Not built");
    
    hash_map_print_stats(data->vocab);
    merge_table_print_stats(data->merges);
}

bool tokenizer_verify(const TokenizerData* data)
{
    if(!data)
    {
        return false;
    }
    if(!data->merges)
    {
        return false;
    }
    if(!data->vocab)
    {
        return false;
    }
    if(hash_map_size(data->vocab)==0)
    {
        return false;
    }
    if(merge_table_size(data->merges))
    {
        return false;
    }
    return true;
}