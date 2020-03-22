#include <assert.h>
#include <stdio.h>
#include "../src/mincc_map.h"
#include "../src/mincc_memory.h"


int* safe_malloc_int(int value) {
    int* ptr = (int*)safe_malloc(sizeof(int));
    *ptr = value;
    return ptr;
}

int main() {
    Map* map = map_new();
    int* ptr = NULL;

    map_insert(map, "abc", safe_malloc_int(5));
    ptr = (int*)map_find(map, "abc");
    assert(*ptr == 5);
    assert(map->size == 1);

    map_insert(map, "xyz", safe_malloc_int(-3));
    ptr = (int*)map_find(map, "xyz");
    assert(*ptr == -3);
    assert(map->size == 2);

    map_insert(map, "abc", safe_malloc_int(2));
    ptr = (int*)map_find(map, "abc");
    assert(*ptr == 2);
    assert(map->size == 2);

    map_insert(map, "pqr", safe_malloc_int(7));
    ptr = (int*)map_find(map, "pqr");
    assert(*ptr == 7);
    assert(map->size == 3);

    ptr = (int*)map_find(map, "abc");
    assert(*ptr == 2);
    ptr = (int*)map_find(map, "xyz");
    assert(*ptr == -3);

    map_delete(map);

    fprintf(stdout, "OK\n");
    return 0;
}
