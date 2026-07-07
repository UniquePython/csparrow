#ifndef TOOLS_HASHMAP_H_
#define TOOLS_HASHMAP_H_

// Generic, type-safe, chained hashmap.
//
// Usage:
//
//   HASHMAP_DEFINE(char*, int, StrIntMap)
//
//   generates a `StrIntMap` type plus a full set of `StrIntMap_*`
//   functions (declarations + implementations, all `static inline`
//   for the header and `static` for the bodies, so it's safe to
//   `HASHMAP_DEFINE` the same (K, V, Name) triple in multiple
//   translation units without linker collisions).
//
// The map OWNS copies of both keys and values: `_put` memcpy's the
// key/value into map-owned storage. For pointer types (e.g. char*),
// this copies the pointer itself, not the pointee -- the map does not
// know how to deep-copy arbitrary types. If you need the map to own
// the pointed-to data too, clone it yourself before calling `_put`
// (see SparrowStrClone) and free it via the `_free`/`_remove`
// callbacks (see below).
//
// You supply a hash function and an equality function at `_new` time:
//
//   uint64_t StrIntMap_hash(char *const *key)   // hash a key
//   bool     StrIntMap_eq(char *const *a, char *const *b)
//
// (Both take POINTERS to the key type, since keys of arbitrary size
// must be passed/compared generically. These parameters are NOT
// `const`-qualified at the pointer-to-key level -- see the comment
// above the typedefs in this header for why: `const K*` does not
// mean what you'd expect when K is itself a pointer type. Treat the
// pointee as read-only by convention; the map never mutates through
// these pointers.)
//
// Optionally supply per-entry destructors (may be NULL) invoked when
// an entry is removed, replaced, or the whole map is freed -- this is
// how you free owned key/value payloads (e.g. free(*key) if K is
// char*, since the map only frees its OWN copy of the pointer, not
// whatever it points to).
//
//   void StrIntMap_keyDtor(char **key)   // optional, may be NULL
//   void StrIntMap_valDtor(int *val)     // optional, may be NULL
//
// Example:
//
//   static uint64_t hashStr(char *const *k) { ... }
//   static bool eqStr(char *const *a, char *const *b) {
//       return strcmp(*a, *b) == 0;
//   }
//   static void freeKey(char **k) { free(*k); }
//
//   StrIntMap *m = StrIntMap_new(hashStr, eqStr, freeKey, NULL);
//   StrIntMap_put(m, someOwnedCString, 42);
//   int *v = StrIntMap_get(m, "lookup key");
//   StrIntMap_remove(m, "lookup key");
//   StrIntMap_free(m);

#include "tools/allocator.h"
#include "tools/ckdarith.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_DEFAULT_CAPACITY 16
#define HASHMAP_MAX_LOAD_FACTOR_NUM 3 // grow when load factor > 3/4
#define HASHMAP_MAX_LOAD_FACTOR_DEN 4

