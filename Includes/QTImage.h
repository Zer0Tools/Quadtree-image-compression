#ifndef QTIMAGE_H
#define QTIMAGE_H

#include "QuadTreeNode.h"
#include <ctype.h>
#include "../Includes/BMPImage.h"

#ifdef QTIMAGE_EXPORTS
  #define QTIMAGE_API __declspec(dllexport)
#else
  #define QTIMAGE_API __declspec(dllimport)
#endif

#define QTIMAGECALL __cdecl

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct qtimage_t
{
    uint32_t width;
    uint32_t height;
    uint8_t depth;    
    QuadTreeNode* rootNode;
} QTImage;

typedef struct
{
    char* filename;
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

typedef struct 
{
    double maxErrorH;  
    double minS;
    double maxS;
    double minV;
    double maxV;
    double averageH;
    double averageS;    
    double averageV;
} QTImage_Encoding_Fragment_params;
static double HueErrorAbs(double average, double val);
static void QTImage_Encoding_Fragment_params_fill_func(QuadTreeNode* node);
static void QTImage_Encoding_Parent_Color_Blend_func(QuadTreeNode* node);
static uint8_t QTImage_Encoding_Merge_func(QuadTreeNode* node);
QTImage* QTImage_Ctor();
void QTImage_Dctor(QTImage** inst);
size_t QTImage_AllocSize(QTImage* inst);
QTIMAGE_API void __cdecl QTImage_Serialize(QTImage* inst, char* filepath);
QTIMAGE_API QTImage* __cdecl QTImage_Deserialize(char* filepath);
QTIMAGE_API QTImage* __cdecl QTImage_Encode(QTImage_Encode_params params);
QTIMAGE_API void __cdecl QTImage_Decode(QTImage* qtImage, char* filename);
static ColorRGB _calculateFragmentColor(BMPImage* bmp, uint32_t startX, uint32_t startY, 
                                uint32_t endX, uint32_t endY);
static void _decode(BMPImage* bmp, QuadTreeNode* treeNode, uint8_t depth, uint8_t requireDepth, 
                    unsigned int x, unsigned int y);
static QuadTreeNode* _encode(QTImage_Encode_r_params params);
#ifdef __cplusplus
}
#endif
#endif


