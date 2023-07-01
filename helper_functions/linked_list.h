#ifndef LINKED_LIST_H
    #define LINKED_LIST_H

    #include <stdlib.h>

    #include "iterator.h"

    #define linked_list struct linked_list_head
    #define position_type size_t

        //linked list head, can be used as a sentinal node to reduce edge cases
    //obviously the value then is unusable
    struct linked_list_head{
        struct linked_list_node *first;
        struct linked_list_node *last;
        int size;
    };

    struct linked_list_node{
        struct linked_list_node *next;
        void* value;
    };


    //available public accessable methods in this list

    //initialize the iterator iter with a iterator over the list list
    void linked_list_iterator_init(linked_list *list, iterator *iter);
    
    void linked_list_init(linked_list *list);
    int linked_list_insert(linked_list *list, void* value, size_t size);
    void* linked_list_get(linked_list *list, position_type position);
    int linked_list_size(linked_list *list);
    int linked_list_is_empty(linked_list *list);
    void linked_list_free(linked_list *list);
#endif