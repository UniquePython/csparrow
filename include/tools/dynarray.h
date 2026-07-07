#ifndef TOOLS_DYNARRAY_H_
#define TOOLS_DYNARRAY_H_

// Generic, type-safe, growable dynamic array.
//
// Usage:
//
//   DYNARRAY_DEFINE(int, IntArray)
//
//   generates an `IntArray` type plus a full set of `IntArray_*`
//   functions (all `static inline`, safe to `DYNARRAY_DEFINE` the same
//   (T, Name) pair in multiple translation units without linker
//   collisions -- same convention as HASHMAP_DEFINE in hashmap.h).
//
// The array OWNS copies of its elements: `_push` copies the element by
// value into array-owned storage. For pointer element types (e.g.
// char*), this copies the pointer itself, not the pointee -- same
// ownership model as hashmap.h. If elements need their own cleanup
// (e.g. free(*elem) for owned char* elements), supply an element
// destructor at `_new` time (may be NULL):
//
//   void IntArray_elemDtor(int *elem)   // optional, may be NULL
//
// invoked on every remaining element when `_free`/`_clear` runs, and
// on the removed element for `_pop`/`_removeAt`.
//
// Example:
//
//   DYNARRAY_DEFINE(char*, StrArray)
//   static void freeStr(char **s) { free(*s); }
//
//   StrArray *a = StrArray_new(freeStr);
//   StrArray_push(a, SparrowStrClone("hello", 5));
//   char **first = StrArray_at(a, 0);
//   StrArray_free(a);

#include "tools/allocator.h"
#include "tools/ckdarith.h"
#include "tools/fatal.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define DYNARRAY_DEFAULT_CAPACITY 8

