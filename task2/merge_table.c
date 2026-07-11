#include "merge_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

MergeTable* merge_table_create(size_t initial_capacity)
{
  MergeTable* table = malloc(sizeof(MergeTable));
  if(!table){
    fprintf(stderr,"Error: Failed to allocate merge Table\n");
    return NULL;
  }
  if(initial_capacity < 100)
    initial_capacity = 1000;
  table->rules = malloc(initial_capacity*sizeof(MergeRule));
  if(!table->rules){
    free(table);
    fprintf(stderr,"Error: Failed to allocate merge rules array\n");
    return NULL;
  }
  table->rank_lookup = hash_map_create(initial_capacity);
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
bool merge_table_add_rule(MergeTable* table,const char* t1,const char* t2,int rank)
{
  if(!table||!t1||!t2)
    return false;
  if(table->count >= table->capacity){
    size_t new_capacity = table->capacity*2;
    MergeRule* new_rules = realloc(table->rules,new_capacity * sizeof(MergeRule));
    if(!new_rules){
      fprintf(stderr,"Error: Failed to resize merge  rules\n");
      return false;
    }
    table->rules = new_rules;
    table->capacity = new_capacity;
  }
  char* t1_copy = strdup(t1);
  char* t2_copy = strdup(t2);
  if(!t1_copy || !t2_copy)
  {
    free(t1_copy);
    free(t2_copy);
    fprintf(stderr, "ERROR: Failed to duplicate merge tokens\n");
    return false;
  }
  table->rules[table->count].token1 = t1_copy;
  table->rules[table->count].token2 = t2_copy;
  table->rules[table->count].rank = rank;
  table->count++;
  char key[512];
  
  snprintf(key,sizeof(key),"%s","%S",t1,t1);
  hash_map_insert(table->rank_lookup,key,rank);

  return true;  
}
