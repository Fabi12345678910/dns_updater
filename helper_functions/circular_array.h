#ifndef CIRCULAR_ARRAY_H
    #define CIRCULAR_ARRAY_H

    #include <sys/types.h>

    #include "iterator.h"

    struct circular_array_struc{
        size_t begin;
        size_t end;
        void* data;
        size_t element_size;
        size_t capacity;
        void (* free_func)(void *);
        size_t size;
    };

    typedef struct circular_array_struc circular_array;

    int circular_array_iterator_init(circular_array* array, iterator* iter);
    void circular_array_iterator_free(iterator* iter);

    circular_array* circular_array_init(circular_array* array, size_t element_size, size_t capacity, void (* free_func)(void *));
    ssize_t circular_array_append(circular_array* array, void* element_ptr);
    void* circular_array_get(circular_array* array, size_t index);
    size_t circular_array_size(circular_array* array);
    size_t circular_array_is_empty(circular_array* array);

    void circular_array_free(circular_array* array);
#endif