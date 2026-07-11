#include "hash_map.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct{
   char* token1;
   char* token2;
   int rank;
}MergeRule;
typedef struct{
  MergeRule* rules;
  size_t count;
  size_t capacity;
  HashMap* rank_lookup;
}MergeTable;

MergeTable* merge_table_create(size_t initial_capacity);
void merge_table_destroy(MergeTable* table);
bool merge_table_add_rule(MergeTable* table, const char* t1, const char* t2, int rank);
int merge_table_get_rank(const MergeTable* table, const char* t1, const char* t2);
size_t merge_table_size(const MergeTable* table);
void merge_table_print_stats(const MergeTable* table);

#endif 
