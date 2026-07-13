#ifndef TOKENIZER_LOADER_H
#define TOKENIZER_LOADER_H

#include<stdbool.h>
#include<stddef.h>
#include "hashmap.h"
#include"merge_table.h"

typedef struct{
    HashMap* vocab;
    char** id_to_token;
    MergeTable* merges;
    size_t array_size;
}TokenizerData;
//Initializer of vocab and merge files
TokenizerData* tokennizerinit(const char* vocab_path,const char* merges_path);
bool tokenizer_lookup_id(const TokenizerData* data,const char* token,int* out_id);
bool tokenizer_lookup_token(const TokenizerData* data,int id,const char** out_token);
int tokenizer_get_merge_rank(const TokenizerData* data,const char * t1,const char* t2);
TokenizerData* tokenizer_init(const char* vocab_path,const char* merges_path);
bool tokenizer_verify(const TokenizerData* data);
size_t tokenizer_vocab_size(const TokenizerData* data);
void tokenizer_free(TokenizerData* data);
void tokenizer_print_stats(const TokenizerData* data);
bool tokenizer_verfiy(const TokenizerData* data);
#endif