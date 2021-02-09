
#ifndef __PLAYER_RAW_H__
#define __PLAYER_RAW_H__

#include <Audio.h>               // https://github.com/PaulStoffregen/Audio
#include "config.h"
#include "llist.h"
#include "blob.h"
#include "mapping.h"

typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

void SETUP_FLASH_PLAYER();
void play_raw(llist_t* blobs_ptr, AudioPlaySerialflashRaw* player_ptr);

#endif /*__PLAYER_RAW__*/
