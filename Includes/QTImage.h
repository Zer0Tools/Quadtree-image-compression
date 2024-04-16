#ifndef QTIMAGE_H
#define QTIMAGE_H

#include "../Src/QuadTreeNode.c"
#include <ctype.h>

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

struct QTImage_t
{
    uint32_t width;
    uint32_t height;
    uint8_t depth;    
    QuadTreeNode rootNode;
};


struct QTImage_FileReader_t
{
    void* (*OpenFile)(const char* filepath);
    void (*CloseFile)(void**);
    uint32_t (*ReadWidth)(void*);
    uint32_t (*ReadHeight)(void*);
    ColorRGB (*ReadPixel)(void*, uint32_t, uint32_t);  
};
struct QTImage_RawColorDataReader_t
{
    ColorRGB (*ReadPixel)(struct QTImage_t*, uint8_t*, uint32_t, uint32_t);  
};



struct QTImage_FragmentReadParams
{
    void* fileProvider;
    ColorRGB (*ReadPixel)(void*, uint32_t x, uint32_t y);  
};
struct QTImage_FragmentRawReadParams
{
    struct QTImage_t* inst;
    uint8_t* rawData;
    ColorRGB (*ReadPixel)(struct QTImage_t*, uint8_t*, uint32_t, uint32_t);  
};


struct QTImage_ColorDataReader_t
{
    void* params;
    ColorRGB (*ReadPixel)(void*, uint32_t, uint32_t);  
};


struct QTImage_CompressionFunctions_t
{
    void (*encoding_fragment_params_fill_func)(QuadTreeNode);
    void (*encoding_parent_color_blend_func)(QuadTreeNode);
    uint8_t (*encoding_child_megre_func) (QuadTreeNode);
};

struct QTImage_EncodeParams_t
{
    struct QTImage_ColorDataReader_t colorReader;
    struct QTImage_CompressionFunctions_t functions;
    uint8_t maxDepth;  
    uint16_t minFragScale;
};

struct QTImage_RecursiveParams
{
    uint8_t depth;
    uint32_t x;
    uint32_t y; 
};


typedef struct QTImage_FileReader_t* QTImage_FileReader; 
typedef struct QTImage_ColorDataReader_t* QTImage_ColorDataReader; 
typedef struct QTImage_RawColorDataReader_t* QTImage_RawColorDataReader;
typedef struct QTImage_EncodeParams_t* QTImage_EncodeParams; 
typedef struct QTImage_CompressionFunctions_t* QTImage_CompressionFunctions;
typedef struct QTImage_t* QTImage; 


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



QTIMAGE_API QTImage QTImage_Ctor();
QTIMAGE_API void QTImage_Dctor(QTImage* inst);

QTIMAGE_API void QTImage_CreateFromFile(    QTImage inst, 
                                                    char* filepath, 
                                                    QTImage_FileReader fileReader,
                                                    QTImage_CompressionFunctions compressionFunctions);
QTIMAGE_API void QTImage_CreateFromRaw( QTImage inst, 
                            uint8_t* rawData,
                            QTImage_RawColorDataReader colorReader, 
                            QTImage_CompressionFunctions compressionFunctions);
                            
QTIMAGE_API void QTImage_GetColorData(QTImage inst, uint8_t** colorsData);           

QTIMAGE_API void QTImage_Save(QTImage inst, char* filepath);
QTIMAGE_API void QTImage_Load(QTImage inst, char* filepath);



static QuadTreeNode _QTImage_BuildTree( QTImage inst,
                                        QTImage_EncodeParams mainParams, 
                                        struct QTImage_RecursiveParams recursiveParams);
static void _QTImage_GetColorData_R(QTImage inst, 
                                    QuadTreeNode node, 
                                    uint8_t** colorsData, 
                                    struct QTImage_RecursiveParams recursiveParams);
static double _QTImage_HueErrorAbs(double average, double val);
static void _QTImage_DefaultParamsFill(QuadTreeNode node);
static void _QTImage_DefaultColorBlend(QuadTreeNode node);
static uint8_t _QTImage_DefaultMergeFunc(QuadTreeNode node);
static ColorRGB _QTImage_FragmentBuilder(void* params, uint32_t x, uint32_t y);
static ColorRGB _QTImage_FragmentBuilderRaw(void* params, uint32_t x, uint32_t y);
static void _QTImage_CreateEncodeParams(QTImage_EncodeParams* inst, 
                                        uint8_t requireDepth, 
                                        uint16_t minFragScale, 
                                        struct QTImage_ColorDataReader_t colorsReader, 
                                        QTImage_CompressionFunctions compressionFunctions);

#ifdef __cplusplus
}
#endif
#endif


