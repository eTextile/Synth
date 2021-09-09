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
#include "presets.h"

typedef struct lnode lnode_t;          // Forward declaration
typedef struct llist llist_t;          // Forward declaration
typedef struct preset preset_t;        // Forward declaration

typedef struct image image_t;
struct image {
  uint8_t* pData;
  uint8_t numCols;
  uint8_t numRows;
};

extern preset_t presets[];   // Located in ...
extern image_t interpFrame;  // Located in ...
extern llist_t blobs;        // Located in blob.cpp

#define COMPUTE_IMAGE_ROW_PTR(pImage, y) \
  ({ \
    __typeof__ (pImage) _pImage = (pImage); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_pImage->pData) + (_pImage->numCols * _y); \
  })

#define IMAGE_GET_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
  })

#define IMAGE_SET_PIXEL_FAST(row_ptr, x, v) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    _row_ptr[_x] = _v; \
  })

#define PIXEL_THRESHOLD(pixel, Threshold) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (Threshold) _Threshold = (Threshold); \
    _pixel > _Threshold; \
  })

#define MIN(a, b)({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define MAX(a, b)({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

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
  float   X;
  float   Y;
  uint8_t Z;
};

typedef struct box box_t;
struct box {
  uint8_t W; // TODO Make it as float
  uint8_t H; // TODO Make it as float
};

typedef struct velocity velocity_t;
struct velocity {
  unsigned long timeTag;
  float XY;
  float Z;
};

typedef enum status {
  FREE,
  NOT_FOUND,
  TO_REMOVE
} status_t;

typedef struct blob blob_t;
struct blob {
  lnode_t node;
  uint8_t UID;
  status_t status;
  unsigned long timeTag;
  uint16_t pixels;
  boolean state;
  boolean lastState;
  box_t box;
  point_t centroid;
  velocity_t velocity;
};

typedef enum params {
  BI,  // [0] Blob UID
  BS,  // [1] Blob State
  BL,  // [2] Blob Last State
  BX,  // [3] Blob X centroid position
  BY,  // [4] Blob Y centroid position
  BW,  // [5] Blob width
  BH,  // [6] Blob Height
  BD   // [7] Blob Depth
} params_t;

void lifo_llist_init(llist_t *list, xylr_t* nodesArray, const int nodes); // TODO: Separation of concerns (SoC)
void blob_llist_init(llist_t *list, blob_t* nodesArray, const int nodes); // TODO: Separation of concerns (SoC)

void BLOB_SETUP(void);
void find_blobs(void);

#endif /*__BLOB_H__*/
