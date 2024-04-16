#include "../Includes/QTImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "colorTypes.c"

QTImage QTIMAGECALL QTImage_Ctor()
{
    QTImage inst = calloc(1, sizeof(struct QTImage_t));
    return inst;
}

void QTIMAGECALL QTImage_Dctor(QTImage* inst)
{
    if(*inst == NULL) return;
    if((*inst)->rootNode != NULL)
        QuadTreeNode_Dctor(&((*inst)->rootNode));
    free((*inst));
    *inst = NULL;
}

void QTIMAGECALL QTImage_CreateFromFile(QTImage inst, 
                                        char* filepath, 
                                        QTImage_FileReader fileReader, 
                                        QTImage_CompressionFunctions compressionFunctions)
{

    void* fileDataProvide = fileReader->OpenFile(filepath);
    inst->width = fileReader->ReadWidth(fileDataProvide);    
    inst->height = fileReader->ReadHeight(fileDataProvide);   
    uint8_t requireDepth = (uint8_t)ceil(log2(inst->width > inst->height ?  inst->width : inst->height)); 
    if(inst->depth == 0)
        inst->depth = requireDepth;  
    else
    {
        if(inst->depth > requireDepth)
            inst->depth = requireDepth; 
    }
    uint16_t minFragScale = 1 << (requireDepth - inst->depth);

    struct QTImage_FragmentReadParams readerParams = {fileDataProvide, fileReader->ReadPixel};
    struct QTImage_ColorDataReader_t colorsReader = {&readerParams, _QTImage_FragmentBuilder};
    QTImage_EncodeParams encodeMainParams = NULL;
    _QTImage_CreateEncodeParams(&encodeMainParams, requireDepth, minFragScale, colorsReader, compressionFunctions);

    struct QTImage_RecursiveParams encodeRecursiveParams = {requireDepth, 0, 0};
    inst->rootNode = _QTImage_BuildTree(inst, encodeMainParams, encodeRecursiveParams);
    free(encodeMainParams);
    fileReader->CloseFile(&fileDataProvide);
}

void QTIMAGECALL QTImage_CreateFromRaw( QTImage inst, 
                                        uint8_t* rawData,
                                        QTImage_RawColorDataReader colorReader, 
                                        QTImage_CompressionFunctions compressionFunctions)
{
    uint8_t requireDepth = (uint8_t)ceil(log2(inst->width > inst->height ?  inst->width : inst->height)); 
    if(inst->depth == 0)
        inst->depth = requireDepth;  
    else
    {
        if(inst->depth > requireDepth)
            inst->depth = requireDepth; 
    }
    uint16_t minFragScale = 1 << (requireDepth - inst->depth);

    struct QTImage_FragmentRawReadParams readerParams = {inst, rawData, colorReader->ReadPixel};
    struct QTImage_ColorDataReader_t colorsReader = {&readerParams, _QTImage_FragmentBuilderRaw};
    QTImage_EncodeParams encodeMainParams = NULL;
    _QTImage_CreateEncodeParams(&encodeMainParams, requireDepth, minFragScale, colorsReader, compressionFunctions);
    struct QTImage_RecursiveParams encodeRecursiveParams = {requireDepth, 0, 0};
    inst->rootNode = _QTImage_BuildTree(inst, encodeMainParams, encodeRecursiveParams);
    free(encodeMainParams);
}


void QTIMAGECALL QTImage_GetColorData(QTImage inst, uint8_t** colorsData)
{
    uint8_t requireDepth = (uint8_t)ceil(log2(inst->width > inst->height ?  inst->width : inst->height)); 
    *colorsData = calloc(inst->width * inst->height, sizeof(uint8_t) * 3);
    _QTImage_GetColorData_R(inst, inst->rootNode, colorsData, 
        (struct QTImage_RecursiveParams) {requireDepth, 0, 0});              
}

void QTIMAGECALL QTImage_Save(QTImage inst, char* filepath)
{
    FILE* fp = fopen(filepath, "wb");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }    
    uint8_t* serializedData = NULL;
    size_t dataSize = QuadTreeNode_Serialize(inst->rootNode, &serializedData);
    fwrite(inst, sizeof(uint32_t), 2, fp);
    fwrite(serializedData, sizeof(uint8_t), dataSize, fp);
    fclose(fp);
}

void QTIMAGECALL QTImage_Load(QTImage inst, char* filepath)
{
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }    
    fread(inst, sizeof(uint32_t), 2, fp);

    fseek(fp, -8l, SEEK_END);
    size_t dataSize = ftell(fp); 
    fseek(fp, (sizeof(int32_t) * 2), SEEK_SET);

    uint8_t* data = calloc(dataSize, sizeof(uint8_t)); 
    fread(data, sizeof(uint8_t), dataSize, fp);

    inst->rootNode = QuadTreeNode_Deserialize(data, dataSize, &(inst->depth));
    fclose(fp);
}

