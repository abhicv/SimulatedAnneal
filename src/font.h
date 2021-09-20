#ifndef FONT_H
#define FONT_H

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "types.h"
#include "render.h"

#define RENDERABLE_CHAR(c) ((c >= 32) && (c <= 126)) 

//8-bit single channel bitmap
typedef struct BitMap
{
    u8 *pixels;
    u32 width; 
    u32 height;
    
} BitMap;

typedef struct FontData
{
    stbtt_fontinfo fontInfo;
    stbtt_bakedchar charDatas[256];
    
    //raw font data read from ttf file
    u8 *rawFontData;
    
    BitMap atlasBitMap;
    
    f32 fontSize;
    u32 lineHeight;
    
    b32 loaded;
    
} FontData;

#endif //FONT_H
