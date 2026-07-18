#ifndef TOKENIZER_SPLITTER_H
#define TOKENIZER_SPLITTER_H

#include<stddef.h>
#include<stdbool.h>

typedef enum{
    TOKEN_TYPE_WORD,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_PUNCTUATION,
    TOKEN_TYPE_WHITESPACE,
    TOKEN_TYPE_SPECIAL,
    TOKEN_TYPE_BYTE,
}TokenType;

typedef struct 
{    /* data */
    char* text;
    TokenType type; 
    size_t length;
}SplitToken;
typedef struct{
    SplitToken* tokens;
    size_t count;
    size_t capacity;
}SplitResult;


SplitResult* gpt2_split_text(const char* text);
void split_result_free(SplitResult* result);
void split_result_print(const SplitResult* result);

#endif