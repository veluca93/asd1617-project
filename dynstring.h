#ifndef DYNSTRING_H
#define DYNSTRING_H

typedef struct dynstr {
    char* data;
    unsigned size;
    unsigned capacity;
} dynstring_t;

dynstring_t* dyns_create();
void dyns_append(dynstring_t* str, char el);
void dyns_truncate(dynstring_t* str, unsigned length);
const char* dyns_cstr(const dynstring_t* str);
unsigned dyns_size(const dynstring_t* str);
char dyns_get(const dynstring_t* str, unsigned pos);
void dyns_free(dynstring_t* str);

#endif
