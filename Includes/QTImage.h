#ifndef QTIMAGE_H
#define QTIMAGE_H

#include "QuadTreeNode.h"
#include <ctype.h>
#include "../Includes/BMPImage.h"

typedef struct qtimage_t
{
    uint32_t width;
    uint32_t height;
    uint8_t depth;    
    QuadTreeNode* rootNode;

} QTImage;

typedef struct
{
    BMPImage* bmp;
    uint8_t maxDepth;  
    void (*encoding_fragment_params_fill_func)(QuadTreeNode*);
    void (*encoding_parent_color_blend_func)(QuadTreeNode*);
    uint8_t (*encoding_child_megre_func) (QuadTreeNode*);
} QTImage_Encode_params;

typedef struct
{
    BMPImage* bmp;
    uint8_t maxDepth;  
    uint8_t depth;
    uint32_t x;
    uint32_t y; 
    uint16_t minFragScale;
    void (*encoding_fragment_params_fill_func)(QuadTreeNode*);
    void (*encoding_parent_color_blend_func)(QuadTreeNode*);
    uint8_t (*encoding_child_megre_func) (QuadTreeNode*);
} QTImage_Encode_r_params;


QTImage* QTImage_Ctor();
void QTImage_Dctor(QTImage** inst);
void QTImage_Serialize(QTImage* inst, char* filepath);
QTImage* QTImage_Deserialize(char* filepath);

size_t QTImage_AllocSize(QTImage* inst);

QTImage* QTImage_Encode(QTImage_Encode_params params);
void QTImage_Decode(QTImage* qtImage, BMPImage** bmp);



static ColorRGB _calculateFragmentColor(BMPImage* bmp, uint32_t startX, uint32_t startY, 
                                uint32_t endX, uint32_t endY);
static void _decode(BMPImage* bmp, QuadTreeNode* treeNode, uint8_t depth, uint8_t requireDepth, 
                    unsigned int x, unsigned int y);
static QuadTreeNode* _encode(QTImage_Encode_r_params params);
#endif