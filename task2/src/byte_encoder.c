#include "byte_encoder.h"
#include <stdio.h>
#include<stdbool.h>

static char BYTE_TO_UNICODE_MAP[256][5];
static bool is_initialized = false;

void init_byte_encoder()
{
    if(is_initialized)
    {
        return;
    }
    int n = 0;
    for(int b = 0; b<256;b++)
    {
        if((b >= 33 && b <= 126) || (b >= 161 && b <= 172) || (b >= 174 && b <= 255))
        {
            BYTE_TO_UNICODE_MAP[b][0] = (char)b;
            BYTE_TO_UNICODE_MAP[b][1] = '\0';
        }
        else if((b >= 161 && b <= 172) || (b >= 174 && b <= 255))
        {
            unsigned int cp = b;
            BYTE_TO_UNICODE_MAP[b][0] = (char)(0xC0 | (cp >> 6));
            BYTE_TO_UNICODE_MAP[b][1] = (char)(0x80 | (cp & 0x3F));
            BYTE_TO_UNICODE_MAP[b][2] = '\0';
        }
        else 
        {
            unsigned int cp = 256 + n++;
            BYTE_TO_UNICODE_MAP[b][0] = (char)(0xC0 | (cp>>6));
            BYTE_TO_UNICODE_MAP[b][1] = (char)(0x80 | (cp & 0x3F));
            BYTE_TO_UNICODE_MAP[b][2] = '\0';
        }
    }
    is_initialized = true;
}
const char* get_unicode_for_byte(unsigned char b)
{
    return BYTE_TO_UNICODE_MAP[b];
}
