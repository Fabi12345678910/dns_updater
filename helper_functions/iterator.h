#ifndef ITERATOR_H
    #define ITERATOR_H

    #define ITERATOR_NEXT(iter) ((iter)->next(iter))
    #define ITERATOR_HAS_NEXT(iter) ((iter)->has_next(iter))
    #define ITERATOR_REMOVE(iter) ((iter)->remove(iter))
    #define iterator struct iterator_struc
    //this header file aims to provide a general datatype for iterations
    
    struct iterator_struc{
        int (*has_next)(iterator*);
        void* (*next)(iterator*);
        int (*remove)(iterator*); //optional, may not be implemented by a iterator
        void *iterator_data; //may be used by the iterator to store positional data or whatever it needs
    };

#endif