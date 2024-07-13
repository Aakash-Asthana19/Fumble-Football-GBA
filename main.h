#ifndef MAIN_H
#define MAIN_H

#include "gba.h"

typedef struct {
    int x; 
    int y;
    int width;
    int height;
    const u16* image;
} Pic; //struct to store the image position

#endif
