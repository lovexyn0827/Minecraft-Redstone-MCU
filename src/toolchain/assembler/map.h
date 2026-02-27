#ifndef HASHMAP_H_INCLUDED
#define HASHMAP_H_INCLUDED

#include <stdlib.h>

#define HASH_MAP_NODE_TYPE(K, V)\
    struct {\
        void *next;\
        K key;\
        V value;\
    }

#define HASH_MAP_TYPE(K, V)\
    struct {\
        HASH_MAP_NODE_TYPE(K, V) **slots;\
        uint_t size;\
        uint_t (*hash)(K);\
        V nil;\
    }

#define HASH_MAP_INIT(K, V, NAME, SIZE, HASH, NIL) {\
    NAME.slots = (typeof(NAME.slots)) malloc(SIZE * sizeof(HASH_MAP_NODE_TYPE(K, V)*));\
    if (NAME.slots == NULL) {\
        fatal("Failed to create map!");\
    }\
    NAME.size = SIZE;\
    NAME.hash = HASH;\
    NAME.nil = NIL;\
    for (int __i_map = 0; __i_map < SIZE; __i_map++) {\
        NAME.slots[__i_map] = NULL;\
    }\
}

#define HASH_MAP_PUT(M, Kval, Vval, K, V, CMP) {\
    bool blackhole;\
    HASH_MAP_PUT_RET(M, Kval, Vval, K, V, CMP, blackhole)\
    blackhole ^= true;\
}\

#define HASH_MAP_PUT_RET(M, Kval, Vval, K, V, CMP, R) {\
    K __key_backup_map = Kval;\
    V __val_backup_map = Vval;\
    uint_t slot_no = M.hash(__key_backup_map) % M.size;\
    typeof(M.slots[slot_no]) node = M.slots[slot_no];\
    while (node != NULL) {\
        if (CMP(node->key, __key_backup_map)) {\
            break;\
        }\
        node = (typeof(node)) (node->next);\
    }\
    if (node == NULL) {\
        typeof(node) new_node = (typeof(node)) malloc(sizeof(HASH_MAP_NODE_TYPE(K, V)));\
        if (new_node == NULL) fatal("Failed to insert to a map");\
        new_node->key = __key_backup_map;\
        new_node->value = __val_backup_map;\
        new_node->next = M.slots[slot_no];\
        M.slots[slot_no] = new_node;\
        R = true;\
    } else {\
        R = false;\
    }\
}

#define HASH_MAP_GET(M, Kval, Vval, K, V, CMP) {\
    K __key_backup_map = Kval;\
    uint_t slot_no = M.hash(__key_backup_map) % M.size;\
    typeof(M.slots[slot_no]) node = M.slots[slot_no];\
    while (node != NULL) {\
        if (CMP(node->key, __key_backup_map)) {\
            Vval = node->value;\
            break;\
        }\
        node = (typeof(node)) (node->next);\
    }\
    if (node == NULL) Vval = M.nil;\
}

#define HASH_MAP_FREE(M) {\
    for (uint_t i = 0; i < M.size; i++) {\
        typeof(M.slots[i]) node = M.slots[i];\
        while (node != NULL) {\
            typeof(M.slots[i]) next = node->next;\
            free(node);\
            node = next;\
        }\
    }\
    free(M.slots);\
}

#endif // HASHMAP_H_INCLUDED
