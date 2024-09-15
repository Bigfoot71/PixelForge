#ifndef PF_INTERNAL_VECTOR_H
#define PF_INTERNAL_VECTOR_H

#include "../pixelforge.h"

#include <string.h>
#include <stdlib.h>

typedef struct {
    void *data;         // Pointer to vector elements
    PFsizei size;       // Number of elements currently in the vector
    PFsizei capacity;   // Total vector capacity (allocated space)
    PFsizei elemSize;   // Size of an element (in bytes)
} PFIvector;


// NOTE: Undef at the end of the file
#define NEXT_POT(x) \
    ((x) == 0 ? 1 : ({ \
        unsigned n = (x); \
        if ((n & (n - 1)) == 0) n *= 2; \
        else { \
            n--; \
            n |= n >> 1; \
            n |= n >> 2; \
            n |= n >> 4; \
            n |= n >> 8; \
            n |= n >> 16; \
            n++; \
        } \
        n; \
    }))

static inline PFIvector
pfiGenVector(PFsizei capacity, PFsizei elemSize)
{
    PFIvector vec = { 0 };

    if (capacity == 0 || elemSize == 0) {
        return vec;
    }

    void *data = PF_MALLOC(capacity * elemSize);
    if (!data) return vec;

    vec.data = data;
    vec.capacity = capacity;
    vec.elemSize = elemSize;

    return vec;
}

static inline void
pfiDeleteVector(PFIvector* vec)
{
    if (vec->data) {
        PF_FREE(vec->data);
        vec->data = NULL;
    }
    vec->size = 0;
    vec->capacity = 0;
    vec->elemSize = 0;
}

static inline PFIvector
pfiCopyVector(const PFIvector* src)
{
    PFIvector vec = { 0 };

    PFsizei size_in_bytes = src->size * src->elemSize;
    if (size_in_bytes == 0) return vec;

    void* data = PF_MALLOC(size_in_bytes);
    if (!data) return vec;

    memcpy(vec.data, src->data, size_in_bytes);

    vec.size = src->size;
    vec.capacity = src->size;
    vec.elemSize = src->elemSize;

    return vec;
}

static inline PFboolean
pfiIsVectorValid(const PFIvector* vec)
{
    return vec->data != NULL
        && vec->capacity > 0
        && vec->elemSize > 0;
}

static inline PFboolean
pfiIsVectorEmpty(const PFIvector* vec)
{
    return vec->size == 0;
}

static inline PFint
pfiResizeVector(PFIvector* vec, PFsizei new_capacity)
{
    if (vec->capacity == new_capacity) {
        return 1;
    }

    void *new_data = PF_REALLOC(vec->data, new_capacity * vec->elemSize);
    if (!new_data) return -1;

    vec->data = new_data;
    vec->capacity = new_capacity;

    return 0;
}

static inline PFint
pfiShrinkVectorToFit(PFIvector* vec)
{
    if (vec->size == vec->capacity) {
        return 1;
    }

    if (vec->size == 0) {
        pfiDeleteVector(vec);
        return 0;
    }

    void *new_data = PF_REALLOC(vec->data, vec->size * vec->elemSize);
    if (!new_data) return -1;

    vec->data = new_data;
    vec->capacity = vec->size;

    return 0;
}

static inline void
pfiClearVector(PFIvector* vec)
{
    vec->size = 0;
}

static inline void
pfiFillVector(PFIvector* vec, const void* data)
{
    const void *end = vec->data + vec->capacity * vec->elemSize;
    for (PFbyte *ptr = vec->data; (void*)ptr < end; ptr += vec->elemSize) {
        memcpy(ptr, data, vec->elemSize);
    }
    vec->size = vec->capacity;
}

static inline PFint
pfiInsertToVector(PFIvector* vec, PFsizei index, const void* elements, PFsizei count)
{
    if (index > vec->size) {
        return -1;
    }

    PFsizei new_size = vec->size + count;

    if (new_size > vec->capacity) {
        if (pfiResizeVector(vec, NEXT_POT(new_size)) != 0) {
            return -2;
        }
    }

    // Moving items to make room for new items
    void *destination = (PFbyte*)vec->data + (index + count) * vec->elemSize;
    void *source = (PFbyte*)vec->data + index * vec->elemSize;
    PFsizei bytes_to_move = (vec->size - index) * vec->elemSize;
    memmove(destination, source, bytes_to_move);

    // Inserting new elements
    void *target = (PFbyte*)vec->data + index * vec->elemSize;
    memcpy(target, elements, count * vec->elemSize);

    // Updating vector size
    vec->size = new_size;

    return 0;
}

