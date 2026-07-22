// Save this part as "main.c" and append the implementation code above to compile directly.
#include "tokenizer_splitter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "byte_encoder.h"
void run_test(int test_num, const char* description, const char* input, size_t expected_count) {
    printf("==================================================\n");
    printf("TEST %d: %s\n", test_num, description);
    printf("INPUT: \"%s\"\n", input);
    printf("==================================================\n");

    SplitResult* res = gpt2_split_text(input);
    if (!res) {
        printf("❌ FAIL: System returned NULL result.\n\n");
        return;
    }

    printf("OUTPUT (%zu chunks):\n", res->count);
    for (size_t i = 0; i < res->count; i++) {
        const char* type_str = "UNKNOWN";
        switch (res->tokens[i].type) {
            case TOKEN_TYPE_WORD:        type_str = "WORD"; break;        // 👈 Assign to type_str
            case TOKEN_TYPE_NUMBER:      type_str = "NUMBER"; break;      // 👈 Assign to type_str
            case TOKEN_TYPE_PUNCTUATION: type_str = "PUNCTUATION"; break; // 👈 Assign to type_str
            case TOKEN_TYPE_WHITESPACE:  type_str = "WHITESPACE"; break;  // 👈 Assign to type_str
            case TOKEN_TYPE_SPECIAL:     type_str = "CONTRACTION"; break; // 👈 Assign to type_str
            case TOKEN_TYPE_BYTE:        type_str = "BYTE"; break;        // 👈 Assign to type_str
        }
        printf("  [%zu]: \"%s\" (Type: %s, Len: %zu)\n", 
               i, res->tokens[i].text, type_str, res->tokens[i].length);
    }

    if (res->count == expected_count) {
        printf("✅ PASS: Token count matches expectations.\n\n");
    } else {
        printf("❌ FAIL: Expected %zu tokens, but received %zu.\n\n", expected_count, res->count);
    }

    split_result_free(res);
}

int main() {
    // Initialize our byte map lookups
    init_byte_encoder();

    // Test 1: Standard punctuation regression (The infinite loop test)
    run_test(1, "Basic Punctuation Continuity", "Hello... World!", 4);

    // Test 2: Standard text sentence mapping with leading spaces glued
    run_test(2, "Standard Text & Leading Spaces", "Hello world 123", 3);

    // Test 3: Standard English contraction splits
    run_test(3, "Contraction Suffix Separation", "Don't they're choice", 5);

    // Test 4: Consecutive spacing rules 
    run_test(4, "Multi-spacing Block Compaction", "Word   spaced", 3);

    // Test 5: Complex trailing whitespace transitions
    run_test(5, "Trailing Space Capture", "End. ", 3);

    // Test 6: Multi-byte Russian (Cyrillic) integration
    run_test(6, "Multi-byte UTF-8 Cyrillic Text", "Привет мир", 2);

    return 0;
}