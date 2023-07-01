#include "iterator.h"

#ifndef ITERATOR_TYPE
    #define ITERATOR_TYPE int
#endif

#ifndef ITERATOR_PREFIX
    #define ITERATOR_PREFIX ITERATOR_TYPE
#endif

//solution using https://stackoverflow.com/questions/1082192/how-to-generate-random-variable-names-in-c-using-macros
#define CONCAT_INNER(a, b) a ## _ ## b
#define PREFIX_NAME(a, b) CONCAT_INNER(a, b)


//available public accessable methods in this list

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
/*    static void PREFIX_NAME(LINKED_LIST_PREFIX, linked_list_iterator_init) (linked_list *list, iterator *iter){
        linked_list_iterator_init(list, iter);
    }*/
    
    static ITERATOR_TYPE * PREFIX_NAME(ITERATOR_PREFIX, ITERATOR_NEXT)(struct iterator_struc *iter){
        return (ITERATOR_TYPE *) iter->next(iter);
    }

//just refer to the original ones for this:
//    int linked_list_size(linked_list *list);
//    int linked_list_is_empty(linked_list *list);
//    void linked_list_free(linked_list *list);

#pragma GCC diagnostic pop

#undef PREFIX_NAME
#undef CONCAT_INNER