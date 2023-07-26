#include "linked_list.h"
#include <string.h>

#define RETURN_ERROR -1
#define RETURN_SUCCESS 0

#define DEBUG_LEVEL 0
#include "debug.h"
#define DEBUG_LINKED_LIST 1
#define DEBUG_PRINT_1_LINKED_LIST(...) DEBUG_PRINT_1_CONDITIONAL(DEBUG_LINKED_LIST, __VA_ARGS__)

void linked_list_init(linked_list *list){
    list->size = 0;
    list->first = NULL;
    list->last = NULL;
}

int linked_list_insert(linked_list *list, void* value, size_t size){
    struct linked_list_node * new_node = (struct linked_list_node *) malloc(sizeof(struct linked_list_node));
    if (new_node == NULL){
        return RETURN_ERROR;
    }
    new_node->value = malloc(size);
    if (new_node->value == NULL){
        return RETURN_ERROR;
    }

    DEBUG_PRINT_1_LINKED_LIST("previous last node: %p\n", (void*) list->last);
    if(linked_list_is_empty(list)){
        DEBUG_PRINT_1_LINKED_LIST("inserting in empty list\n");
        list->first = new_node;
        list->last = list->first;
    }else{
        DEBUG_PRINT_1_LINKED_LIST("inserting in non-empty list\n");
        list->last->next = new_node;
        list->last = list->last->next;
    }
    DEBUG_PRINT_1_LINKED_LIST("new last node: %p\n", (void*) list->last);

    memcpy(new_node->value, value, size);
    new_node->next = NULL;

    int index = list->size;

    list->size++;
    return index;
}

void* linked_list_get(linked_list *list, position_type position){
    iterator list_iterator;
    linked_list_iterator_init(list, &list_iterator);
    for(position_type i = 0; i < position; i++){
        if(!ITERATOR_HAS_NEXT(&list_iterator)){
            return NULL;
        }
        ITERATOR_NEXT(&list_iterator);
    }
    return ITERATOR_NEXT(&list_iterator);
}

int linked_list_size(linked_list *list){
    return list->size;
}

int linked_list_is_empty(linked_list *list){
    return linked_list_size(list) == 0;
}


//  to be implemented if required
//    int linked_list_remove(linked_list *list, LINKED_LIST_TYPE value){};
//    int linked_list_remove_at(linked_list *list, position_type position){};
//    int linked_list_replace(linked_list *list, LINKED_LIST_TYPE value){};

void linked_list_free_with_function(linked_list *list, void (* free_func)(void *)){
    struct linked_list_node *node,
                            *next_node;
    node = list->first;
    while(node != NULL){
        next_node = node->next;
        if(free_func != NULL){
            free_func(node->value);
        }

        free(node->value);
        free(node);
        node = next_node;
    }
}

void linked_list_free(linked_list *list){
    linked_list_free_with_function(list, NULL);
}

/**
 * Iterator implementation regarding iterator.h
*/

#define ITERATOR_NODE ((struct linked_list_node *) iter->iterator_data)

//  not required as of now
//    void linked_list_iterator_free(iterator *iter);

int linked_list_iterator_implementation_has_next(iterator* iter){
    return ITERATOR_NODE->next != NULL;
}

void* linked_list_iterator_implementation_next(iterator* iter){
    iter->iterator_data = ITERATOR_NODE->next;
    DEBUG_PRINT_1_LINKED_LIST("accessing next node at %p\n", (void*) ITERATOR_NODE)
    if (ITERATOR_NODE == NULL){
        return NULL;
    }
    return ITERATOR_NODE->value;
}

int iterator_unimplemented_remove (iterator* iter){
    (void) iter;
    return -1;
}

void linked_list_iterator_init(linked_list *list, iterator *iter){
    iter->has_next = linked_list_iterator_implementation_has_next;
    iter->next = linked_list_iterator_implementation_next;
    iter->remove = iterator_unimplemented_remove;
    iter->iterator_data = list;
}
#undef ITERATOR_NODE

#undef RETURN_ERROR
#undef RETURN_SUCCESS