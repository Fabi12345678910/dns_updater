#ifndef DEBUG_H
    #define DEBUG_H

    #include <stdio.h>

    #ifndef DEBUG_LEVEL
        #define DEBUG_LEVEL 0
    #endif

    #define DEBUG_PRINT_CONDITIONAL(condition, ...) while (condition) \
    { \
        fprintf(stdout,__VA_ARGS__); \
        break; \
    }

    #define DEBUG_PRINT_LEVEL(verbosity, ...) DEBUG_PRINT_CONDITIONAL(DEBUG_LEVEL >= verbosity, __VA_ARGS__ )

    #define DEBUG_PRINT_0(...) DEBUG_PRINT_LEVEL(0, __VA_ARGS__)
    #define DEBUG_PRINT_1(...) DEBUG_PRINT_LEVEL(1, __VA_ARGS__)
    #define DEBUG_PRINT_2(...) DEBUG_PRINT_LEVEL(2, __VA_ARGS__)
    #define DEBUG_PRINT_3(...) DEBUG_PRINT_LEVEL(3, __VA_ARGS__)


    //debug using levels and conditionals
    #define DEBUG_PRINT_LEVEL_CONDITIONAL(verbosity, condition, ...) DEBUG_PRINT_CONDITIONAL((condition) && (DEBUG_LEVEL >= verbosity), __VA_ARGS__ )

    #define DEBUG_PRINT_0_CONDITIONAL(condition, ...) DEBUG_PRINT_LEVEL_CONDITIONAL(0, condition, __VA_ARGS__)
    #define DEBUG_PRINT_1_CONDITIONAL(condition, ...) DEBUG_PRINT_LEVEL_CONDITIONAL(1, condition, __VA_ARGS__)
    #define DEBUG_PRINT_2_CONDITIONAL(condition, ...) DEBUG_PRINT_LEVEL_CONDITIONAL(2, condition, __VA_ARGS__)
    #define DEBUG_PRINT_3_CONDITIONAL(condition, ...) DEBUG_PRINT_LEVEL_CONDITIONAL(3, condition, __VA_ARGS__)

#endif