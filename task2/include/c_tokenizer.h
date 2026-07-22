#ifndef C_TOKENIZER_H
#define C_TOKENIZER_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct TokenizerData TokenizerData;

TokenizerData* tokenizer_init(const char* vocab_path, const char* merges_path);
void tokenizer_free(TokenizerData* data);

int* tokenizer_encode(const TokenizerData* data, const char* text, size_t* out_len);
char* tokenizer_decode(const TokenizerData* data, const int* ids, size_t len);

#ifdef __cplusplus
}
#endif

#endif