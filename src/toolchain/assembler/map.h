#ifndef HASHMAP_H_INCLUDED
#define HASHMAP_H_INCLUDED

#define HASH_MAP_NODE_TYPE(K, V)\
    struct {\
        void *next;\
        K key;\
        V value;\
    }

#define HASH_MAP_TYPE(K, V)\
    struct {\
        HASH_MAP_NODE_TYPE(K, V) **slots;\
        uint32_t size;\
        uint32_t (*hash)(K);\
        V nil;\
    }

#define HASH_MAP_INIT(K, V, NAME, SIZE, HASH, NIL)\
    MAP_TYPE(K, V) NAME;\
    NAME.slots = (HASH_MAP_NODE_TYPE(K, V)**) malloc(SIZE * sizeof(HASH_MAP_NODE_TYPE(K, V)));\
    NAME.size = SIZE;\
    NAME.hash = HASH;\
    NAME.nil = NIL;\
    for (int __i_map = 0; __i_map < SIZE; __i_map++) {\
        NAME.slots[__i_map] = NULL;\
    }\

#define HASH_MAP_PUT(M, Kval, Vval, K, V)\
    K __key_kackup_map = Kval;\
    uint32_t slot_no = M.hash(__key_kackup_map) % M.size;\
    HASH_MAP_NODE_TYPE(K, V) *node = M.slots[slot_no];\
    M.slots[slot_no] = (HASH_MAP_NODE_TYPE(K, V) *) malloc(sizeof(HASH_MAP_NODE_TYPE(K, V)));\
    if (M.slots[slot_no] == NULL) error();\
    M.slots[slot_no].next = node;\
    M.slots[slot_no].key = __key_kackup_map;\
    M.slots[slot_no].value = Vval;

#define HASH_MAP_GET(M, Kval, Vval, K, V)\
    K __key_kackup_map = Kval;\
    HASH_MAP_NODE_TYPE(K, V) *node = M.slots[M.hash(__key_kackup_map) % M.size];\
    while (node != NULL) {\
        if (node->key == __key_kackup_map) Vval = node->value; break;\
        node = (HASH_MAP_NODE_TYPE(K, V)*) (node->next);\
    }\
    if (node == NULL) Vval = M.nil;

#endif // HASHMAP_H_INCLUDED
