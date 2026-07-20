#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer_loader.h"
#include "tokenizer_splitter.h"
#include "byte_encoder.h"

// Explicit forward declarations to prevent compiler warnings
int* tokenizer_encode(const TokenizerData* data, const char* text, size_t* out_len);
char* tokenizer_decode(const TokenizerData* data, const int* ids, size_t len);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"<text_to_encode>\"\n", argv[0]);
        return 1;
    }

    // 1. Initialize data structures
    init_byte_encoder();
    TokenizerData* data = tokenizer_init("vocab.json", "merges.txt");
    if (!data) {
        fprintf(stderr, "Error: Failed to initialize tokenizer data structure.\n");
        return 1;
    }

    const char* original_input = argv[1];
    size_t encoded_len = 0;

    // 2. RUN ENCODER & WRITE TO FILE
    int* encoded_tokens = tokenizer_encode(data, original_input, &encoded_len);
    if (!encoded_tokens) {
        fprintf(stderr, "Error: Encoding pipeline failed.\n");
        tokenizer_free(data);
        return 1;
    }

    FILE* token_file_out = fopen("c_tokens.txt", "w");
    if (!token_file_out) {
        fprintf(stderr, "Error: Could not open c_tokens.txt for writing.\n");
        free(encoded_tokens);
        tokenizer_free(data);
        return 1;
    }
    for (size_t i = 0; i < encoded_len; i++) {
        fprintf(token_file_out, "%d\n", encoded_tokens[i]);
    }
    fclose(token_file_out);
    free(encoded_tokens); // Clear temporary encoder buffer memory
    printf("[C Encoder] Successfully wrote %zu tokens to c_tokens.txt\n", encoded_len);

    // 3. READ TOKEN IDs FROM FILE (Testing file isolation)
    FILE* token_file_in = fopen("c_tokens.txt", "r");
    if (!token_file_in) {
        fprintf(stderr, "Error: Could not open c_tokens.txt for reading.\n");
        tokenizer_free(data);
        return 1;
    }

    size_t capacity = 128;
    int* read_ids = malloc(capacity * sizeof(int));
    size_t read_count = 0;
    int temp_id;

    while (fscanf(token_file_in, "%d", &temp_id) == 1) {
        if (read_count >= capacity) {
            capacity *= 2;
            read_ids = realloc(read_ids, capacity * sizeof(int));
        }
        read_ids[read_count++] = temp_id;
    }
    fclose(token_file_in);
    printf("[C Decoder] Safely read %zu tokens from c_tokens.txt\n", read_count);

    // 4. RUN DECODER 
    char* decoded_text = tokenizer_decode(data, read_ids, read_count);
    free(read_ids); // Clear ID holder array

    if (!decoded_text) {
        fprintf(stderr, "Error: Decoding logic returned NULL.\n");
        tokenizer_free(data);
        return 1;
    }

    // 5. WRITE DECODED PAYLOAD TO FILE
    FILE* decoded_file_out = fopen("decoded_output.txt", "w");
    if (!decoded_file_out) {
        fprintf(stderr, "Error: Could not open decoded_output.txt for output storage.\n");
        free(decoded_text);
        tokenizer_free(data);
        return 1;
    }
    fprintf(decoded_file_out, "%s", decoded_text);
    fclose(decoded_file_out);
    printf("[C Decoder] Successfully wrote decoded string to decoded_output.txt\n");

    // Internal quick check
    if (strcmp(original_input, decoded_text) == 0) {
        printf("\n✅ C INTERNAL CHECK: Round-trip matches perfectly!\n");
    } else {
        printf("\n❌ C INTERNAL CHECK: Mismatch encountered!\n");
    }

    free(decoded_text);
    tokenizer_free(data);
    return 0;
}