static inline void*
pfiBeginVector(PFIvector* vec)
{
    return vec->data;
}

static inline const void*
pfiEndVector(const PFIvector* vec)
{
    return (const PFbyte*)vec->data + vec->size * vec->elemSize;
}

static inline PFint
pfiPushBackVector(PFIvector* vec, const void *element)
{
    if (vec->size >= vec->capacity) {
        if (pfiResizeVector(vec, NEXT_POT(vec->capacity)) != 0) {
            return -1;
        }
    }

    void *target = (PFbyte*)vec->data + vec->size * vec->elemSize;
    memcpy(target, element, vec->elemSize);
    vec->size++;

    return 0;
}

static inline PFint
pfiPushFrontVector(PFIvector* vec, const void *element)
{
    if (vec->size >= vec->capacity) {
        if (pfiResizeVector(vec, NEXT_POT(vec->capacity)) != 0) {
            return -1;
        }
    }

    // Move all existing items to the right to make room
    void *destination = (PFbyte*)vec->data + vec->elemSize;
    void *source = vec->data;
    PFsizei bytes_to_move = vec->size * vec->elemSize;
    memmove(destination, source, bytes_to_move);

    // Copy new item to start
    memcpy(vec->data, element, vec->elemSize);

    // Increment size
    vec->size++;

    return 0;
}

static inline PFint
pfiPushAtVector(PFIvector* vec, PFsizei index, const void* element)
{
    if (index >= vec->size) {
        return -1;
    }

    if (vec->size >= vec->capacity) {
        if (pfiResizeVector(vec, NEXT_POT(vec->capacity)) != 0) {
            return -2;
        }
    }

    // Move existing items from index to make room
    void *destination = (PFbyte*)vec->data + index * vec->elemSize;
    void *source = (PFbyte*)vec->data + index * vec->elemSize;
    PFsizei bytes_to_move = (vec->size - index) * vec->elemSize;
    memmove(destination, source, bytes_to_move);

    // Copy the new element to the specified position
    if (element != NULL) {
        memcpy(source, element, vec->elemSize);
    }

    // Increment size
    vec->size++;

    return 0;
}

static inline PFint
pfiPopBackVector(PFIvector* vec, void* element)
{
    if (vec->size == 0) {
        return -1;
    }

    vec->size--;
    if (element != NULL) {
        void *source = (PFbyte*)vec->data + vec->size * vec->elemSize;
        memcpy(element, source, vec->elemSize);
    }

    return 0;
}

static inline PFint
pfiPopFrontVector(PFIvector* vec, void* element)
{
    if (vec->size == 0) {
        return -1;
    }

    if (element != NULL) {
        memcpy(element, vec->data, vec->elemSize);
    }

    // Move all remaining items to the left
    void *source = (PFbyte*)vec->data + vec->elemSize;
    void *destination = vec->data;
    PFsizei bytes_to_move = (vec->size - 1) * vec->elemSize;
    memmove(destination, source, bytes_to_move);

    // Reduce vector size
    vec->size--;

    return 0;
}

static inline PFint
pfiPopAtVector(PFIvector* vec, PFsizei index, void* element)
{
    if (index >= vec->size) {
        return -1;
    }

    if (element != NULL) {
        void *source = (PFbyte*)vec->data + index * vec->elemSize;
        memcpy(element, source, vec->elemSize);
    }

    // Move the remaining items to the left to fill the hole
    void *destination = (PFbyte*)vec->data + index * vec->elemSize;
    void *source_start = (PFbyte*)vec->data + (index + 1) * vec->elemSize;
    PFsizei bytes_to_move = (vec->size - index - 1) * vec->elemSize;

    memmove(destination, source_start, bytes_to_move);

    // Reduce vector size
    vec->size--;

    return 0;
}

static inline void*
pfiAtVector(PFIvector* vec, PFsizei index)
{
    if (index >= vec->size) return NULL;
    return (PFbyte*)vec->data + index * vec->elemSize;
}

#undef NEXT_POT

#endif //VEC_H
