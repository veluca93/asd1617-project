#include "dynarray.h"
#include <stdlib.h>

dynarray_t* dyn_create() {
    dynarray_t* res = malloc(sizeof(dynarray_t));
    res->size = 0;
    res->capacity = 1;
    res->data = malloc(sizeof(void*)*res->capacity);
    return res;
}

unsigned dyn_size(dynarray_t* arr) {
    return arr->size;
}

void dyn_append(dynarray_t* arr, void* el) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(void*)*arr->capacity);
    }
    dyn_set(arr, arr->size++, el);
}

void* dyn_pop(dynarray_t* arr) {
    void* res = dyn_back(arr);
    arr->size--;
    return res;
}

void* dyn_back(dynarray_t* arr) {
    return dyn_get(arr, arr->size-1);
}

void dyn_set(dynarray_t* arr, unsigned pos, void* el) {
    arr->data[pos] = el;    
}

void* dyn_get(dynarray_t* arr, unsigned pos) {
    return arr->data[pos]; 
}

void dyn_free(dynarray_t* arr) {
    free(arr->data);
    free(arr);
}
