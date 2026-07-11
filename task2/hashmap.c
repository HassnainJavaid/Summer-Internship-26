#include "hashmap.h"
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
void hash_map_insert(HashMap* map,const char* key,int valeu)
{
    if(!map||!key){
        return;
    }
    if(should_resize(map)){
        resize_map(map);
    }
    size_t index = get_bucket_index(map,key);

    HashNode* current = map->buckets[index];
    while(current){
        if(strcmp(current->key,key)==0){
            current->value = valeu;
            map->version++;
            return;
        }
        current = current->next;
    }
   HashNode* new_node = malloc(sizeof(HashNode));
   if(!new_node){
       fprintf(stderr,"Error:failed to allocate Hash node\n");
       return;
   }
   char* key_copy = strdup(key);
   if(!key_copy){
       free(new_node);
       fprintf(stderr,"Error: Failed to duplicate key\n");
       return;
   }
   new_node->key = key_copy;
   new_node-> value = valeu;
   new_node->next = map->buckets[index];
   map->buckets[index] = new_node;
   map->size++;
   map->version++;
}
bool hash_map_get(const HashMap* map,const char* key ,int* out_value){
    if(!map||!key)
        return false;
    size_t index = get_bucket_index(map,key);
    HashNode* current = map->buckets[index];

    while(current){
        if(strcmp(current->key,key)==0){
            if(out_value) 
                *out_value = current->value;
            return true;
        }
        current = current->next;
    }
    return false;
}
bool hash_map_contains(const HashMap* map,const char* key){
    int dummy;
    return hash_map_get(map,key,&dummy);
}
size_t hash_map_size(const HashMap* map)
{
    return map ? map->size:0;
}
void hash_map_clear(HashMap* map){
    if(!map)
        return;
    for(size_t i = 0;i < map->capacity;i++){
        HashNode* current = map->buckets[i];
        while(current) {
            HashNode* next = current->next;
            free(current->key);
            free(current);
            current = next;
        }
        map->buckets[i] = NULL;
    }
    map->size = 0;
    map->version++;
}
void hash_map_print_stats(const HashMap* map) {
    if (!map) {
        printf("Hash map: NULL\n");
        return;
    }
    
    printf("=== Hash Map Statistics ===\n");
    printf("Size: %zu entries\n", map->size);
    printf("Capacity: %zu buckets\n", map->capacity);
    printf("Load Factor: %.2f (threshold: %.2f)\n", 
           (float)map->size / map->capacity, map->load_factor);
    size_t max_chain = 0;
    size_t empty_buckets = 0;
    size_t total_chain_length = 0;
    
    for (size_t i = 0; i < map->capacity; i++) {
        size_t chain_len = 0;
        HashNode* current = map->buckets[i];
        while (current) {
            chain_len++;
            current = current->next;
        }
        
        if (chain_len == 0) empty_buckets++;
        if (chain_len > max_chain) max_chain = chain_len;
        total_chain_length += chain_len;
    }
    
    printf("Empty Buckets: %zu (%.2f%%)\n", empty_buckets, 
           100.0 * empty_buckets / map->capacity);
    printf("Max Chain Length: %zu\n", max_chain);
    printf("Average Chain Length: %.2f\n", 
           (double)total_chain_length / map->capacity);
    printf("==========================\n");
}
