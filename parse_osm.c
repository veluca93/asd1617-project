#include "parse_osm.h"
#include "xml.h"
#include <stdlib.h>
#include <string.h>

static void osm_free_tag(osm_tag_t* tag) {
    free(tag->key);
    free(tag->value);
    free(tag);
}

static void osm_free_node(osm_node_t* node) {
    unsigned i = 0;
    for (i=0; i<dyn_size(node->tags); i++) {
        osm_free_tag(dyn_get(node->tags, i));
    }
    free(node);
}

static void osm_free_way(osm_way_t* way) {
    unsigned i = 0;
    for (i=0; i<dyn_size(way->node_ids); i++) {
        free(dyn_get(way->node_ids, i));
    }
    for (i=0; i<dyn_size(way->tags); i++) {
        osm_free_tag(dyn_get(way->tags, i));
    }
    free(way);
}

static void osm_free_rel_member(osm_rel_member_t* rmemb) {
    free(rmemb->role);
    free(rmemb->type);
    free(rmemb);
}

static void osm_free_relation(osm_relation_t* relation) {
    unsigned i = 0;
    for (i=0; i<dyn_size(relation->members); i++) {
        osm_free_rel_member(dyn_get(relation->members, i));
    }
    for (i=0; i<dyn_size(relation->tags); i++) {
        osm_free_tag(dyn_get(relation->tags, i));
    }
    free(relation);
}

void osm_free_map(osm_map_t* map) {
    unsigned i=0;
    for (i=0; i<dyn_size(map->nodes); i++) {
        osm_free_node(dyn_get(map->nodes, i));
    }
    dyn_free(map->nodes);
    for (i=0; i<dyn_size(map->ways); i++) {
        osm_free_way(dyn_get(map->ways, i));
    }
    dyn_free(map->ways);
    for (i=0; i<dyn_size(map->relations); i++) {
        osm_free_relation(dyn_get(map->relations, i));
    }
    dyn_free(map->relations);
    free(map);
}

static osm_tag_t* osm_tag_from_xml_tag(const xml_tag_t* xt) {
    unsigned i = 0;
    const char* key = NULL;
    const char* value = NULL;
    for (i=0; i<dyn_size(xt->attributes); i++) {
        xml_attribute_t* attr = dyn_get(xt->attributes, i);
        if (strcmp(attr->key, "k") == 0) key = attr->value;
        if (strcmp(attr->key, "v") == 0) value = attr->value;
    }
    if (key == NULL || value == NULL) {
        fprintf(stderr, "<tag> element without k or v attribute!\n");
        return NULL;
    }
    osm_tag_t* res = malloc(sizeof(osm_tag_t));
    res->key = malloc(strlen(key)+1);
    strncpy(res->key, key, strlen(key)+1);
    res->value = malloc(strlen(value)+1);
    strncpy(res->value, value, strlen(value)+1);
    return res;
}

static osm_rel_member_t* osm_rel_member_from_xml_tag(const xml_tag_t* memb) {
    unsigned i = 0;
    const char* id = NULL;
    const char* type = NULL;
    const char* role = NULL;
    for (i=0; i<dyn_size(memb->attributes); i++) {
        xml_attribute_t* attr = dyn_get(memb->attributes, i);
        if (strcmp(attr->key, "type") == 0) type = attr->value;
        if (strcmp(attr->key, "ref") == 0) id = attr->value;
        if (strcmp(attr->key, "role") == 0) role = attr->value;
    }
    if (id == NULL || type == NULL) {
        fprintf(stderr, "<member> element without ref or type attribute!\n");
        return NULL;
    }
    if (role == NULL) role = "";
    osm_rel_member_t* res = malloc(sizeof(osm_rel_member_t));
    char* end = NULL;
    res->id = strtoul(id, &end, 10);
    if (*end != 0) {
        fprintf(stderr, "<member> element with invalid ref (%s)!\n", id);
        free(res);
        return NULL;
    }
    res->type = malloc(strlen(type)+1);
    strncpy(res->type, type, strlen(type)+1);
    res->role = malloc(strlen(role)+1);
    strncpy(res->role, role, strlen(role)+1);
    return res;
}

