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
        size_t char_start = idx;
        unsigned char b1 = (unsigned char)combined_unicode[idx];



        if(b1 < 0x80)
        {
            idx++;
        }
        else if((b1 & 0xE0) == 0xC0)
        {
            idx+=2;
        }
        else if((b1 & 0xF0) == 0xE0)
        {
            idx +=3;
        }
        else if((b1 & 0xF8) == 0xF0)
        {
            idx += 4;
        }
        else
        {
            idx++;
        }

        if(idx > write_offset)
        {
            idx = write_offset;
        }
        size_t char_len = idx - char_start;

        char char_buf[8];
        memcpy(char_buf, combined_unicode + char_start, char_len);
        char_buf[char_len] = '\0';


        unsigned char original_byte = 0;
        for(int b = 0;b < 256; b++)
        {
            if(strcmp(get_unicode_for_byte((unsigned char)b),char_buf) == 0)
            {
                original_byte = (unsigned char)b;
                break;
            }        
        }
        clear_text[out_idx++] = (unsigned char)original_byte; 
    }
    clear_text[out_idx] = '\0';
    free(combined_unicode);

    return clear_text;
}