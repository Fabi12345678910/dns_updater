#include "linked_list.h"

#ifndef LINKED_LIST_TYPE
    #define LINKED_LIST_TYPE int
#endif

#ifndef LINKED_LIST_PREFIX
    #define LINKED_LIST_PREFIX LINKED_LIST_TYPE
#endif

#ifndef ITERATOR_TYPE
    #define ITERATOR_TYPE LINKED_LIST_TYPE
#endif

#ifndef ITERATOR_PREFIX
    #define ITERATOR_PREFIX LINKED_LIST_PREFIX
#endif
#include "iterator_comfort.h"

//solution using https://stackoverflow.com/questions/1082192/how-to-generate-random-variable-names-in-c-using-macros
#define CONCAT_INNER(a, b) a ## _ ## b
#define PREFIX_NAME(a, b) CONCAT_INNER(a, b)


//available public accessable methods in this list

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
    
    static int PREFIX_NAME(LINKED_LIST_PREFIX, linked_list_insert)(linked_list *list, LINKED_LIST_TYPE value){
        return linked_list_insert(list, &value, sizeof(LINKED_LIST_TYPE));
    }

    static LINKED_LIST_TYPE* PREFIX_NAME(LINKED_LIST_PREFIX, linked_list_get) (linked_list *list, position_type position){
        return (LINKED_LIST_TYPE*) linked_list_get(list, position);
    }

//just refer to the original ones for this:
//    void linked_list_iterator_init(list, iter);
//    int linked_list_size(linked_list *list);
//    int linked_list_is_empty(linked_list *list);
//    void linked_list_free(linked_list *list);

#pragma GCC diagnostic pop

#undef PREFIX_NAME
#undef CONCAT_INNER