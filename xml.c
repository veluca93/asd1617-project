#include "xml.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// TODO: parse &amp; etc
static xml_attribute_t* xml_create_attribute(const char* key, const char* value) {
    xml_attribute_t* res = malloc(sizeof(xml_attribute_t));
    res->key = malloc(strlen(key)+1);
    strncpy(res->key, key, strlen(key)+1);
    res->value = malloc(strlen(value)+1);
    strncpy(res->value, value, strlen(value)+1);
    return res;
}

static xml_tag_t* xml_create_tag(const char* name) {
    xml_tag_t* res = malloc(sizeof(xml_tag_t));
    res->name = malloc(strlen(name)+1);
    strncpy(res->name, name, strlen(name)+1);
    res->attributes = dyn_create();
    res->children = dyn_create();
    res->content = dyns_create();
    return res;
}

static void xml_free_attribute(xml_attribute_t* att) {
    free(att->key);
    free(att->value);
    free(att);
}

static void xml_free_tag(xml_tag_t* tag) {
    unsigned i;
    for (i=0; i<dyn_size(tag->attributes); i++) {
        xml_free_attribute(dyn_get(tag->attributes, i));
    }
    dyn_free(tag->attributes);
    for (i=0; i<dyn_size(tag->children); i++) {
        xml_free_tag(dyn_get(tag->children, i));
    }
    dyn_free(tag->children);
    dyns_free(tag->content);
    free(tag);
}

#define ADVANCE(what, cond) while ( (read_char = fgetc(in)) != EOF && (cond)) { \
                    { what } \
                } \
                if (read_char == EOF) { \
                    fprintf(stderr, "Unexpected end of file.\n"); \
                    break; \
                }

void xml_parse(FILE* in, void (*tag_close_cb)(void*, const xml_tag_t*), void* cb_arg) {
    dynstring_t* namebuf = dyns_create();
    dynstring_t* keybuf = dyns_create();
    dynstring_t* valuebuf = dyns_create();
    dynarray_t* tagstack = dyn_create();
    int read_char;
    do {
        read_char = fgetc(in);
        if (read_char == EOF) break;
        if (read_char == '<') { // Parse tag
            dyns_truncate(namebuf, 0);
            dyns_append(namebuf, fgetc(in));
            // Don't do this at home
            ADVANCE(dyns_append(namebuf, read_char);, isalpha((unsigned char)read_char));
            ungetc(read_char, in);
            if (dyns_cstr(namebuf)[0] == '?') { // "xml" tag, ignore
                ADVANCE(, read_char != '>');
                continue;
            }
            if (dyns_cstr(namebuf)[0] == '/') { // close tag
                if (dyn_size(tagstack) == 0) {
                    fprintf(stderr, "Unexpected close tag.\n");
                    break;
                }
                xml_tag_t* bottom = dyn_pop(tagstack);
                if (strcmp(bottom->name, dyns_cstr(namebuf)+1) != 0) {
                    fprintf(stderr, "Closed a tag that is not open (%s, %s).\n", bottom->name, dyns_cstr(namebuf)+1);
                    break;
                }
                tag_close_cb(cb_arg, bottom);
                if (dyn_size(tagstack) != 0) {
                   xml_tag_t* parent = dyn_back(tagstack);
                   dyn_append(parent->children, bottom);
                } else {
                    xml_free_tag(bottom);
                }
                continue;
            }
            // Read attributes
            xml_tag_t* tag = xml_create_tag(dyns_cstr(namebuf));
            do {
                ADVANCE(, !isalpha((unsigned char)read_char) && read_char != '/' && read_char != '>');
                if (read_char == '/') { // Tag with no children nor content
                    tag_close_cb(cb_arg, tag);
                    if (dyn_size(tagstack) != 0) {
                       xml_tag_t* parent = dyn_back(tagstack);
                       dyn_append(parent->children, tag);
                    }
                    break;
                }
                if (read_char == '>') { // Opened tag
                    dyn_append(tagstack, tag);
                    break;
                }
                dyns_truncate(keybuf, 0);
                dyns_append(keybuf, read_char);
                ADVANCE(dyns_append(keybuf, read_char);, read_char != '=');
                read_char = fgetc(in);
                if (read_char != '"') {
                    fprintf(stderr, "Invalid attribute value!\n");
                    read_char = EOF;
                    break;
                }
                dyns_truncate(valuebuf, 0);
                ADVANCE(if (read_char == '\\') read_char = fgetc(in); dyns_append(valuebuf, read_char);, read_char != '"');
                dyn_append(tag->attributes, xml_create_attribute(dyns_cstr(keybuf), dyns_cstr(valuebuf)));
            } while (read_char != EOF);
            continue;
        }
        if (dyn_size(tagstack) > 0) {
            xml_tag_t* tg = dyn_back(tagstack);
            dyns_append(tg->content, read_char);
            continue;
        }
    } while (read_char != EOF);
}

