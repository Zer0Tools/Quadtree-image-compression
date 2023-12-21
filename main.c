#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "Src/QuadTreeNode.c"
#include "Src/colorTypes.c"
#include "Src/QTImage.c"
#include "Src/BMPImage.c"
#include "Src/BitArray.c"




double HueErrorAbs(double average, double val)
{
    double error = fabs(average - val);
    if(error > 180.0)
        error = 360.0 - error;
    return error;
}

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

void QTImage_Encoding_Fragment_params_fill_func(QuadTreeNode* node)
{
    QTImage_Encoding_Fragment_params* params = calloc(1, sizeof(QTImage_Encoding_Fragment_params));
    
    params->maxErrorH = 0;   
    params->minS = 1;
    params->maxS = 0;        
    params->minV = 1;
    params->maxV = 0;
    params->averageV = 0;

    ColorHSV colorsHSV[4];

    int count = 0;
    for(int i = 0; i < 4; i++)
    {
        if(node->childrens[i] == NULL) continue;
        colorsHSV[i] = Colors_rgb2hsv(node->childrens[i]->color);
        params->averageH += colorsHSV[i].h;
        params->averageS += colorsHSV[i].s;
        params->averageV += colorsHSV[i].v;
        count++;

    }
    params->averageH /= count; params->averageS /= count; params->averageV /= count;

    for(int i = 0; i < 4; i++)
    {
        if(node->childrens[i] == NULL) continue;
        if(node->childrens[i]->dataPtr == NULL)
        {
            params->maxErrorH = 0;       
            params->minS = fmin(params->minS, colorsHSV[i].s);
            params->maxS = fmax(params->maxS, colorsHSV[i].s);             
            params->minV = fmin(params->minV, colorsHSV[i].v);
            params->maxV = fmax(params->maxV, colorsHSV[i].v);                            
        }
        else
        {
            params->maxErrorH = fmax(params->maxErrorH, HueErrorAbs(params->averageH, colorsHSV[i].h));
            params->minV = fmin(params->minV, ((QTImage_Encoding_Fragment_params*)(node->childrens[i]->dataPtr))->minV);
            params->maxV = fmax(params->maxV, ((QTImage_Encoding_Fragment_params*)(node->childrens[i]->dataPtr))->maxV);
            params->minS = fmin(params->minS, ((QTImage_Encoding_Fragment_params*)(node->childrens[i]->dataPtr))->minS);
            params->maxS = fmax(params->maxS, ((QTImage_Encoding_Fragment_params*)(node->childrens[i]->dataPtr))->maxS);

        }
        
    }
    
    node->dataPtr = params;
    node->dataSize = sizeof(QTImage_Encoding_Fragment_params);
}

void QTImage_Encoding_Parent_Color_Blend_func(QuadTreeNode* node)
{
    QTImage_Encoding_Fragment_params* params = (QTImage_Encoding_Fragment_params*)node->dataPtr;   
    uint16_t averageR = 0, averageG = 0, averageB = 0;
    int count = 0;
    for(int i = 0; i < 4; i++)
    {
        if(node->childrens[i] == NULL) continue;    
        averageR += node->childrens[i]->color.rgba[0];
        averageG += node->childrens[i]->color.rgba[1];
        averageB += node->childrens[i]->color.rgba[2];
        count++;
    }
    if(count == 0) return;
    node->color.rgba[0] = averageR / count;
    node->color.rgba[1] = averageG / count;
    node->color.rgba[2] = averageB / count;
}

uint8_t QTImage_Encoding_Merge_func(QuadTreeNode* node)
{
    QTImage_Encoding_Fragment_params* params = (QTImage_Encoding_Fragment_params*)node->dataPtr;  
    
    double maxErrorS = 0, maxErrorV = 0, maxErrorH = 0;
    for(int i = 0; i < 4; i++)
    {
        
        if(node->childrens[i] == NULL) continue;
        ColorHSV hsv = Colors_rgb2hsv(node->childrens[i]->color);
        maxErrorS = fmax(maxErrorS, fabs(params->averageS - hsv.s));
        maxErrorV = fmax(maxErrorV, fabs(params->averageV - hsv.v));
        maxErrorH = fmax(maxErrorH, HueErrorAbs(params->averageH, hsv.h));
    }

    double dV = params->maxV - params->minV;
    double dS = params->maxS - params->minS;
    // maxErrorS < 0.035 && maxErrorV < 0.035 && maxErrorH < 50
    // if(dV < 0.05 && dS < 0.05 && params->maxErrorH < 20)
    //     return 1;
    if(dV < 0.25 && dS < 0.25 && params->maxErrorH < 25)
        return 1;        
    return 0;
}





int main(int argc, char* argv[])
{


    // BMPImage* bmpImage = BMPImage_Ctor(4, 4, 24);
    // printf("%d \n", sizeof(ColorRGB));
    // for(int i = 0; i < 4; i++)
    // {
    //     ColorRGB color;
    //     color.ecolor = 0;
    //     color.rgba[0] = 255;
    //     BMPImage_SetPixel(bmpImage, i, 0, color);
    //     color = BMPImage_GetPixel(bmpImage, i, 0);
    //     printf(" (%d %d %d) ", color.rgba[0], color.rgba[1], color.rgba[2]);
    //     //printf("%d", bmpImage->pixelBuffer[i]);
    // }
    // BMPImage_Save(bmpImage, "generated_by_code.bmp");
    // return 0;

    // BitArray* bitArray = BitArray_Ctor(10);
    // BitArray_Set(bitArray, 1, 5, BIT_ARRAY_BLOCK_SIZE_3);

    // for(int i = 0; i < 10; i++)
    // {
    //     printf("%d ", BitArray_Get(bitArray, i, BIT_ARRAY_BLOCK_SIZE_1));
    // }
    // BitArray_Dctor(&bitArray);

    // return 0;

    BMPImage* bmp = BMPImage_FromFile("temp/test.bmp");
    QTImage_Encode_params params = {bmp, 12, QTImage_Encoding_Fragment_params_fill_func,
         QTImage_Encoding_Parent_Color_Blend_func, QTImage_Encoding_Merge_func};
        
    QTImage* qtImage = QTImage_Encode(params);
    BMPImage_Dctor(&bmp);
    
    QTImage_Serialize(qtImage, "temp/test.qti");

    QTImage* qtImageDeser = QTImage_Deserialize("temp/test.qti");

    BMPImage* deserImage;
    QTImage_Decode(qtImageDeser, &deserImage);
    BMPImage_Save(deserImage, "temp/decoded_test_01.bmp");
    BMPImage_Dctor(&deserImage);

    // BMPImage* bmpc;
    // QTImage_Decode(qtImageDeser, bmpc);
    // bwrite(bmpc, "E:/Source/Repos/Zer0Tools.QuadTreeImageCompression/testing4_out.bmp");  

    // QTImage_Dctor(&qtImage);
    // QTImage_Dctor(&qtImageDeser);

    // // QuadtreeNode_SetChildren(node, 0, NULL);
    // // printf("%p \n", (uintptr_t)node->childrens);
    // // printf("----------------------------------\n");
    // // if(!QuadtreeNode_IsLeaf(node))
    // //     for(int i = 0; i < 4; i++)
    // //         if(node->childrens[i] != NULL)
    // //             printf("%p \n", (uintptr_t)node->childrens[i]);

    // //printf("%p \n", (uintptr_t)node);
    return 0;
}