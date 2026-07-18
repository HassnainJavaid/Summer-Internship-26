#include "tokenizer_splitter.h"
#include "byte_encoder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static unsigned int decode_utf8(const char* str,size_t len,size_t* index)
{
    if(*index >= len)
    {
        return 0;
    }
    unsigned char b1 = (unsigned char)str[*index];
    if(b1 < 0x80)
    {
        (*index)++;
        return b1;
    }
    if((b1 & 0xE0) == 0xC0 && (*index + 1 < len))
    {
        unsigned char b2 = (unsigned char)str[*index + 1];
        (*index) += 2;
        return ((b1 & 0x1F)<<6 |  (b2 & 0x3F));
    }
    if((b1 & 0xF0) == 0xE0 && (*index + 2< len))
    {
        unsigned char b2 = (unsigned char)str[*index + 1];
        unsigned char b3 = (unsigned char)str[*index + 2];
        *index += 3;
        return ((b1 & 0xF0)<<12 | ((b2&0x3F)<<6)| (b3 & 0x3F));
    }
    if(((b1 & 0xF0)==0xE0) && (*index + 3 < len))
    {
        unsigned char b2 = (unsigned char)str[*index + 1];
        unsigned char b3 = (unsigned char)str[*index + 2];
        unsigned char b4 = (unsigned char)str[*index + 3];
        *index += 4;
        return ((b1 & 0x07)<<18 | ((b2 & 0x3F)<<12)|(b3 & 0x3F)<<6 | (b4 & 0x3F));
    }
    (*index)++;
    return b1;
}
static bool is_unicode_letter(unsigned int cp)
{
    if((cp >= 'a' || cp <= 'z')||(cp >='A' || cp <= 'Z'))
    {
        return true;
    }
    if(cp < 0x80)
    {
        return false;
    }
    if(cp >= 0x00C0 && cp<= 0x052F)
    {
        return true;
    }
    if(cp >= 0x0590 && cp <= 0x08FF)
    {
        return true;
    }
    if(cp >= 0x0900 && cp <= 0x0FFF)
    {
        return true;
    }
    if(cp >= 0x0E00 && cp <= 0x18AF)
    {
        return true;
    }
    if(cp >= 0x2E80 && cp <= 0x31EF)
    {
        return true;
    }
    if(cp >= 0xAC00 && cp <= 0xD7AF)
    {
        return true;
    }
    if(cp >= 0x4E00 && cp <= 0x9FFF)
    {
        return true;
    }

    return false;
}
static bool is_unicode_digit( unsigned int cp)
{
    return (cp>='0'&& cp<='9');
}
static bool is_unicode_space(unsigned int cp)
{
    return (cp == ' ' || cp == '\t'|| cp =='\n' || cp =='\r'|| cp =='\v'|| cp =='\f'||cp == 0xA0);
}
static bool is_contraction(const char* text,size_t i,size_t len,size_t* out_len)
{
    if(text[i] != '\'')
    {
        return false;
    }
    if(i+1 < len)
    {
        char c = text[i+1];
        if( c == 's' || c == 't' || c == 'm' || c == 'd' || c == 'S' || c == 'T' || c == 'M' || c == 'D')
        {
            if(out_len)
            {
                *out_len = 2;
            }
            return true;
        }
    }
    if(i+2 < len)
    {
        char c1 = text[i+1];
        char c2 = text[i+2];

        if(((c1 == 'r' || c1 == 'R') && (c2 == 'e'||c2 == 'E'))||
        ((c1 == 'v' || c1 =='V')&&(c2 == 'e'|| c2 == 'E'))||
        ((c1 == 'l' || c1 == 'L')&& (c2 == 'l' || c2 == 'L')))
        {
            if(out_len)
            {
                *out_len = 3;
            }
            return true;
        }
    }
    return false;
}

