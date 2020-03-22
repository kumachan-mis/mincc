#include <assert.h>
#include <stdio.h>
#include "../src/mincc_map.h"
#include "../src/mincc_memory.h"


int main() {
    Map* map = map_new();
    int* ptr = NULL;

    map_insert(map, "abc", int_new(5));
    ptr = (int*)map_find(map, "abc");
    assert(*ptr == 5);
    assert(map->size == 1);

    map_insert(map, "xyz", int_new(-3));
    ptr = (int*)map_find(map, "xyz");
    assert(*ptr == -3);
    assert(map->size == 2);

    map_insert(map, "abc", int_new(2));
    ptr = (int*)map_find(map, "abc");
    assert(*ptr == 2);
    assert(map->size == 2);

    map_insert(map, "pqr", int_new(7));
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