#define HASHMAP_DEFINE(K, V, Name)                                             \
                                                                               \
    typedef struct Name##Entry                                                 \
    {                                                                          \
        K key;                                                                 \
        V value;                                                               \
        uint64_t hash;                                                         \
        struct Name##Entry *next;                                              \
    } Name##Entry;                                                             \
                                                                               \
    /* NOTE: these typedefs intentionally take non-const `K *`, not            \
       `const K *`. If K is itself a pointer type (e.g. char*), `const K*`     \
       expands to `char *const *`, which is a different, incompatible          \
       type from the more natural `const char **` / `const char *const *`      \
       a user would otherwise write. Leaving these non-const lets user         \
       hash/eq functions take whichever const-qualification they want on       \
       the pointee; the map itself never mutates through these pointers. */    \
    typedef uint64_t (*Name##HashFn)(K * key);                                 \
    typedef bool (*Name##EqFn)(K * a, K * b);                                  \
    typedef void (*Name##KeyDtorFn)(K * key);                                  \
    typedef void (*Name##ValDtorFn)(V * value);                                \
                                                                               \
    typedef struct Name                                                        \
    {                                                                          \
        Name##Entry **buckets;                                                 \
        size_t bucketCount;                                                    \
        size_t size;                                                           \
        Name##HashFn hashFn;                                                   \
        Name##EqFn eqFn;                                                       \
        Name##KeyDtorFn keyDtor; /* may be NULL */                             \
        Name##ValDtorFn valDtor; /* may be NULL */                             \
    } Name;                                                                    \
                                                                               \
    static inline Name *Name##_new(Name##HashFn hashFn, Name##EqFn eqFn,       \
                                   Name##KeyDtorFn keyDtor,                    \
                                   Name##ValDtorFn valDtor)                    \
    {                                                                          \
        Name *map = SparrowMalloc(sizeof(Name));                               \
        map->buckets = SparrowCalloc(HASHMAP_DEFAULT_CAPACITY,                 \
                                     sizeof(Name##Entry *));                   \
        map->bucketCount = HASHMAP_DEFAULT_CAPACITY;                           \
        map->size = 0;                                                         \
        map->hashFn = hashFn;                                                  \
        map->eqFn = eqFn;                                                      \
        map->keyDtor = keyDtor;                                                \
        map->valDtor = valDtor;                                                \
        return map;                                                            \
    }                                                                          \
                                                                               \
    static inline void Name##_free(Name *map)                                  \
    {                                                                          \
        if (map == NULL)                                                       \
            return;                                                            \
                                                                               \
        for (size_t i = 0; i < map->bucketCount; i++)                          \
        {                                                                      \
            Name##Entry *entry = map->buckets[i];                              \
            while (entry != NULL)                                              \
            {                                                                  \
                Name##Entry *next = entry->next;                               \
                if (map->keyDtor != NULL)                                      \
                    map->keyDtor(&entry->key);                                 \
                if (map->valDtor != NULL)                                      \
                    map->valDtor(&entry->value);                               \
                free(entry);                                                   \
                entry = next;                                                  \
            }                                                                  \
        }                                                                      \
                                                                               \
        free(map->buckets);                                                    \
        free(map);                                                             \
    }                                                                          \
                                                                               \
    static inline void Name##_rehash(Name *map, size_t newBucketCount)         \
    {                                                                          \
        Name##Entry **newBuckets =                                             \
            SparrowCalloc(newBucketCount, sizeof(Name##Entry *));              \
                                                                               \
        for (size_t i = 0; i < map->bucketCount; i++)                          \
        {                                                                      \
            Name##Entry *entry = map->buckets[i];                              \
            while (entry != NULL)                                              \
            {                                                                  \
                Name##Entry *next = entry->next;                               \
                size_t idx = (size_t)(entry->hash % newBucketCount);           \
                entry->next = newBuckets[idx];                                 \
                newBuckets[idx] = entry;                                       \
                entry = next;                                                  \
            }                                                                  \
        }                                                                      \
                                                                               \
        free(map->buckets);                                                    \
        map->buckets = newBuckets;                                             \
        map->bucketCount = newBucketCount;                                     \
    }                                                                          \
                                                                               \
    static inline void Name##_maybeGrow(Name *map)                             \
    {                                                                          \
        size_t loadNum;                                                        \
        SPARROW_CHECKED_MUL(map->size, HASHMAP_MAX_LOAD_FACTOR_DEN, &loadNum); \
                                                                               \
        size_t threshold;                                                      \
        SPARROW_CHECKED_MUL(map->bucketCount, HASHMAP_MAX_LOAD_FACTOR_NUM,     \
                            &threshold);                                       \
                                                                               \
        if (loadNum > threshold)                                               \
        {                                                                      \
            size_t newBucketCount;                                             \
            SPARROW_CHECKED_MUL(map->bucketCount, 2, &newBucketCount);         \
            Name##_rehash(map, newBucketCount);                                \
        }                                                                      \
    }                                                                          \
                                                                               \
    /* Returns pointer to existing entry with matching key, or NULL. */        \
    static inline Name##Entry *Name##_findEntry(const Name *map, K *key,       \
                                                uint64_t hash)                 \
    {                                                                          \
        size_t idx = (size_t)(hash % map->bucketCount);                        \
        Name##Entry *entry = map->buckets[idx];                                \
                                                                               \
        while (entry != NULL)                                                  \
        {                                                                      \
            if (entry->hash == hash && map->eqFn(&entry->key, key))            \
                return entry;                                                  \
            entry = entry->next;                                               \
        }                                                                      \
                                                                               \
        return NULL;                                                           \
    }                                                                          \
                                                                               \
    /* Inserts, or overwrites the value (and re-runs valDtor on the old        \
       value) if the key already exists. Returns true if a new entry was       \
       inserted, false if an existing entry was overwritten. */                \
    static inline bool Name##_put(Name *map, K key, V value)                   \
    {                                                                          \
        uint64_t hash = map->hashFn(&key);                                     \
        Name##Entry *existing = Name##_findEntry(map, &key, hash);             \
                                                                               \
        if (existing != NULL)                                                  \
        {                                                                      \
            if (map->keyDtor != NULL)                                          \
                map->keyDtor(&key); /* caller's new key copy is unused */      \
            if (map->valDtor != NULL)                                          \
                map->valDtor(&existing->value);                                \
            existing->value = value;                                           \
            return false;                                                      \
        }                                                                      \
                                                                               \
        Name##_maybeGrow(map);                                                 \
                                                                               \
        size_t idx = (size_t)(hash % map->bucketCount);                        \
        Name##Entry *entry = SparrowMalloc(sizeof(Name##Entry));               \
        entry->key = key;                                                      \
        entry->value = value;                                                  \
        entry->hash = hash;                                                    \
        entry->next = map->buckets[idx];                                       \
        map->buckets[idx] = entry;                                             \
        map->size++;                                                           \
                                                                               \
        return true;                                                           \
    }                                                                          \
                                                                               \
    /* Returns a pointer to the stored value, or NULL if not found. The        \
       pointer is invalidated by any subsequent _put/_remove/_free call. */    \
    static inline V *Name##_get(const Name *map, K key)                        \
    {                                                                          \
        uint64_t hash = map->hashFn(&key);                                     \
        Name##Entry *entry = Name##_findEntry(map, &key, hash);                \
        return entry != NULL ? &entry->value : NULL;                           \
    }                                                                          \
                                                                               \
    static inline bool Name##_contains(const Name *map, K key)                 \
    {                                                                          \
        return Name##_get(map, key) != NULL;                                   \
    }                                                                          \
                                                                               \
    /* Removes the entry for `key`, running keyDtor/valDtor on the owned       \
       copies if set. Returns true if an entry was removed. */                 \
    static inline bool Name##_remove(Name *map, K key)                         \
    {                                                                          \
        uint64_t hash = map->hashFn(&key);                                     \
        size_t idx = (size_t)(hash % map->bucketCount);                        \
                                                                               \
        Name##Entry **cur = &map->buckets[idx];                                \
        while (*cur != NULL)                                                   \
        {                                                                      \
            if ((*cur)->hash == hash && map->eqFn(&(*cur)->key, &key))         \
            {                                                                  \
                Name##Entry *toRemove = *cur;                                  \
                *cur = toRemove->next;                                         \
                                                                               \
                if (map->keyDtor != NULL)                                      \
                    map->keyDtor(&toRemove->key);                              \
                if (map->valDtor != NULL)                                      \
                    map->valDtor(&toRemove->value);                            \
                                                                               \
                free(toRemove);                                                \
                map->size--;                                                   \
                return true;                                                   \
            }                                                                  \
            cur = &(*cur)->next;                                               \
        }                                                                      \
                                                                               \
        return false;                                                          \
    }                                                                          \
                                                                               \
    static inline size_t Name##_size(const Name *map)                          \
    {                                                                          \
        return map->size;                                                      \
    }                                                                          \
                                                                               \
    /* Calls fn(key, value, userData) for every entry. Order is                \
       unspecified. `fn` must not mutate the map, and should treat `key`       \
       as read-only even though it isn't `const`-qualified (see note above     \
       on why K* is left non-const throughout this header). */                 \
    static inline void Name##_forEach(const Name *map,                         \
                                      void (*fn)(K * key, V * value,           \
                                                 void *userData),              \
                                      void *userData)                          \
    {                                                                          \
        for (size_t i = 0; i < map->bucketCount; i++)                          \
        {                                                                      \
            Name##Entry *entry = map->buckets[i];                              \
            while (entry != NULL)                                              \
            {                                                                  \
                fn(&entry->key, &entry->value, userData);                      \
                entry = entry->next;                                           \
            }                                                                  \
        }                                                                      \
    }

#endif