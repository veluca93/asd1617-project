#ifndef XML_H
#define XML_H
#include "dynarray.h"
#include "dynstring.h"
#include <stdio.h>

typedef struct xml_attribute {
    char* key;
    char* value;
} xml_attribute_t;

typedef struct xml_tag {
    char* name;                 // Name of the tag
    dynarray_t* attributes;     // Array of attributes of this tag
    dynarray_t* children;       // Array of children of this tag
    dynstring_t* content;       // The content of the tag
} xml_tag_t;

void xml_parse(FILE* in, void (*tag_close_cb)(void*, const xml_tag_t*), void* cb_arg);

#endif
