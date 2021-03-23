/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - http://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include "config.h"
#include "llist.h"

typedef struct lnode lnode_t; // Forward declaration
typedef struct llist llist_t; // Forward declaration

#define IM_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define IM_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  IM_LOG2_2((x) >>  2)) :  IM_LOG2_2(x)) // NO ({ ... }) !
#define IM_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  IM_LOG2_4((x) >>  4)) :  IM_LOG2_4(x)) // NO ({ ... }) !
#define IM_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  IM_LOG2_8((x) >>  8)) :  IM_LOG2_8(x)) // NO ({ ... }) !
#define IM_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define UINT8_T_BITS    (sizeof(uint8_t) * 8)
#define UINT8_T_MASK    (UINT8_T_BITS - 1)
#define UINT8_T_SHIFT   IM_LOG2(UINT8_T_MASK)

#define COMPUTE_IMAGE_ROW_PTR(pImage, y) \
  ({ \
    __typeof__ (pImage) _pImage = (pImage); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_pImage->pData) + (_pImage->numCols * _y); \
  })

#define COMPUTE_BINARY_IMAGE_ROW_PTR(pBitmap, y) \
  ({ \
    __typeof__ (pBitmap) _pBitmap = (pBitmap); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_pBitmap->pData) + (((_pBitmap->numCols + UINT8_T_MASK) >> UINT8_T_SHIFT) * _y); \
  })

#define IMAGE_GET_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
  })

#define IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x >> UINT8_T_SHIFT] |= 1 << (_x & UINT8_T_MASK); \
  })

#define IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    (_row_ptr[_x >> UINT8_T_SHIFT] >> (_x & UINT8_T_MASK)) & 1; \
  })

#define PIXEL_THRESHOLD(pixel, Threshold) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (Threshold) _Threshold = (Threshold); \
    _pixel > _Threshold; \
  })

#define MAX(a, b) \
  ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; \
  })

#define MIN(a, b) \
  ({ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; \
  })

typedef struct image image_t;

struct image {
  uint8_t numCols;
  uint8_t numRows;
  uint8_t* pData;
};

typedef struct xylr xylr_t;

struct xylr {
  lnode_t node;
  uint8_t x;
  uint8_t y;
  uint8_t l;
  uint8_t r;
  uint8_t t_l;
  uint8_t b_l;
};

typedef struct point point_t;

struct point {
  float X;
  float Y;
};

typedef struct bbox bbox_t;

struct bbox {
  uint8_t W; // TODO Make it as float
  uint8_t H; // TODO Make it as float
  uint8_t D; // TODO Make it as float
};

// Blob states
typedef enum status {
  FREE,
  TO_ADD,
  TO_UPDATE,
  TO_REMOVE
} status_t;

typedef struct blob blob_t;

struct blob {
  lnode_t node;
  uint8_t UID;
  uint32_t timeTag;
  uint16_t pixels;
  boolean state;
  boolean lastState;
  status_t status;
  point_t centroid;
  bbox_t box;
};

void bitmap_clear(image_t* bitmap_ptr);
static int sum_m_to_n(int m, int n);

void blob_llist_init(llist_t *list, blob_t* nodesArray, uint8_t max_nodes);
void lifo_llist_init(llist_t *list, xylr_t* nodesArray, uint8_t max_nodes);

void SETUP_BLOB(
  uint8_t* bitmapArray_ptr,
  image_t* bitmap_ptr,
  llist_t* lifo_ptr,
  llist_t* lifo_stack_ptr,
  xylr_t*  lifoArray_ptr,
  llist_t* blobs_ptr,
  llist_t* blobs_stack_ptr,
  blob_t*  blobArray_ptr,
  llist_t* outputBlobs_ptr
);

float distance(blob_t* blobA, blob_t* blobB);

void find_blobs(
  uint8_t   zThreshold,
  image_t*  inputFrame_ptr,
  image_t*  bitmap_ptr,
  llist_t*  lifo_stack_ptr,
  llist_t*  lifo_ptr,
  llist_t*  blobs_stack_ptr,
  llist_t*  inputBlobs_ptr,
  llist_t*  outputBlobs_ptr
);

void print_bitmap(image_t* bitmap);
void print_blobs(llist_t* src);

#endif /*__BLOB_H__*/
