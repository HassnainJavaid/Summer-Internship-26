#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stddef.h>
#include <stdbool.h>

typedef struct HashMap HashMap;
typedef struct HashNode HashNode;

HashMap* hash_map_create(size_t initial_Capacity);
void hash_map_destroy(HashMap* map);
void hash_map_insert(HashMap* map,const char* key,int val);
bool hash_map_get(const HashMap* map,const char* key,int* out_valeu);
bool hash_map_get_string(const HashMap* map,int key ,const char** out_valeu);
void hash_map_remove(const HashMap* map,const char*);
size_t hash_map_size(const HashMap* map);
void hash_map_clear(HashMap* map);
bool hash_map_contains(const HashMap* map,const char* key);
void hash_map_print(const HashMap* map);

#endif