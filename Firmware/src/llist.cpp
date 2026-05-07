/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "llist.h"

#define LLIST_NODES_POOL  3000 // Set the maximum nodes number

lnode_t llist_nodes_array[LLIST_NODES_POOL] = {0}; // Store linked list nodes

static llist_t llist_nodes_pool;

void llist_raz(llist_t* llist_ptr) {
  llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
};

static lnode_t* llist_pop_node_front(llist_t*);
static void llist_push_node_front(llist_t*, lnode_t*);

void llist_setup(void) {
  llist_raz(&llist_nodes_pool);
  for (uint16_t i = 0; i < LLIST_NODES_POOL; i++) {
    llist_push_node_front(&llist_nodes_pool, &llist_nodes_array[i]);
  }
};

static lnode_t* llist_alloc_node(void) {
  lnode_t* node_ptr = llist_pop_node_front(&llist_nodes_pool);
  if (node_ptr != NULL) {
    node_ptr->data_ptr = NULL;
    return node_ptr;
  }
  else {
    return NULL;
  }
};

static void llist_free_node(lnode_t* node_ptr) {
  node_ptr->data_ptr = NULL;
  llist_push_node_front(&llist_nodes_pool, node_ptr);
};

void llist_builder(llist_t* llist_ptr, void* nodes_array_ptr, const int item_count, const int item_size) {
  uint8_t* item_ptr = (uint8_t*)nodes_array_ptr;
  llist_raz(llist_ptr);
  for (uint16_t i = 0; i < item_count; i++) {
    llist_push_front(llist_ptr, item_ptr);
    item_ptr += item_size;
  }
};

static lnode_t* llist_pop_node_front(llist_t* llist_ptr) {
  if (llist_ptr->head_ptr) {
    lnode_t* node_ptr = llist_ptr->head_ptr;
    if (llist_ptr->head_ptr != llist_ptr->tail_ptr) {
      llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
    }
    else {
      llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
    }
    return node_ptr;
  }
  else {
    return NULL;
  }
};

static void llist_push_node_front(llist_t* llist_ptr, lnode_t* node_ptr) {
  if (llist_ptr->head_ptr) {
    node_ptr->next_ptr = llist_ptr->head_ptr;
    llist_ptr->head_ptr = node_ptr;
  }
  else {
    node_ptr->next_ptr = NULL;
    llist_ptr->head_ptr = llist_ptr->tail_ptr = node_ptr;
  }
};

static void llist_push_node_back(llist_t* llist_ptr, lnode_t* node_ptr) {
  node_ptr->next_ptr = NULL;
  if (llist_ptr->tail_ptr) {
    llist_ptr->tail_ptr->next_ptr = node_ptr;
    llist_ptr->tail_ptr = node_ptr;
  }
  else {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = node_ptr;
  }
};

void* llist_pop_front(llist_t* llist_ptr) {
  lnode_t* node_ptr = llist_pop_node_front(llist_ptr);
  if (node_ptr != NULL) {
    void* data_ptr = node_ptr->data_ptr;
    llist_free_node(node_ptr);
    return data_ptr;
  }
  else {
    return NULL;
  }
};

void llist_push_front(llist_t* llist_ptr, void* data_ptr) {
  lnode_t* node_ptr = llist_alloc_node();
  if (node_ptr != NULL) {
    node_ptr->data_ptr = data_ptr;
    llist_push_node_front(llist_ptr, node_ptr);
  }
  else {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
      Serial.println("no more nodes left in the: llist_nodes_pool -> see llist_push_front()"); 
    #endif
    //set_mode(ERROR_MODE);
  }
};

void llist_push_back(llist_t* llist_ptr, void* data_ptr) {
  lnode_t* node_ptr = llist_alloc_node();
  if (node_ptr != NULL) {
    node_ptr->data_ptr = data_ptr;
    llist_push_node_back(llist_ptr, node_ptr);
  }
  else {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
      Serial.println("no more nodes left in the: llist_nodes_pool -> see llist_push_back()"); 
    #endif
    //set_mode(ERROR_MODE);
  }
};

void llist_swap_llist(llist_t* llistA_ptr, llist_t* llistB_ptr) {
  if (llistA_ptr->head_ptr || llistB_ptr->head_ptr) {
    lnode_t* tmp_head_ptr = llistA_ptr->head_ptr;
    lnode_t* tmp_tail_ptr = llistA_ptr->tail_ptr;
    llistA_ptr->head_ptr = llistB_ptr->head_ptr;
    llistA_ptr->tail_ptr = llistB_ptr->tail_ptr;
    llistB_ptr->head_ptr = tmp_head_ptr;
    llistB_ptr->tail_ptr = tmp_tail_ptr;
  }
};

/*
void llist_concat_nodes(llist_t* dst_ptr, llist_t* src_ptr) {
  if (src_ptr->head_ptr) {
    if (dst_ptr->head_ptr) {
      dst_ptr->tail_ptr->next_ptr = src_ptr->head_ptr;
      dst_ptr->tail_ptr = src_ptr->tail_ptr;
      src_ptr->tail_ptr = src_ptr->head_ptr = NULL;
    }
    else {
      dst_ptr->head_ptr = src_ptr->head_ptr;
      dst_ptr->tail_ptr = src_ptr->tail_ptr;
      src_ptr->tail_ptr = src_ptr->head_ptr = NULL;
    }
  }
};
*/

void* llist_find_node(llist_t* llist_ptr, void* data_ptr, llist_compare_func_t* func_ptr) {
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(llist_ptr); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    if ((*func_ptr)(node_ptr->data_ptr, data_ptr)) {
      return node_ptr->data_ptr;
    }
  }
  return NULL;
};
