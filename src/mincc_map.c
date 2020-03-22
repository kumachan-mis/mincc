#include "mincc_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mincc_memory.h"


Map* map_extend(Map* map);
size_t hash_function(map_key_t key, size_t capacity);
char* key_new(char* key);
int has_key(map_item_t item, char* key);


Map* map_new() {
    size_t initial_capacity = 4;
    Map* map = (Map*)safe_malloc(sizeof(Map));
    map->data = (map_item_t*)safe_malloc(initial_capacity*sizeof(map_item_t));
    map->size = 0;
    map->capacity = initial_capacity;
    size_t i;
    for (i = 0; i < initial_capacity; i++) {
        map->data[i].key = NULL;
        map->data[i].value = NULL;
        map->data[i].state = Empty;
    }
    return map;
}

void map_insert(Map* map, map_key_t key, map_value_t value) {
    if (map->size + 1 > map->capacity / 2) {
        map = map_extend(map);
    }
    size_t capacity = map->capacity;
    size_t hash_index = hash_function(key, capacity);
    int found = 0;
    while (1) {
        found = has_key(map->data[hash_index], key);
        if (found || map->data[hash_index].state != Filled) break;
        hash_index = (hash_index + 1) % capacity;
    }

    if (!found) {
        map->data[hash_index].key = key_new(key);
        map->data[hash_index].value = value;
        map->data[hash_index].state = Filled;
        map->size++;
    } else {
        free(map->data[hash_index].value);
        map->data[hash_index].value = value;
    }
}

map_value_t map_find(Map* map, map_key_t key) {
    size_t capacity = map->capacity;
    size_t hash_index = hash_function(key, map->capacity);
    int found = 0;
    while (1) {
        found = has_key(map->data[hash_index], key);
        if (found || map->data[hash_index].state == Empty) break;
        hash_index = (hash_index + 1) % capacity;
    }
    return found? map->data[hash_index].value : NULL;

}

void map_delete(Map* map) {
    size_t capacity = map->capacity;
    size_t i;
    for (i = 0; i < capacity; ++i) {
        map_item_t item = map->data[i];
        if (item.state != Filled) continue;
        free(item.key);
        free(item.value);
    }
    free(map->data);
    free(map);
}

Map* map_extend(Map* map) {
    size_t current_capacity = map->capacity;
    size_t new_capacity = 2*current_capacity;

    map_item_t* temp_data = map->data;
    map->data = (map_item_t*)safe_malloc(new_capacity * sizeof(map_item_t));
    map->capacity = new_capacity;

    size_t i;
    for (i = 0; i < new_capacity; i++) {
        map->data[i].key = NULL;
        map->data[i].value = NULL;
        map->data[i].state = Empty;
    }

    for (i = 0; i < current_capacity; i++) {
        if (temp_data[i].state != Filled) continue;
        int hash_index = hash_function(temp_data[i].key, new_capacity);
        while (1) {
            if (map->data[hash_index].state != Filled) break;
            hash_index = (hash_index + 1) % new_capacity;
        }
        map->data[hash_index] = temp_data[i];
    }

    free(temp_data);
    return map;
}

size_t hash_function(map_key_t key, size_t capacity) {
    size_t a = 31415, b = 27183, hash = 0; 
    char* p;
    for (p = key; *p != '\0'; p++) {
        hash = (a*hash + *p) % capacity;
        a = a * b % (capacity - 1);
    }
    return hash;
}

char* key_new(char* str) {
    char* ret = (char*)safe_malloc((strlen(str) + 1) * sizeof(char));
    strcpy(ret, str);
    return ret;
}

int has_key(map_item_t item, char* key) {
    return item.key != NULL && strcmp(item.key, key) == 0;
}
