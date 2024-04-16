#include "../Includes/BMPImage.h"
#include <malloc.h>
#include <stdlib.h>

BMPImage* BMPImage_Ctor(int width, int height, int bitsPerPixel)
{
    BMPImage* inst = (BMPImage*)calloc(1, sizeof(BMPImage));
    int pixelBufferSize = width * height * bitsPerPixel / 8;
    uint8_t* pixelBuffer = (uint8_t*)calloc(pixelBufferSize, sizeof(uint8_t));

    inst->header.signature[0] = 'B';
    inst->header.signature[1] = 'M';
    inst->header.filesize = pixelBufferSize + BMP_HEADER_SIZE + BMP_INFO_HEADER_SIZE;
    inst->header.fileoffset_to_pixelarray = BMP_HEADER_SIZE + BMP_INFO_HEADER_SIZE;
    inst->infoHeader.dibheadersize = BMP_INFO_HEADER_SIZE;
    inst->infoHeader.width = width;
    inst->infoHeader.height = height;
    inst->infoHeader.planes = 1;
    inst->infoHeader.bitsperpixel = bitsPerPixel;
    inst->infoHeader.compression = 0;
    inst->infoHeader.imagesize = pixelBufferSize;
    inst->infoHeader.ypixelpermeter = 11811 ;
    inst->infoHeader.xpixelpermeter = 11811 ;
    inst->infoHeader.numcolorspallette = 0;

    inst->pixelBuffer = pixelBuffer;
    return inst;
}

void BMPImage_Dctor(BMPImage** inst)
{
    free((*inst)->pixelBuffer);
    free(*inst);
    *inst = NULL;
}

BMPImage* BMPImage_FromFile(const char* filepath)
{
    FILE* fp = fopen(filepath, "rb");
    if(fp == NULL)
    {
        return NULL;
    }
    BMPImage* bmpImage =(BMPImage*)calloc(1, sizeof(BMPImage));

    uint32_t bmpFullSize = _BMPImage_GetFileFullSize(fp);
    
    fread((uint8_t*)bmpImage + BMP_HEADER_ALIGN_DUMMY, 1, BMP_HEADER_SIZE + BMP_INFO_HEADER_SIZE, fp);
    fseek(fp, bmpImage->header.fileoffset_to_pixelarray, SEEK_SET);

    uint32_t bmpPixelBufferSize = bmpFullSize - bmpImage->header.fileoffset_to_pixelarray;
    uint8_t* buffer = (uint8_t*) malloc(bmpPixelBufferSize * sizeof(uint8_t));

    fread(buffer, 1, bmpPixelBufferSize, fp);
    fclose(fp);
    bmpImage->pixelBuffer = buffer;
    return bmpImage;   
}

void BMPImage_Save(const BMPImage* inst, const char* filepath)
{
    FILE* fp = fopen(filepath, "wb");    
    if(fp == NULL)
        exit(-1);
    fwrite((void*)(inst) + 2, 1, BMP_HEADER_SIZE + BMP_INFO_HEADER_SIZE, fp);

    void* zeroBuffer = calloc(inst->header.fileoffset_to_pixelarray - (BMP_HEADER_SIZE + BMP_INFO_HEADER_SIZE), sizeof(uint8_t));
    
    fwrite(zeroBuffer, 1, inst->header.fileoffset_to_pixelarray - (BMP_HEADER_SIZE + BMP_INFO_HEADER_SIZE), fp);

    fwrite((void*)(inst->pixelBuffer), 1, inst->infoHeader.imagesize, fp);

    fclose(fp);

}

void BMPImage_SetPixel(BMPImage* inst, int x, int y, ColorRGB color)
{
    uint32_t offset = x + inst->infoHeader.width * y;
    uint8_t depth = inst->infoHeader.bitsperpixel / 8;
    memcpy((void*)(inst->pixelBuffer) + offset * depth, &color, depth); 
}

ColorRGB BMPImage_GetPixel(BMPImage* inst, uint32_t x, uint32_t y)
{
    ColorRGB color;
    color.ecolor = 0;
    uint32_t offset = x + inst->infoHeader.width * y;
    uint8_t depth = inst->infoHeader.bitsperpixel / 8;
    memcpy(&color, (void*)(inst->pixelBuffer) + offset * depth, depth);
    return color;       
}
uint32_t  BMPImage_GetWidth(BMPImage* inst)
{
    return inst->infoHeader.width;
}
uint32_t BMPImage_GetHeight(BMPImage* inst)
{
    return inst->infoHeader.height;
}


static uint32_t _BMPImage_GetFileFullSize(FILE* fp)
{
    uint32_t byte_number;
    fseek(fp, 0, SEEK_END);
    byte_number = ftell(fp);
    rewind(fp);
    return byte_number;    
}