#ifndef PARSE_OSM_H
#define PARSE_OSM_H
#include "dynarray.h"
#include <stdio.h>

typedef unsigned long osm_id_t;

typedef struct osm_tag {
    // See http://wiki.openstreetmap.org/wiki/Tags for details
    char* key;
    char* value;
} osm_tag_t;

typedef struct osm_node {
    // See http://wiki.openstreetmap.org/wiki/Node for details
    osm_id_t id;            // Node id
    double lat, lon;        // Latitude and longitude at which the node is located
    dynarray_t* tags;       // Array of tags (osm_tag_t) associated to this node
} osm_node_t;

typedef struct osm_way {
    // See http://wiki.openstreetmap.org/wiki/Way for details
    osm_id_t id;            // Way id
    dynarray_t* node_ids;   // Array of ids (osm_id_t) of the nodes this way is composed of
    dynarray_t* tags;       // Array of tags (osm_tag_t) associated to this way
} osm_way_t;

typedef struct osm_rel_member {
    osm_id_t id;            // Id of the member of the relation
    char* type;             // Type of the member of the relation (eg. "way", "node")
    char* role;             // Optional role of the member
} osm_rel_member_t;

typedef struct osm_relation {
    // See http://wiki.openstreetmap.org/wiki/Relation for details
    osm_id_t id;            // Relation id
    dynarray_t* members;    // Array of members (osm_rel_member_t) that are part of this relation
    dynarray_t* tags;       // Array of tags (osm_tag_t) associated to this relation
} osm_relation_t;

typedef struct osm_map {
    dynarray_t* nodes;      // All the nodes that form this map
    dynarray_t* ways;       // All the ways that compose this map
    dynarray_t* relations;  // All the relations that compose this map
} osm_map_t;

osm_map_t* osm_parse(FILE* file);
void osm_free_map(osm_map_t* map);

#endif
