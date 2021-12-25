#include "allocate.h"

char* config_ptr = NULL;

char* allocate(char* data_ptr, unsigned int size) {
  if (config_ptr == NULL){
    config_ptr = (char*) malloc(size);
    return config_ptr;
    } else {
    config_ptr = (char*) realloc(config_ptr, size);
    return config_ptr;
  };
};