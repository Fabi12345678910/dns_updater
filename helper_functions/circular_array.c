#include "circular_array.h"

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

//#define DEBUG_LEVEL 2
#include "debug.h"

circular_array* circular_array_init(circular_array* array, size_t element_size,
                            size_t capacity, void (* free_func)(void *)){
    DEBUG_PRINT_1("initializing circular array\n");

    //allocate memory
    array->data = malloc(element_size * capacity);
    if(array->data == NULL){
        return NULL;
    }

    array->capacity = capacity;
    array->element_size = element_size;
    array->free_func = free_func;

    array->begin = 0;
    array->end = 0;
    array->size = 0;
    return array;
}

void* circular_array_get(circular_array* array, size_t index){
    DEBUG_PRINT_1("getting element %zu\n", index);

    if(index >= array->capacity){
        DEBUG_PRINT_2("index too high, returning NULL\n");
        return NULL;
    }

    char* data = (char*) array->data;
    size_t actual_index = (array->begin + index) % array->capacity;
    DEBUG_PRINT_2("this is actually element nr %zu\n", actual_index);
    return (void *) (data + (actual_index * array->element_size));
}

ssize_t circular_array_append(circular_array* array, void* element_ptr){
    DEBUG_PRINT_1("inserting new element\n")
    void* element_location = circular_array_get(array, array->size % array->capacity);
    if(array->size == array->capacity){
        DEBUG_PRINT_2("overflow, adjusting begin and size\n")
        //delete first element
        if(array->free_func != NULL){
            DEBUG_PRINT_2("implicitly free the overwritten element\n");
            array->free_func(element_location);
        }
        array->begin = (array->begin + 1) % array->capacity;
        array->size--;
    }

    memcpy(element_location, element_ptr, array->element_size);
    array->end = (array->end + 1) % array->capacity;
    array->size++;

    return array->size-1;
}

size_t circular_array_size(circular_array* array){
    return array->size;
}

size_t circular_array_is_empty(circular_array* array){
    return array->size == 0;
}

struct circular_array_iterator_data{
    circular_array* array;
    size_t index;
};

int circular_array_iterator_has_next(iterator* iter){
    struct circular_array_iterator_data * iter_data = (struct circular_array_iterator_data *) iter->iterator_data;
    return iter_data->index < iter_data->array->size;
}

void * circular_array_iterator_next(iterator* iter){
    struct circular_array_iterator_data * iter_data = (struct circular_array_iterator_data *) iter->iterator_data;
    if (!circular_array_iterator_has_next(iter)){
        return NULL;
    }

    void* ret_value = circular_array_get(iter_data->array, iter_data->index);
    iter_data->index++;
    return ret_value;
}

int circular_array_iterator_init(circular_array* array, iterator* iter){    
    iter->iterator_data = malloc(sizeof(struct circular_array_iterator_data));
    if(iter->iterator_data == NULL){
        return -1;
    }
    struct circular_array_iterator_data * iter_data = (struct circular_array_iterator_data *) iter->iterator_data;
    
    iter_data->array = array;
    iter_data->index = 0;

    iter->remove = NULL;
    iter->has_next = circular_array_iterator_has_next;
    iter->next = circular_array_iterator_next;

    return 0;
}

void circular_array_iterator_free(iterator* iter){
    free(iter->iterator_data);
}

void circular_array_free(circular_array* array){
    if(array->free_func != NULL){
        for(size_t i = 0; i < array->size; i++){
            array->free_func(circular_array_get(array, i));
        }
    }
    free(array->data);
}