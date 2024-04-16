
#ifndef BMP_IMAGE_H
#define BMP_IMAGE_H
#include <stdint.h>
#include <stdio.h>
#include "ctype.h"
#include<memory.h>
#include "../Includes/colorTypes.h"
#define BMP_HEADER_SIZE 14
#define BMP_INFO_HEADER_SIZE 40
#define BMP_HEADER_ALIGN_DUMMY 2
typedef struct BMPHeader_t{
    uint16_t dummy;
    uint8_t signature[2];
    uint32_t filesize;
    uint32_t reserved;
    uint32_t fileoffset_to_pixelarray;
} BMPHeader;
typedef struct BMPInfoHeader_t{
    uint32_t dibheadersize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsperpixel;
    uint32_t compression;
    uint32_t imagesize;
    uint32_t ypixelpermeter;
    uint32_t xpixelpermeter;
    uint32_t numcolorspallette;
    uint32_t mostimpcolor;
} BMPInfoHeader;
typedef struct BMPImage_t
{
    BMPHeader header;
    BMPInfoHeader infoHeader;
    uint8_t* pixelBuffer;
} BMPImage;
BMPImage* BMPImage_Ctor(int width, int height, int depth);
void BMPImage_Dctor(BMPImage** inst);
BMPImage* BMPImage_FromFile(const char* filepath);
void BMPImage_Save(const BMPImage* inst, const char* filepath);
void BMPImage_SetPixel(BMPImage* inst, int x, int y, ColorRGB color);
ColorRGB BMPImage_GetPixel(BMPImage* inst, uint32_t x, uint32_t y);
uint32_t BMPImage_GetWidth(BMPImage* inst);
uint32_t BMPImage_GetHeight(BMPImage* inst);
static uint32_t _BMPImage_GetFileFullSize(FILE* fp);
#endif