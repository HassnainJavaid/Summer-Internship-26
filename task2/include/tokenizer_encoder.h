#ifndef TOKENIZER_ENCODER_H
#define TOKENIZER_ENCODER_H


#include "tokenizer_loader.h"
#include <stddef.h>

int* tokenizer_encode(const TokenizerData* data,const char* text, size_t* out_len );

#endif