QuadTreeNode _QTImage_BuildTree( QTImage inst, 
                                QTImage_EncodeParams mainParams, 
                                struct QTImage_RecursiveParams recursiveParams)
{
    if(recursiveParams.x >= inst->width || recursiveParams.y >= inst->height)
        return NULL;
    QuadTreeNode node = QuadTreeNode_Ctor();
    if(recursiveParams.depth + inst->depth == mainParams->maxDepth)
    {
        uint32_t fragEndX = (recursiveParams.x + mainParams->minFragScale) >= inst->width ?
            inst->width : recursiveParams.x + mainParams->minFragScale;
        uint32_t fragEndY = (recursiveParams.y + mainParams->minFragScale) >= inst->height ?
            inst->height : recursiveParams.y + mainParams->minFragScale;                  
      
        uint32_t r = 0, g = 0, b = 0, a = 0;
        uint64_t fragCount = (fragEndX - recursiveParams.x) * (fragEndY - recursiveParams.y); 
        ColorRGB fragColor, subFragColor;
        for(uint32_t fx = recursiveParams.x; fx < fragEndX; fx++)
        {
            for(uint32_t fy = recursiveParams.y; fy < fragEndY; fy++)
            {
                subFragColor = mainParams->colorReader.ReadPixel(mainParams->colorReader.params, fx, fy);
                r+=subFragColor.rgba[0]; g+=subFragColor.rgba[1]; b+=subFragColor.rgba[2]; a+=subFragColor.rgba[3];
            }
        }
        fragColor.rgba[0] = r / fragCount;
        fragColor.rgba[1] = g / fragCount;
        fragColor.rgba[2] = b / fragCount;
        fragColor.rgba[3] = a / fragCount;   
        node->color = fragColor;
        node->isLeaf = 1;
        return node;
    }   
    recursiveParams.depth--;
    int nodeSize = pow(2.0, recursiveParams.depth);
    node->childrens[0] = _QTImage_BuildTree(inst, mainParams, 
        (struct QTImage_RecursiveParams){recursiveParams.depth, recursiveParams.x, recursiveParams.y});
    node->childrens[1] = _QTImage_BuildTree(inst, mainParams, 
        (struct QTImage_RecursiveParams){recursiveParams.depth, recursiveParams.x + nodeSize, recursiveParams.y});
    node->childrens[2] = _QTImage_BuildTree(inst, mainParams, 
        (struct QTImage_RecursiveParams){recursiveParams.depth, recursiveParams.x + nodeSize, recursiveParams.y + nodeSize});
    node->childrens[3] = _QTImage_BuildTree(inst, mainParams, 
        (struct QTImage_RecursiveParams){recursiveParams.depth, recursiveParams.x, recursiveParams.y + nodeSize});

    if(node->childrens[0] == NULL && node->childrens[1] == NULL &&
       node->childrens[2] == NULL && node->childrens[3] == NULL)
       {
            QuadTreeNode_Dctor(&node);
            return NULL;
       }
    mainParams->functions.encoding_fragment_params_fill_func(node);
    mainParams->functions.encoding_parent_color_blend_func(node);
    if(mainParams->functions.encoding_child_megre_func(node))
    {
        for( int i = 0; i < 4; i++)
            if(node->childrens[i] != NULL)
                QuadTreeNode_Dctor(&(node->childrens[i]));  
        node->isLeaf = 1;
    }    
    return node;
}

void _QTImage_CreateEncodeParams(QTImage_EncodeParams* inst, 
                                        uint8_t requireDepth, 
                                        uint16_t minFragScale, 
                                        struct QTImage_ColorDataReader_t colorsReader, 
                                        QTImage_CompressionFunctions compressionFunctions)
{
    (*inst) = calloc(1, sizeof(struct QTImage_EncodeParams_t));
    (*inst)->maxDepth = requireDepth;
    (*inst)->colorReader = colorsReader;
    (*inst)->minFragScale = minFragScale;
    
    if(compressionFunctions == NULL)
    {
        (*inst)->functions.encoding_child_megre_func = _QTImage_DefaultMergeFunc; 
        (*inst)->functions.encoding_fragment_params_fill_func = _QTImage_DefaultParamsFill; 
        (*inst)->functions.encoding_parent_color_blend_func = _QTImage_DefaultColorBlend; 
    }
    else
        (*inst)->functions = *compressionFunctions;
}