#define DYNARRAY_DEFINE(T, Name)                                            \
                                                                            \
    typedef void (*Name##ElemDtorFn)(T * elem);                             \
                                                                            \
    typedef struct Name                                                     \
    {                                                                       \
        T *items;                                                           \
        size_t size;                                                        \
        size_t capacity;                                                    \
        Name##ElemDtorFn elemDtor; /* may be NULL */                        \
    } Name;                                                                 \
                                                                            \
    static inline Name *Name##_new(Name##ElemDtorFn elemDtor)               \
    {                                                                       \
        Name *arr = SparrowMalloc(sizeof(Name));                            \
        arr->items = SparrowMalloc(DYNARRAY_DEFAULT_CAPACITY * sizeof(T));  \
        arr->size = 0;                                                      \
        arr->capacity = DYNARRAY_DEFAULT_CAPACITY;                          \
        arr->elemDtor = elemDtor;                                           \
        return arr;                                                         \
    }                                                                       \
                                                                            \
    static inline void Name##_clear(Name *arr)                              \
    {                                                                       \
        if (arr->elemDtor != NULL)                                          \
        {                                                                   \
            for (size_t i = 0; i < arr->size; i++)                          \
                arr->elemDtor(&arr->items[i]);                              \
        }                                                                   \
        arr->size = 0;                                                      \
    }                                                                       \
                                                                            \
    static inline void Name##_free(Name *arr)                               \
    {                                                                       \
        if (arr == NULL)                                                    \
            return;                                                         \
                                                                            \
        Name##_clear(arr);                                                  \
        free(arr->items);                                                   \
        free(arr);                                                          \
    }                                                                       \
                                                                            \
    static inline void Name##_reserve(Name *arr, size_t newCapacity)        \
    {                                                                       \
        if (newCapacity <= arr->capacity)                                   \
            return;                                                         \
                                                                            \
        size_t newSize;                                                     \
        SPARROW_CHECKED_MUL(newCapacity, sizeof(T), &newSize);              \
                                                                            \
        arr->items = SparrowRealloc(arr->items, newSize);                   \
        arr->capacity = newCapacity;                                        \
    }                                                                       \
                                                                            \
    static inline void Name##_maybeGrow(Name *arr)                          \
    {                                                                       \
        if (arr->size < arr->capacity)                                      \
            return;                                                         \
                                                                            \
        size_t newCapacity;                                                 \
        SPARROW_CHECKED_MUL(arr->capacity, 2, &newCapacity);                \
        Name##_reserve(arr, newCapacity);                                   \
    }                                                                       \
                                                                            \
    /* Appends a copy of `value` to the end of the array. */                \
    static inline void Name##_push(Name *arr, T value)                      \
    {                                                                       \
        Name##_maybeGrow(arr);                                              \
        arr->items[arr->size] = value;                                      \
        arr->size++;                                                        \
    }                                                                       \
                                                                            \
    /* Removes and returns the last element by value. Aborts (via           \
       SparrowFatal, see tools/fatal.h) if the array is empty -- callers    \
       should check `_size(arr) > 0` first if emptiness is expected.        \
       NOTE: if elemDtor is set, it is NOT called here, since the           \
       element is being handed back to the caller, not destroyed; the       \
       caller now owns whatever the popped value points to. */              \
    static inline T Name##_pop(Name *arr)                                   \
    {                                                                       \
        if (arr->size == 0)                                                 \
            SparrowFatal("pop on empty " #Name);                            \
                                                                            \
        arr->size--;                                                        \
        return arr->items[arr->size];                                       \
    }                                                                       \
                                                                            \
    /* Bounds-checked pointer to the element at `index`, or NULL if out     \
       of range. The pointer is invalidated by any subsequent _push/        \
       _reserve/_removeAt/_free call that may reallocate `items`. */        \
    static inline T *Name##_at(const Name *arr, size_t index)               \
    {                                                                       \
        if (index >= arr->size)                                             \
            return NULL;                                                    \
        return &arr->items[index];                                          \
    }                                                                       \
                                                                            \
    /* Copies the element at `index` into `*out`. Returns false (and        \
       leaves *out untouched) if index is out of range. */                  \
    static inline bool Name##_get(const Name *arr, size_t index, T *out)    \
    {                                                                       \
        if (index >= arr->size)                                             \
            return false;                                                   \
        *out = arr->items[index];                                           \
        return true;                                                        \
    }                                                                       \
                                                                            \
    /* Overwrites the element at `index`, running elemDtor on the old       \
       value first if set. Returns false (does nothing) if out of range. */ \
    static inline bool Name##_set(Name *arr, size_t index, T value)         \
    {                                                                       \
        if (index >= arr->size)                                             \
            return false;                                                   \
                                                                            \
        if (arr->elemDtor != NULL)                                          \
            arr->elemDtor(&arr->items[index]);                              \
                                                                            \
        arr->items[index] = value;                                          \
        return true;                                                        \
    }                                                                       \
                                                                            \
    /* Removes the element at `index`, shifting subsequent elements down    \
       (O(n)). Runs elemDtor on the removed element if set. Returns false   \
       (does nothing) if out of range. */                                   \
    static inline bool Name##_removeAt(Name *arr, size_t index)             \
    {                                                                       \
        if (index >= arr->size)                                             \
            return false;                                                   \
                                                                            \
        if (arr->elemDtor != NULL)                                          \
            arr->elemDtor(&arr->items[index]);                              \
                                                                            \
        size_t tailCount = arr->size - index - 1;                           \
        if (tailCount > 0)                                                  \
            memmove(&arr->items[index], &arr->items[index + 1],             \
                    tailCount * sizeof(T));                                 \
                                                                            \
        arr->size--;                                                        \
        return true;                                                        \
    }                                                                       \
                                                                            \
    static inline size_t Name##_size(const Name *arr)                       \
    {                                                                       \
        return arr->size;                                                   \
    }                                                                       \
                                                                            \
    static inline size_t Name##_capacity(const Name *arr)                   \
    {                                                                       \
        return arr->capacity;                                               \
    }                                                                       \
                                                                            \
    static inline bool Name##_isEmpty(const Name *arr)                      \
    {                                                                       \
        return arr->size == 0;                                              \
    }                                                                       \
                                                                            \
    /* Calls fn(elem, index, userData) for every element, in order.         \
       `fn` must not mutate the array (no push/pop/removeAt/free from       \
       within the callback). */                                             \
    static inline void Name##_forEach(const Name *arr,                      \
                                      void (*fn)(T * elem, size_t index,    \
                                                 void *userData),           \
                                      void *userData)                       \
    {                                                                       \
        for (size_t i = 0; i < arr->size; i++)                              \
            fn(&arr->items[i], i, userData);                                \
    }

#endif