static void osm_parse_cb(void* data, const xml_tag_t* tag) {
    unsigned i=0;
    unsigned found_attrs = 0;
    osm_map_t* map = data;
    // Parse a node
    if (strcmp(tag->name, "node") == 0) {
        osm_node_t* nd = malloc(sizeof(osm_node_t));
        nd->tags = dyn_create();
        // Handle attributes
        for (i=0; i<dyn_size(tag->attributes); i++) {
            xml_attribute_t* attr = dyn_get(tag->attributes, i);
            char* end = NULL;
            if (strcmp(attr->key, "id") == 0) {
                found_attrs++;
                nd->id = strtoul(attr->value, &end, 10);
                if (*end != 0) {
                    fprintf(stderr, "<node> element with invalid id (%s)!\n", attr->value);
                    free(nd);
                    return;
                }
            }
            if (strcmp(attr->key, "lat") == 0) {
                found_attrs++;
                nd->lat = strtod(attr->value, &end);
                if (*end != 0) {
                    fprintf(stderr, "<node> element with invalid latitude (%s)!\n", attr->value);
                    free(nd);
                    return;
                }
            }
            if (strcmp(attr->key, "lon") == 0) {
                found_attrs++;
                nd->lon = strtod(attr->value, &end);
                if (*end != 0) {
                    fprintf(stderr, "<node> element with invalid longitude (%s)!\n", attr->value);
                    free(nd);
                    return;
                }
            }
        }
        if (found_attrs != 3) {
            fprintf(stderr, "<node> element with missing attributes!\n");
            free(nd);
            return;
        }
        // Handle children
        for (i=0; i<dyn_size(tag->children); i++) {
            xml_tag_t* tg = dyn_get(tag->children, i);
            if (strcmp(tg->name, "tag") == 0) {
                osm_tag_t* val = osm_tag_from_xml_tag(tg);
                if (val != NULL) dyn_append(nd->tags, val);
            }
        }
        dyn_append(map->nodes, nd);
    }
    // Parse a way
    if (strcmp(tag->name, "way") == 0) {
        osm_way_t* nd = malloc(sizeof(osm_way_t));
        nd->tags = dyn_create();
        nd->node_ids = dyn_create();
        // Handle attributes
        for (i=0; i<dyn_size(tag->attributes); i++) {
            xml_attribute_t* attr = dyn_get(tag->attributes, i);
            if (strcmp(attr->key, "id") == 0) {
                char* end = NULL;
                found_attrs++;
                nd->id = strtoul(attr->value, &end, 10);
                if (*end != 0) {
                    fprintf(stderr, "<way> element with invalid id (%s)!\n", attr->value);
                    free(nd);
                    return;
                }
            }
        }
        if (found_attrs != 1) {
            fprintf(stderr, "<way> element with missing id!\n");
            free(nd);
            return;
        }
        // Handle children
        for (i=0; i<dyn_size(tag->children); i++) {
            xml_tag_t* tg = dyn_get(tag->children, i);
            if (strcmp(tg->name, "tag") == 0) {
                osm_tag_t* val = osm_tag_from_xml_tag(tg);
                if (val != NULL) dyn_append(nd->tags, val);
            }
            if (strcmp(tg->name, "nd") == 0) {
                found_attrs = 0;
                unsigned j=0;
                osm_id_t child_id = 0;
                for (j=0; j<dyn_size(tg->attributes); j++) {
                    xml_attribute_t* attr = dyn_get(tg->attributes, j);
                    if (strcmp(attr->key, "ref") == 0) {
                        char* end = NULL;
                        found_attrs++;
                        child_id = strtoul(attr->value, &end, 10);
                        if (*end != 0) {
                            fprintf(stderr, "<nd> element with invalid id (%s)!\n", attr->value);
                            free(nd);
                            return;
                        }
                    }
                }
                if (found_attrs != 1) {
                    fprintf(stderr, "<nd> element with missing ref!\n");
                    osm_free_way(nd);
                    return;
                }
                osm_id_t* chld = malloc(sizeof(osm_id_t));
                *chld = child_id;
                dyn_append(nd->node_ids, chld);
            }
        }
        dyn_append(map->ways, nd);
    }
    // Parse a relation
    if (strcmp(tag->name, "relation") == 0) {
        osm_relation_t* nd = malloc(sizeof(osm_relation_t));
        nd->members = dyn_create();
        nd->tags = dyn_create();
        // Handle attributes
        for (i=0; i<dyn_size(tag->attributes); i++) {
            xml_attribute_t* attr = dyn_get(tag->attributes, i);
            if (strcmp(attr->key, "id") == 0) {
                char* end = NULL;
                found_attrs++;
                nd->id = strtoul(attr->value, &end, 10);
                if (*end != 0) {
                    fprintf(stderr, "<relation> element with invalid id (%s)!\n", attr->value);
                    free(nd);
                    return;
                }
            }
        }
        if (found_attrs != 1) {
            fprintf(stderr, "<relation> element with missing id!\n");
            free(nd);
            return;
        }
        // Handle children
        for (i=0; i<dyn_size(tag->children); i++) {
            xml_tag_t* tg = dyn_get(tag->children, i);
            if (strcmp(tg->name, "tag") == 0) {
                osm_tag_t* val = osm_tag_from_xml_tag(tg);
                if (val != NULL) dyn_append(nd->tags, val);
            }
            if (strcmp(tg->name, "member") == 0) {
                osm_rel_member_t* val = osm_rel_member_from_xml_tag(tg);
                if (val != NULL) dyn_append(nd->members, val);
            }
        }
        dyn_append(map->relations, nd);
    }
#ifdef VERBOSE
    if (strcmp(tag->name, "note") == 0) {
        fprintf(stderr, "Note: %s\n", dyns_cstr(tag->content));
    }
#endif
}


osm_map_t* osm_parse(FILE* file) {
    osm_map_t* res = malloc(sizeof(osm_map_t));
    res->nodes = dyn_create();
    res->ways = dyn_create();
    res->relations = dyn_create();
    xml_parse(file, osm_parse_cb, res);
    return res;
}
