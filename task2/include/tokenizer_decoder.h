#ifndef TOKENIZER_DECODER_H
#define TOKENIZER_DECODER_H


#include<stddef.h>
#include "tokenizer_loader.h"


char* tokenizer_decode(const TokenizerData* data,const int* ids,size_t len);


#endif