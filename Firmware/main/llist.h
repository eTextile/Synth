/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __LLIST_H__
#define __LLIST_H__

#include "config.h"

typedef struct lnode lnode_t;

struct lnode {
  lnode_t* next_ptr;
};

typedef struct llist llist_t;

struct llist {
  lnode_t* head_ptr;
  lnode_t* tail_ptr;
  uint8_t max_nodes;
  int8_t index; // If linked list umpty: index == -1
};

////////////// Linked list - Fonction prototypes //////////////

void llist_raz(llist_t* src);

void* llist_pop_front(llist_t* src);
void llist_push_back(llist_t* dst, void* data);

void llist_save_blobs(llist_t* dst, llist_t* src);
void llist_remove_node(llist_t* src, void* data);
//void llist_sort(llist_t* ptr); // NOT_USE

////////////// Iterators //////////////

#define ITERATOR_START_FROM_HEAD(list) (((llist_t *)(list))->head_ptr)

#define ITERATOR_NEXT(node) (((lnode_t*)(node))->next_ptr)

#endif /*__LLIST_H__*/
