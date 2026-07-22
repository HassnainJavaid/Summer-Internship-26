#include "../include/tokenizer_decoder.h"
#include "../include/tokenizer_loader.h"
#include "../include/byte_encoder.h"
#include <stdlib.h>
#include<string.h>
#include<stdbool.h>


char* tokenizer_decode(const TokenizerData* data,const int* ids,size_t len)
{
    if(!data || !ids  || !len)
    {
        return NULL;
    }
    size_t total_bytes = 0;
    for(size_t i = 0;i<len;i++)
    {
        const char* token_str = NULL;
        if(tokenizer_lookup_token(data,ids[i],&token_str))
        {
            total_bytes += strlen(token_str);
        }
    }
    char*  combined_unicode = malloc(total_bytes + 1);
    if(!combined_unicode)
    {
        return NULL;
    }
    size_t write_offset = 0;
    for(size_t i = 0; i < len;i++)
    {
        const char* token_str = NULL;
        if(tokenizer_lookup_token(data,ids[i],&token_str))
        {
            size_t t_len = strlen(token_str);
            memcpy(combined_unicode + write_offset,token_str,t_len);
            write_offset += t_len;
        }
    }
    combined_unicode[write_offset] = '\0';
    
    char* clear_text = malloc(total_bytes + 1);
    if(!clear_text)
    {
        free(combined_unicode);
        return NULL;
    }
    size_t idx = 0;
    size_t out_idx = 0;

    while(idx < write_offset)
    {
        int best_byte = -1;
        size_t best_len = 0;
        for(int b = 0; b < 256; b++)
        {
            const char* mapping = get_unicode_for_byte((unsigned char)b);
            size_t map_len = strlen(mapping);
            if(idx + map_len <= write_offset && memcmp(combined_unicode + idx ,mapping , map_len) == 0)
            {
                if(map_len > best_len)
                {
                    best_len = map_len;
                    best_byte = b;
                }
            }
        }
        if(best_len > 0)
        {
            clear_text[out_idx++] = (unsigned char)best_byte;
            idx += best_len;
        }
        else 
        {
            clear_text[out_idx++] = combined_unicode[idx++];
        }
    }
    clear_text[out_idx] = '\0';

    free(combined_unicode);
    return clear_text;
}