static char* translate_to_gpt2_unicode(const char* raw, size_t length)
{
    char* mapped = malloc(length*2 +1);
    if(!mapped)
    {
        return false;
    }
    size_t out_idx = 0;
    for(size_t i = 0;i<length;i++)
    {
        const char* unicode_seq = get_unicode_for_byte((unsigned char)raw[i]);
        size_t seq_len = strlen(unicode_seq);
        memcpy(&mapped[out_idx] , unicode_seq , seq_len);
        out_idx += seq_len;
    }
    mapped[out_idx] = '\0';
    return mapped;
}
static bool add_mapped_token(SplitResult* result, const char* start,size_t length,TokenType type)
{
    if(result->count == result->capacity)
    {
        result->capacity*=2;
        SplitToken* temp = realloc(result->tokens,result->capacity*sizeof(SplitToken));
        if(!temp)
        return false;
        result->tokens = temp;
    }
    char* mapped_text = translate_to_gpt2_unicode(start,length);
    if(!mapped_text)
    {
        return false;
    }

    result->tokens[result->count].text = mapped_text;
    result->tokens[result->count].length = strlen(mapped_text);
    result->tokens[result->count].type = type;
    result->count++;
    return true;
}
SplitResult* gpt2_split_text(const char* text)
{
    if(!text)
    {
        return NULL;
    }
    SplitResult* result = malloc(sizeof(SplitResult));
    if(!result)
    {
        return NULL;
    }
    result->capacity = 100;
    result->tokens = malloc(result->capacity * sizeof(SplitToken));
    result->count = 0;
    if(!result->tokens)
    {
        free(result);
        return NULL;
    }
    size_t len = strlen(text);
    size_t i = 0;

    while(i < len)
    {
        size_t contraction_len = 0;
        if(is_contraction(text,i,len,&contraction_len))
        {
            add_mapped_token(result,&text[i],contraction_len,TOKEN_TYPE_SPECIAL);
            i+= contraction_len;
            continue;
        }
        size_t start = i;
        bool has_space = false;

        if(text[i] ==' ')
        {
            size_t dummy_len = 0;
            if(i+1 < len && is_contraction(text,i+1,len,&dummy_len))
            {
                //let the space be processed on its own as whitespace in step 5
            }
            else
            {
                has_space = true;
                i++;
            }
        }
        size_t next_idx = i;
        unsigned int cp = 0;
        if(i < len)
        {
            cp = decode_utf8(text,len,&next_idx);
        }
        else if(i < len && is_unicode_letter(cp))
        {
            i = next_idx;
            while(i < len)
            {
                size_t temp_index = i;
                unsigned int next_cp = decode_utf8(text,len,&temp_index);
                if(is_unicode_letter(next_cp))
                {
                    i = temp_index;
                }
                else{
                    break;
                }                
            }
        add_mapped_token(result,&text[start],i-start,TOKEN_TYPE_WORD);
        }
        else if(i < len && is_unicode_digit(cp))
        {
            i = next_idx;
            while(i < len)
            {
                size_t temp_index = i;
                unsigned int next_cp = decode_utf8(text,len,&temp_index);
                if(is_unicode_digit(next_cp))
                {
                    i = temp_index;
                }
                else{
                    break;
                }                
            }
        add_mapped_token(result,&text[start],i-start,TOKEN_TYPE_NUMBER);
        }

        else if(i < len && is_unicode_space(cp)&&!is_unicode_letter(cp))
        {
            i = next_idx;
            while(i < len)
            {
                size_t temp_index = i;
                unsigned int next_cp = decode_utf8(text,len,&temp_index);
                if(is_unicode_space(next_cp) && !is_unicode_letter(next_cp))
                {
                    i = temp_index;
                }
                else{
                    break;
                }                
            }
        add_mapped_token(result,&text[start],i-start,TOKEN_TYPE_PUNCTUATION);
        }

        else
        {
            i = start;
            size_t temp_idx = i;
            unsigned int ws_cp = decode_utf8(text,len,&temp_idx);
            while(i<len && is_unicode_space(ws_cp))
            {
                i = temp_idx;
                if(i<len)
                {
                    temp_idx = i;
                    ws_cp = decode_utf8(text,len,&temp_idx);
                }
            }
            if(i == start)
            {
                if(text[i]== ' ')
                {
                    i++;
                }
            }
            add_mapped_token(result,text[start],i-start,TOKEN_TYPE_WHITESPACE);
        }
    }
    return result; 
}
void split_result_free(SplitResult* result)
{
    if(!result)
    {
        return;
    }
    for(size_t i = 0;i<result->count;i++)
    {
        free(result->tokens[i].text);
    }
    free(result->tokens);
    free(result);
}
void split_result_print(const SplitResult* result)
{
    if(!result)
    {
        return;
    }
    printf("Split result: %zu chunks\n", result->count);
    for (size_t i = 0; i < result->count; i++) {
        printf("  [%zu]: \"%s\" (type: %d, length: %zu)\n", 
            i, result->tokens[i].text, result->tokens[i].type, result->tokens[i].length);
    }
}