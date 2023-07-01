#include "common.h"
#define RETURN_ERROR -1
#define RETURN_SUCCESS 0

char* read_file_to_string(char* filename);

struct updater_data *read_config_from_string(char const* string);
