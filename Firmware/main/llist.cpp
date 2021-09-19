/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "llist.h"

void llist_raz(llist_t* llist_ptr) {
  llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
};

void* llist_pop_front(llist_t* llist_ptr) {
  lnode_t* node = llist_ptr->head_ptr;
  if (node != NULL) {
    if (llist_ptr->head_ptr != llist_ptr->tail_ptr) {
      llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
      return node;
    }
    else {
      llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
      return node;
    };
  }
  else {
    return NULL;
  };
};

void llist_push_front(llist_t* llist_ptr, void* data_ptr) {
  lnode_t* node = (lnode_t*)data_ptr;
  if (llist_ptr->head_ptr != NULL) {
    node->next_ptr = llist_ptr->head_ptr;
    llist_ptr->head_ptr = node;
  }
  else {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = node;
    node->next_ptr = NULL;
  };
};

// linked-list node extractor
void llist_extract_node(llist_t* llist_ptr, void* prevData_ptr, void* data_ptr) {
  lnode_t* nodeToExtract = (lnode_t*)data_ptr;
  lnode_t* prevNode_ptr = (lnode_t*)prevData_ptr;

  if (llist_ptr->head_ptr == llist_ptr->tail_ptr) {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
  }
  else {
    if (nodeToExtract == llist_ptr->head_ptr) {
      llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
    }
    else if (nodeToExtract == llist_ptr->tail_ptr) {
      llist_ptr->tail_ptr = prevNode_ptr;
    }
    else {
      prevNode_ptr->next_ptr = nodeToExtract->next_ptr;
    };
  };
};

void llist_swap_llist(llist_t* llistA_ptr, llist_t* llistB_ptr) {
  lnode_t* tmp_head_ptr = llistA_ptr->head_ptr;
  lnode_t* tmp_tail_ptr = llistA_ptr->tail_ptr;
  llistA_ptr->head_ptr = llistB_ptr->head_ptr;
  llistA_ptr->tail_ptr = llistB_ptr->tail_ptr;
  llistB_ptr->head_ptr = tmp_head_ptr;
  llistB_ptr->tail_ptr = tmp_tail_ptr;
};

void llist_save_nodes(llist_t* dst_ptr, llist_t* src_ptr) {
  if (src_ptr->head_ptr != NULL) {
    dst_ptr->tail_ptr->next_ptr = src_ptr->head_ptr;
    dst_ptr->tail_ptr = src_ptr->tail_ptr;
    src_ptr->tail_ptr = src_ptr->head_ptr = NULL;
  };
};
