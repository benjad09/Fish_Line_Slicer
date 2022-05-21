#ifndef CUT_STRUCT_H_
#define CUT_STRUCT_H_

#include <zephyr.h>

struct cut_settings {
    uint8_t     save_slot;
    uint16_t    cuts;
    uint32_t    len_mm;
};

#endif