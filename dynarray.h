#ifndef DYNARRAY_H
#define DYNARRAY_H

typedef struct dynarr {
    void** data;
    unsigned size;
    unsigned capacity;
} dynarray_t;

dynarray_t* dyn_create();
unsigned dyn_size(dynarray_t* arr);
void dyn_append(dynarray_t* arr, void* el);
void* dyn_pop(dynarray_t* arr);
void* dyn_back(dynarray_t* arr);
void dyn_set(dynarray_t* arr, unsigned pos, void* el);
void* dyn_get(dynarray_t* arr, unsigned pos);
void dyn_free(dynarray_t* arr);

#endif
