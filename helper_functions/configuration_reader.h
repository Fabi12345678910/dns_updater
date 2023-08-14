#include "common.h"
#define RETURN_ERROR -1
#define RETURN_SUCCESS 0

void free_config(struct updater_data *config);

char* read_file_to_string(char const* filename);

struct updater_data *read_config_from_string(char const* string);
