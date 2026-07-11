#include "hash_map.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static unsigned long hash_function(const char* str) {
    unsigned long hash = 1469598103934665603UL;  
    
    while (*str) {
        hash ^= (unsigned char)(*str);
        hash *= 1099511628211UL;  
        str++;
    }
    
    return hash;
}
struct HashNode {
    char* key;
    int value;
    struct HashNode* next;
};

struct HashMap {
    HashNode** buckets;
    size_t capacity;
    size_t size;
    float load_factor;
    int version;  
};
static size_t get_bucket_index(const HashMap* map, const char* key) {
    return hash_function(key) % map->capacity;
}

static bool should_resize(const HashMap* map) {
    return (float)map->size / map->capacity >= map->load_factor;
}

static HashMap* resize_map(HashMap* map) {
    size_t new_capacity = map->capacity * 2;
        HashNode** new_buckets = calloc(new_capacity, sizeof(HashNode*));
    if (!new_buckets) {
        fprintf(stderr, "ERROR: Failed to resize hash map\n");
        return NULL;
    }

    HashNode** old_buckets = map->buckets;
    size_t old_capacity = map->capacity;
    map->buckets = new_buckets;
    map->capacity = new_capacity;
    map->size = 0;  
    
    for (size_t i = 0; i < old_capacity; i++) {
        HashNode* current = old_buckets[i];
        while (current) {
            HashNode* next = current->next;
            size_t new_index = get_bucket_index(map, current->key);
            current->next = map->buckets[new_index];
            map->buckets[new_index] = current;
            map->size++;
            
            current = next;
        }
    }
    
    free(old_buckets);
    return map;
}
HashMap* hash_map_create(size_t initial_capacity) {
    size_t capacity = initial_capacity;
    if (capacity < 64) capacity = 64;
    
    HashMap* map = malloc(sizeof(HashMap));
    if (!map) {
        fprintf(stderr, "ERROR: Failed to allocate hash map\n");
        return NULL;
    }
    
    map->buckets = calloc(capacity, sizeof(HashNode*));
    if (!map->buckets) {
        free(map);
        fprintf(stderr, "ERROR: Failed to allocate hash buckets\n");
        return NULL;
    }
    
    map->capacity = capacity;
    map->size = 0;
    map->load_factor = 0.75f;
    map->version = 0;
    
    return map;
}

void hash_map_destroy(HashMap* map) {
    if (!map) return;
    
    // Free all nodes
    for (size_t i = 0; i < map->capacity; i++) {
        HashNode* current = map->buckets[i];
        while (current) {
            HashNode* next = current->next;
            free(current->key);  // Free duplicated string
            free(current);
            current = next;
        }
    }
    
    free(map->buckets);
    free(map);
}
