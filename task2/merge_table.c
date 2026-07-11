#include "merge_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

MergeTable* merge_table_create(size_t initial_capacity)
{
  MergeTable* table = malloc(sizeof(MergeTable));
  if(!tabel){
    fprintf(stderr,"Error: Failed to allocate merge Table\n");
    return NULL:
  }
  if(initial_capacity < 100)
    initial_capacity = 1000;
  table->rules = malloc(initial_capacity*sizeof(MergeRule));
  if(!table->rules){
    free(table);
    fprintf(stderr,"Error: Failed to allocate merge rules array\n");
    return NULL;
  }
  table->rank_lookup = hash_map_create(intitial_capacity);
  if(!table->rank_lookup){
    free(table->rules);
    free(table);
    fprintf(stderr, "ERROR: Failed to create rank lookup hash map\n");
    return NULL;
  }
  table->count = 0;
  table->capacity = initial_capacity;

  return table;
}
void merge_table_destroy(MergeTable* table){
  if(!table)
    return;
  for(size_t i = 0;i<table->count;i++){
    free(table->rules[i].token1);
    free(table->rules[i].token2);
  }
  free(table->rules);
  hash_map_destroy(table->rank_lookup);
  free(table);
}
