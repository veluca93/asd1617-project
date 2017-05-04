#include "dynstring.h"
#include <stdlib.h>

dynstring_t* dyns_create() {
    dynstring_t* res = malloc(sizeof(dynstring_t));
    res->size = 0;
    res->capacity = 1;
    res->data = malloc(res->capacity);
    res->data[0] = 0;
    return res;
}

void dyns_append(dynstring_t* str, char el) {
    if (str->size == str->capacity - 1) {
        str->capacity *= 2;
        str->data = realloc(str->data, str->capacity);
    }
    str->data[str->size++] = el;
    str->data[str->size] = 0;
}

void dyns_truncate(dynstring_t* str, unsigned length) {
    str->size = length;
    str->data[length] = 0;
}

const char* dyns_cstr(const dynstring_t* str) {
    return str->data;
}

unsigned dyns_size(const dynstring_t* str) {
    return str->size;
}

char dyns_get(const dynstring_t* str, unsigned pos) {
    return str->data[pos];
}

void dyns_free(dynstring_t* str) {
    free(str->data);
    free(str);
}

