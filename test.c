#include "parse_osm.h"

int main() {
    osm_map_t* map = osm_parse(stdin);
    unsigned i;
    for (i=0; i<dyn_size(map->nodes); i++) {
        osm_node_t* node = dyn_get(map->nodes, i);
        printf("%lu (%lf, %lf)", node->id, node->lat, node->lon);
        unsigned j=0;
        for (j=0; j<dyn_size(node->tags); j++) {
            osm_tag_t* tag = dyn_get(node->tags, j);
            printf(" %s='%s'", tag->key, tag->value);
        }
        printf("\n");
    }
    osm_free_map(map);
    return 0;
}
