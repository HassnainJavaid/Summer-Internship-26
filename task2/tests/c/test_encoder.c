#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer_loader.h"
#include "tokenizer_splitter.h"
#include "byte_encoder.h"

// Explicit forward declarations to prevent compiler warnings
int* tokenizer_encode(const TokenizerData* data, const char* text, size_t* out_len);
char* tokenizer_decode(const TokenizerData* data, const int* ids, size_t len);

// 📖 The missing link: Function to read an entire file into a string buffer
static char* read_text_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open input text file: %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(file);
        return NULL;
    }

    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed for input text file.\n");
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0'; 
    
    fclose(file);
    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_text_file.txt>\n", argv[0]);
        return 1;
    }

    // 1. Read input text file into memory instead of using the raw argument string
    char* original_input = read_text_file(argv[1]);
    if (!original_input) {
        return 1; 
    }

    // 2. Initialize data structures
    init_byte_encoder();
    TokenizerData* data = tokenizer_init("data/tokenizer/vocab.json", "data/tokenizer/merges.txt");
    if (!data) {
        fprintf(stderr, "Error: Failed to initialize tokenizer data structure.\n");
        free(original_input);
        return 1;
    }

    size_t encoded_len = 0;

    // 3. RUN ENCODER & WRITE TO FILE
    int* encoded_tokens = tokenizer_encode(data, original_input, &encoded_len);
    if (!encoded_tokens) {
        fprintf(stderr, "Error: Encoding pipeline failed.\n");
        free(original_input);
        tokenizer_free(data);
        return 1;
    }

    FILE* token_file_out = fopen("artifacts/c_tokens.txt", "w");
    if (!token_file_out) {
        fprintf(stderr, "Error: Could not open artifacts/c_tokens.txt for writing.\n");
        free(encoded_tokens);
        free(original_input);
        tokenizer_free(data);
        return 1;
    }
    for (size_t i = 0; i < encoded_len; i++) {
        fprintf(token_file_out, "%d\n", encoded_tokens[i]);
    }
    fclose(token_file_out);
    free(encoded_tokens); 
    printf("[C Encoder] Successfully wrote %zu tokens to artifacts/c_tokens.txt\n", encoded_len);

    // 4. READ TOKEN IDs FROM FILE
    FILE* token_file_in = fopen("artifacts/c_tokens.txt", "r");
    if (!token_file_in) {
        fprintf(stderr, "Error: Could not open artifacts/c_tokens.txt for reading.\n");
        free(original_input);
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
    printf("[C Decoder] Safely read %zu tokens from artifacts/c_tokens.txt\n", read_count);

    // 5. RUN DECODER 
    char* decoded_text = tokenizer_decode(data, read_ids, read_count);
    free(read_ids); 

    if (!decoded_text) {
        fprintf(stderr, "Error: Decoding logic returned NULL.\n");
        free(original_input);
        tokenizer_free(data);
        return 1;
    }

    // 6. WRITE DECODED PAYLOAD TO FILE
    FILE* decoded_file_out = fopen("artifacts/decoded_output.txt", "w");
    if (!decoded_file_out) {
        fprintf(stderr, "Error: Could not open artifacts/decoded_output.txt.\n");
        free(decoded_text);
        free(original_input);
        tokenizer_free(data);
        return 1;
    }
    fprintf(decoded_file_out, "%s", decoded_text);
    fclose(decoded_file_out);
    printf("[C Decoder] Successfully wrote artifacts/decoded_output.txt\n");

    // 7. INTERNAL ROUND-TRIP CHECK
    if (strcmp(original_input, decoded_text) == 0) {
        printf("\n✅ C INTERNAL CHECK: Round-trip matches perfectly!\n");
    } else {
        printf("\n❌ C INTERNAL CHECK: Mismatch encountered!\n");
    }

    free(decoded_text);
    free(original_input);
    tokenizer_free(data);
    return 0;
}