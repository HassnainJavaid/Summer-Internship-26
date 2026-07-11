#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hashmap.h"
#include"merge_table.h"


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
int main() {
    test_hash_map();
    test_merge_table();
    return 0;
}