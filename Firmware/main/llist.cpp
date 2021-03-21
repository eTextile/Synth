/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "llist.h"

void llist_raz(llist_t* llist_ptr) {
  llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
}

void* llist_pop_front(llist_t* llist_ptr) {
  if (llist_ptr->head_ptr == NULL) {
    return NULL;
  }
  else {
    lnode_t* node = llist_ptr->head_ptr;

    if (llist_ptr->head_ptr != llist_ptr->tail_ptr) {
      llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
      node->next_ptr = NULL;
      return node;
    }
    else if (llist_ptr->head_ptr == llist_ptr->tail_ptr) {
      llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
      return node;
    }
  }

}

void llist_push_front(llist_t* llist_ptr, void* pData) {
  lnode_t* node = (lnode_t*)pData;
  if (llist_ptr->head_ptr != NULL) {
    node->next_ptr = llist_ptr->head_ptr;
    llist_ptr->head_ptr = node;
  }
  else {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = node;
  }
}

void llist_push_back(llist_t* llist_ptr, void* pData) {
  lnode_t* node = (lnode_t*)pData;
  //node->next_ptr = NULL;
  if (llist_ptr->tail_ptr != NULL) {
    llist_ptr->tail_ptr->next_ptr = node;
    llist_ptr->tail_ptr = node;
  }
  else {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = node;
  }
}

// linked-list node extractor
void* llist_extract_node(llist_t* llist_ptr, void* pData) {
  lnode_t* nodeToExtract = (lnode_t*)pData;
  lnode_t* prevNode_ptr = NULL;
  for (lnode_t* node = ITERATOR_START_FROM_HEAD(llist_ptr); node != NULL; node = ITERATOR_NEXT(node)) {
    if (node == nodeToExtract) {
      if (llist_ptr->head_ptr == llist_ptr->tail_ptr) {
        llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
        return node;
      }
      else if (node == llist_ptr->head_ptr) {
        llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
        node->next_ptr = NULL;
        return node;
      }
      else if (node == llist_ptr->tail_ptr) {
        llist_ptr->tail_ptr = prevNode_ptr;
        prevNode_ptr->next_ptr = NULL;
        return node;
      }
      else {
        prevNode_ptr->next_ptr = node->next_ptr;
        node->next_ptr = NULL;
        return node;
      }
    }
    prevNode_ptr = node;
  }
}

// TODO
void llist_swap_nodes(llist_t* llistA_ptr, void* pDataA, llist_t* llistB_ptr, void* pDataB) {
  lnode_t* nodeA = (lnode_t*)pDataA;
  lnode_t* nodeB = (lnode_t*)pDataB;
  lnode_t* nodeA_next_ptr = NULL;
  lnode_t* nodeA_prev_ptr = NULL;
  lnode_t* nodeB_prev_ptr = NULL;

  for (lnode_t* node = ITERATOR_START_FROM_HEAD(llistA_ptr); node != NULL; node = ITERATOR_NEXT(node)) {
    if (node == nodeA) {
      nodeA_next_ptr = nodeA->next_ptr;
      if (nodeA->next_ptr == NULL) {
        nodeA_prev_ptr->next_ptr = NULL;
        llistA_ptr->tail_ptr = nodeA_prev_ptr;
      }
      else if (nodeA == llistA_ptr->head_ptr) {
        llistA_ptr->head_ptr = llistA_ptr->head_ptr->next_ptr;
      }
      else {
        nodeA_prev_ptr->next_ptr = node->next_ptr;
      }
    }
    nodeA_prev_ptr = nodeA;
  }
}

void llist_save_nodes(llist_t* dst_ptr, llist_t* src_ptr) {
  lnode_t* node = NULL;
  while (src_ptr->head_ptr != NULL) {
    node = llist_pop_front(src_ptr);
    llist_push_back(dst_ptr, node);
    Serial.println("SAVE");
  }
}

/*
  // Linked-list sort all nodes by UID
  void llist_sort(llist_t* llist_ptr) {
  lnode_t* nodeA_prev_ptr = NULL;
  while (1) {
    boolean isSorted = true;
    for (lnode_t* nodeA = ITERATOR_START_FROM_HEAD(llist_ptr); nodeA != NULL; nodeA = ITERATOR_NEXT(nodeA)) {
      if (nodeA->UID > nodeA->next_ptr->UID) {
        isSorted = false;
        lnode_t* node_B = node_A->next_ptr;
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / prev_blob_A: %p", prev_blob_A);
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_A: %p", blob_A);
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B: %p", blob_B);
        if (nodeA_prev_ptr != NULL) { // Test if prev_ptr_A is the head of linked list
          nodeA_prev_ptr->next_ptr = node_B;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / previous blob_A is not the HEAD : %p", prev_blob_A->next_ptr);
        }
        else { // Set curr_ptr_B as NEW HEAD
          llist_ptr->head_ptr = node_B;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / SET previous blob_A as HEAD : %p", src->head_ptr);
        }
        if (node_B->next_ptr != NULL) { // Test if current blob_B is not the TAIL of linked list
          node_A->next_ptr = node_B->next_ptr;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B is not the TAIL : %p", blob_A->next_ptr);
        }
        else { // Set current blob_A as NEW TAIL
          llist_ptr->tail_ptr = node_A;
          node_A->next_ptr = NULL;
          //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / blob_B is the TAIL : %p", src->tail_ptr);
        }
        node_B->next_ptr = node_A; // Swap pointers
        //if (DEBUG_SORT) Serial.printf("\n DEBUG_SORT / llist_sort / swaped blob_A: %p", blob_B->next_ptr);
      }
      nodeA_prev_ptr = node_A;
      break;
    }
    if (isSorted) {
      return;
    }
  }
  }
*/
