#include "allocate.h"

uint8_t* allocate(uint8_t* data_ptr, uint16_t size) {
  if (data_ptr == NULL){
    data_ptr = (uint8_t*) malloc(size);
    return data_ptr;
    } else {
    data_ptr = (uint8_t*) realloc(data_ptr, size);
    return data_ptr;
  };
};