void _QTImage_GetColorData_R(   QTImage inst, 
                                QuadTreeNode node, 
                                uint8_t** colorsData, 
                                struct QTImage_RecursiveParams recursiveParams)
{
    if(node == NULL)
        return;
    if(node->isLeaf == 1)
    {
        uint16_t fragScale = pow(2, recursiveParams.depth);
        uint32_t fragSizeX = (recursiveParams.x + fragScale) >= inst->width ? 
            (inst->width) : recursiveParams.x + fragScale; 
        uint32_t fragSizeY = (recursiveParams.y + fragScale) >= inst->height ? 
            (inst->height) : recursiveParams.y + fragScale; 

        for(uint32_t fx = recursiveParams.x; fx < fragSizeX; fx++)
            for(uint32_t fy = recursiveParams.y; fy < fragSizeY; fy++)            
                memcpy((*colorsData) + (inst->width * fy + fx) * 3, &(node->color), 3);       
              
        return;
    }
    recursiveParams.depth--;
    uint16_t nodeSize = pow(2, recursiveParams.depth);
    _QTImage_GetColorData_R(inst, node->childrens[0], colorsData,
        (struct QTImage_RecursiveParams){recursiveParams.depth, recursiveParams.x, recursiveParams.y});

    _QTImage_GetColorData_R(inst, node->childrens[1], colorsData,
        (struct QTImage_RecursiveParams){recursiveParams.depth, recursiveParams.x + nodeSize, recursiveParams.y});

    _QTImage_GetColorData_R(inst, node->childrens[2], colorsData,
        (struct QTImage_RecursiveParams){recursiveParams.depth, recursiveParams.x + nodeSize, recursiveParams.y + nodeSize});

    _QTImage_GetColorData_R(inst, node->childrens[3], colorsData,
        (struct QTImage_RecursiveParams){recursiveParams.depth, recursiveParams.x, recursiveParams.y + nodeSize});
}

ColorRGB _QTImage_FragmentBuilder(void* params, uint32_t x, uint32_t y)
{
    return ((struct QTImage_FragmentReadParams*)params)->ReadPixel(
        ((struct QTImage_FragmentReadParams*)params)->fileProvider, 
        x, y);    
}

ColorRGB _QTImage_FragmentBuilderRaw(void* params, uint32_t x, uint32_t y)
{

    return ((struct QTImage_FragmentRawReadParams*)params)->ReadPixel(
        ((struct QTImage_FragmentRawReadParams*)params)->inst, 
        ((struct QTImage_FragmentRawReadParams*)params)->rawData, 
        x, y);
}

double _QTImage_HueErrorAbs(double average, double val)
{
    double error = fabs(average - val);
    if(error > 180.0)
        error = 360.0 - error;
    return error;
}

void _QTImage_DefaultParamsFill(QuadTreeNode node)
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
            params->maxErrorH = fmax(params->maxErrorH, _QTImage_HueErrorAbs(params->averageH, colorsHSV[i].h));
            params->minV = fmin(params->minV, ((QTImage_Encoding_Fragment_params*)(node->childrens[i]->dataPtr))->minV);
            params->maxV = fmax(params->maxV, ((QTImage_Encoding_Fragment_params*)(node->childrens[i]->dataPtr))->maxV);
            params->minS = fmin(params->minS, ((QTImage_Encoding_Fragment_params*)(node->childrens[i]->dataPtr))->minS);
            params->maxS = fmax(params->maxS, ((QTImage_Encoding_Fragment_params*)(node->childrens[i]->dataPtr))->maxS);

        }
        
    }
    
    node->dataPtr = params;
    node->dataSize = sizeof(QTImage_Encoding_Fragment_params);
}

void _QTImage_DefaultColorBlend(QuadTreeNode node)
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

uint8_t _QTImage_DefaultMergeFunc(QuadTreeNode node)
{
    QTImage_Encoding_Fragment_params* params = (QTImage_Encoding_Fragment_params*)node->dataPtr;  
    
    double maxErrorS = 0, maxErrorV = 0, maxErrorH = 0;
    for(int i = 0; i < 4; i++)
    {
        
        if(node->childrens[i] == NULL) continue;
        ColorHSV hsv = Colors_rgb2hsv(node->childrens[i]->color);
        maxErrorS = fmax(maxErrorS, fabs(params->averageS - hsv.s));
        maxErrorV = fmax(maxErrorV, fabs(params->averageV - hsv.v));
        maxErrorH = fmax(maxErrorH, _QTImage_HueErrorAbs(params->averageH, hsv.h));
    }

    double dV = params->maxV - params->minV;
    double dS = params->maxS - params->minS;

    free(params);
    node->dataPtr = NULL;
    node->dataSize = 0;
    if(dV < 0.01 && dS < 0.01 && params->maxErrorH < 3)
        return 1;        
    return 0;
}


