#ifndef _MINCC_MAP_H_
#define _MINCC_MAP_H_


#include <stddef.h>


typedef char* map_key_t;
typedef void* map_value_t;

enum _MapState {
    Empty,
    Filled,
    Deleted,
};

typedef struct {
    map_key_t key;
    map_value_t value;
    enum _MapState state;
} map_item_t;

typedef struct {
    map_item_t* data;
    size_t size;
    size_t capacity;
} Map;

Map* map_new();
void map_insert(Map* map, map_key_t key, map_value_t value);
map_value_t map_find(Map* map, map_key_t key);
void map_delete(Map* map);


#endif  // _MINCC_MAP_H_
