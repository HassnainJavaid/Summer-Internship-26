#include "tokenizer_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void test_hash_map() {
    printf("\n=== Testing Hash Map ===\n");
    
    HashMap* map = hash_map_create(100);
    assert(map != NULL);
    assert(hash_map_size(map) == 0);
    
    // Test insert and get
    hash_map_insert(map, "Hello", 15496);
    hash_map_insert(map, "world", 995);
    hash_map_insert(map, "!", 0);
    
    assert(hash_map_size(map) == 3);
    
    int value;
    assert(hash_map_get(map, "Hello", &value));
    assert(value == 15496);
    assert(hash_map_get(map, "world", &value));
    assert(value == 995);
    assert(hash_map_get(map, "!", &value));
    assert(value == 0);
    
    // Test update
    hash_map_insert(map, "Hello", 999);
    assert(hash_map_get(map, "Hello", &value));
    assert(value == 999);
    
    // Test contains
    assert(hash_map_contains(map, "Hello"));
    assert(!hash_map_contains(map, "Nonexistent"));
    
    hash_map_destroy(map);
    printf("Hash map tests passed!\n");
}

void test_merge_table() {
    printf("\n=== Testing Merge Table ===\n");
    
    MergeTable* table = merge_table_create(100);
    assert(table != NULL);
    assert(merge_table_size(table) == 0);
    
    // Test add rules
    assert(merge_table_add_rule(table, "h", "e", 0));
    assert(merge_table_add_rule(table, "i", "n", 1));
    assert(merge_table_add_rule(table, "t", "h", 2));
    
    assert(merge_table_size(table) == 3);
    
    // Test get rank
    assert(merge_table_get_rank(table, "h", "e") == 0);
    assert(merge_table_get_rank(table, "i", "n") == 1);
    assert(merge_table_get_rank(table, "t", "h") == 2);
    assert(merge_table_get_rank(table, "x", "y") == -1);
    
    merge_table_destroy(table);
    printf("Merge table tests passed!\n");
}

void test_tokenizer_loader() {
    printf("\n=== Testing Tokenizer Loader ===\n");
    
    // You'll need actual files for this test
    const char* vocab_path = "vocab.json";
    const char* merges_path = "merges.txt";
    
    TokenizerData* data = tokenizer_init(vocab_path, merges_path);
    
    if (data && tokenizer_verify(data)) {
        printf("Tokenizer loader initialized successfully!\n");
        tokenizer_print_stats(data);
        
        // Test lookups
        int id;
        if (tokenizer_lookup_id(data, "Hello", &id)) {
            printf("Found 'Hello' -> id: %d\n", id);
        }
        
        // Test merge rank
        int rank = tokenizer_get_merge_rank(data, "h", "e");
        if (rank >= 0) {
            printf("Merge rank for 'h e': %d\n", rank);
        }
        
        tokenizer_free(data);
    } else {
        printf("Warning: Could not load tokenizer files (files may not exist)\n");
        printf("This is expected if you haven't downloaded the files yet.\n");
    }
}

int main() {
    printf("========================================\n");
    printf("TOKENIZER LOADER TEST SUITE\n");
    printf("========================================\n");
    
    test_hash_map();
    test_merge_table();
    test_tokenizer_loader();
    
    printf("\n========================================\n");
    printf("ALL TESTS COMPLETED\n");
    printf("========================================\n");
    
    return 0;
}