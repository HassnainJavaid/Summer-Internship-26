#include "tokenizer_splitter.h"
#include "tokenizer_loader.h"
#include "hashmap.h"
#include "merge_table.h"
#include "byte_encoder.h"
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<stdio.h>
#include<stdbool.h>

#define MAX_STACK_SYMBOLS 4096

typedef struct
{
    size_t start;
    size_t len;
    int next_idx;
}BPESymbolNode;

static void extract_symbol_text(const char* base,const BPESymbolNode* node,char* dest,size_t dest_max)
{
    size_t copy_len = node->len;
    if(copy_len > dest_max)
    {
        copy_len = dest_max -1;
    }
    memcpy(dest,base+node->start,copy_len);
    dest[copy_len] = '\0';
}

static void run_bpe_on_chunk(const TokenizerData* data,const char* chunk,BPESymbolNode* symbols)
{
    size_t chunk_len = strlen(chunk);
    size_t  sym_count = 0;
    size_t idx = 0;

    while(idx < chunk_len && sym_count < MAX_STACK_SYMBOLS)
    {
        size_t char_start = idx;
        unsigned char b1 = (unsigned char)chunk[idx];
        
        if(b1 < 0x80)
        {
            idx++;
        }
        else if((b1 & 0xE0) == 0xC0)
        {
            idx += 2;
        }
        else if((b1 & 0xF0) == 0xE0)
        {
            idx += 3;
        }
        else if((b1 & 0xF8) == 0xF0)
        {
            idx+= 4;
        }
        else
        {
            idx++;/* code */
        }
        symbols[sym_count].start = char_start;
        symbols[sym_count].len = idx - char_start;
        symbols[sym_count].next_idx = (int)(sym_count+1);
        sym_count++;
    }
    if(sym_count == 0)
    {
        return;
    }
    symbols[sym_count-1].next_idx = -1;

    while(true)
    {
        int best_rank = INT_MAX;
        int best_curr = -1;
        int best_next = -1;

        int curr = 0;
        while(curr != -1 && symbols[curr].next_idx != -1)
        {
            int next = symbols[curr].next_idx;
            char t1[256];
            char t2[256];
            extract_symbol_text(chunk, &symbols[curr], t1, sizeof(t1));
            extract_symbol_text(chunk,&symbols[next],t2,sizeof(t2));

            int rank = tokenizer_get_merge_rank(data,t1,t2);
            if(rank != -1 && rank < best_rank)
            {
                best_rank = rank;
                best_curr = curr;
                best_next = next;
            }
            curr = next;
        }
        if(best_curr == -1)
        {
            break;
        }

        symbols[best_curr].len += symbols[best_next].len;
        symbols[best_curr].next_idx = symbols[best_next].next_idx;
    }
}

int* tokenizer_encode(const TokenizerData* data,const char* text,size_t* out_len)
{
    if(!data||!text||!out_len)
    {
        return NULL;
    }
    SplitResult* split = gpt2_split_text(text);

    if(!split)
    {
        return NULL;
    }
    size_t capacity = split->count > 0 ? split->count*2 :8;
    int* token_ids = malloc(capacity * sizeof(int));
    size_t total_tokens = 0;

    BPESymbolNode symbols[MAX_STACK_SYMBOLS];

    for(size_t i = 0; i < split->count;i++)
    {
        const char* chunk = split->tokens[i].text;

        int direct_id;
        if(tokenizer_lookup_id(data,chunk,&direct_id))
        {
            if(total_tokens >= capacity)
            {
                capacity*=2;
                token_ids = realloc(token_ids,capacity*sizeof(int));
            }
            token_ids[total_tokens++] = direct_id;
            continue;
        }

        run_bpe_on_chunk(data,chunk,symbols);
        int curr  = 0;

        while(curr != -1)
        {
            char final_tokens[256];
            extract_symbol_text(chunk,&symbols[curr],final_tokens,sizeof(final_tokens));
            int token_id;
            if(tokenizer_lookup_id(data,final_tokens,&token_id))
            {
                if(total_tokens >= capacity)
                {
                    capacity *= 2;
                    token_ids = realloc(token_ids,capacity*sizeof(int));
                }
                token_ids[total_tokens++] = token_id;
            }
        curr = symbols[curr].next_idx;
        }
    }
    split_result_free(split);
    *out_len = total_tokens;
    return token_ids;
}