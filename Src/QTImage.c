#include "../Includes/QTImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double HueErrorAbs(double average, double val)
{
    double error = fabs(average - val);
    if(error > 180.0)
        error = 360.0 - error;
    return error;
}

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

    if(dV < 0.1 && dS < 0.1 && params->maxErrorH < 15)
        return 1;        
    return 0;
}



ColorRGB _calculateFragmentColor(BMPImage* bmp, uint32_t startX, uint32_t startY, 
                                uint32_t endX, uint32_t endY)
{
    uint32_t r = 0, g = 0, b = 0;
    uint64_t fragCount = (endX - startX) * (endY - startY); 
    ColorRGB fragColor;
    ColorRGB subFragColor;
    
    for(uint32_t fx = startX; fx < endX; fx++)
    {
        for(uint32_t fy = startY; fy < endY; fy++)
        {
            subFragColor = BMPImage_GetPixel(bmp, fx, fy);
            r+=subFragColor.rgba[0];
            g+=subFragColor.rgba[1];
            b+=subFragColor.rgba[2];
        }
    }
    fragColor.rgba[0] = r / fragCount;
    fragColor.rgba[1] = g / fragCount;
    fragColor.rgba[2] = b / fragCount;
    fragColor.rgba[3] = 0;   
    return fragColor;     
} 

void _decode(BMPImage* bmp, QuadTreeNode* treeNode, uint8_t depth, uint8_t requireDepth, 
            unsigned int x, unsigned int y)
{
    if(x >= bmp->infoHeader.width || y >= bmp->infoHeader.height) return;
    if(treeNode->isLeaf == 1)
    {
        uint16_t fragScale = pow(2, requireDepth);
        uint16_t fragSizeX = (x + fragScale) > bmp->infoHeader.width ? (fragScale - (x + fragScale -  bmp->infoHeader.width)) : fragScale; 
        uint16_t fragSizeY = (y + fragScale) > bmp->infoHeader.height ? (fragScale - (y + fragScale -  bmp->infoHeader.height)) : fragScale; 
        unsigned char depthColor_r = (12 - depth) * 20;
        ColorHSV hsvColor = Colors_rgb2hsv(treeNode->color);
        ColorRGB depthColor;
        depthColor.ecolor = 0;
        depthColor.rgba[2] = depthColor_r;
        for(uint16_t fx = 0; fx < fragSizeX; fx++)
            for(uint16_t fy = 0; fy < fragSizeY; fy++)
                BMPImage_SetPixel(bmp, x + fx, y + fy, treeNode->color);
        return;
    }
    uint16_t nodeSize = pow(2, requireDepth - 1);
    _decode(bmp, treeNode->childrens[0], depth - 1, requireDepth - 1, x, y);
    _decode(bmp, treeNode->childrens[1], depth - 1, requireDepth - 1, x + nodeSize, y);
    _decode(bmp, treeNode->childrens[2], depth - 1, requireDepth - 1, x + nodeSize, y + nodeSize);
    _decode(bmp, treeNode->childrens[3], depth - 1, requireDepth - 1, x, y + nodeSize);
}

QuadTreeNode* _encode(QTImage_Encode_r_params params)
{
    
    if(params.x >= params.bmp->infoHeader.width || params.y >= params.bmp->infoHeader.height) return NULL;
    QuadTreeNode* parent = QuadTreeNode_ctor();
    if(params.depth == 0)
    {
        uint16_t fragSizeX = (params.x + params.minFragScale) > params.bmp->infoHeader.width ? 
            (params.minFragScale - (params.x + params.minFragScale -  params.bmp->infoHeader.width)) : params.minFragScale; 
        uint16_t fragSizeY = (params.y + params.minFragScale) > params.bmp->infoHeader.height ? 
            (params.minFragScale - (params.y +params. minFragScale -  params.bmp->infoHeader.height)) : params.minFragScale; 

        parent->color = _calculateFragmentColor(params.bmp, params.x, params.y, params.x + fragSizeX, params.y + fragSizeY);
        parent->isLeaf = 1;
        return parent;
    }
    
    int nodeSize = pow(2.0, params.maxDepth - 1);
    
    parent->childrens[0] = _encode((QTImage_Encode_r_params){
        params.bmp, params.maxDepth - 1, params.depth - 1, params.x, params.y, params.minFragScale, 
        params.encoding_fragment_params_fill_func, params.encoding_parent_color_blend_func, params.encoding_child_megre_func});  
    parent->childrens[1] = _encode((QTImage_Encode_r_params){
        params.bmp, params.maxDepth - 1, params.depth - 1, params.x + nodeSize, params.y, params.minFragScale, 
        params.encoding_fragment_params_fill_func, params.encoding_parent_color_blend_func, params.encoding_child_megre_func});            
    parent->childrens[2] = _encode((QTImage_Encode_r_params){
        params.bmp, params.maxDepth - 1, params.depth - 1, params.x + nodeSize, params.y + nodeSize, params.minFragScale, 
        params.encoding_fragment_params_fill_func, params.encoding_parent_color_blend_func, params.encoding_child_megre_func});       
    parent->childrens[3] = _encode((QTImage_Encode_r_params){
        params.bmp, params.maxDepth - 1, params.depth - 1, params.x, params.y + nodeSize, params.minFragScale, 
        params.encoding_fragment_params_fill_func, params.encoding_parent_color_blend_func, params.encoding_child_megre_func});                   

    if(parent->childrens[0] == NULL && parent->childrens[1] == NULL &&
       parent->childrens[2] == NULL && parent->childrens[3] == NULL)
       {
            QuadTreeNode_dctor(&parent);
            return NULL;
       }

    params.encoding_fragment_params_fill_func(parent);
    params.encoding_parent_color_blend_func(parent);
    if(params.encoding_child_megre_func(parent))
    {
        for( int i = 0; i < 4; i++)
            if(parent->childrens[i] != NULL)
                QuadTreeNode_dctor(&(parent->childrens[i]));  
        parent->isLeaf = 1;
    }
    return parent;
}

QTImage* QTImage_Ctor()
{
    QTImage* inst = malloc(sizeof(QTImage));
    inst->rootNode = NULL;
    inst->width = 0;
    inst->height = 0;
    return inst;
}

void QTImage_Dctor(QTImage** inst)
{
    if(*inst == NULL) return;
    if((*inst)->rootNode != NULL)
        QuadTreeNode_dctor(&((*inst)->rootNode));
    free((*inst));
    *inst = NULL;
}

void QTIMAGECALL QTImage_Serialize(QTImage* inst, char* filepath)
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

QTImage* QTIMAGECALL QTImage_Deserialize(char* filepath)
{
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }    
    QTImage* qtImage = QTImage_Ctor();   
    fread(qtImage, sizeof(uint32_t), 2, fp);

    fseek(fp, -8l, SEEK_END);
    size_t dataSize = ftell(fp); 
    fseek(fp, (sizeof(int32_t) * 2), SEEK_SET);

    uint8_t* data = calloc(dataSize, sizeof(uint8_t)); 
    fread(data, sizeof(uint8_t), dataSize, fp);

    qtImage->rootNode = QuadTreeNode_Deserialize(data, dataSize);

    fclose(fp);
    return qtImage;    
}

size_t QTImage_AllocSize(QTImage* inst)
{   
    return QuadTreeNode_AllocSize(inst->rootNode);
}

QTImage* QTIMAGECALL QTImage_Encode(QTImage_Encode_params params)
{
    if(params.filename == NULL)
        exit(-10);
    
    BMPImage* bmp = BMPImage_FromFile(params.filename);
    if(bmp == NULL)
        exit(-11);    

    if(params.encoding_child_megre_func == NULL)
        params.encoding_child_megre_func = QTImage_Encoding_Merge_func;
    if(params.encoding_fragment_params_fill_func == NULL)
        params.encoding_fragment_params_fill_func = QTImage_Encoding_Fragment_params_fill_func;
    if(params.encoding_parent_color_blend_func == NULL)
        params.encoding_parent_color_blend_func = QTImage_Encoding_Parent_Color_Blend_func;
    

    uint8_t requireTreeDepth = (uint8_t)ceil(log2(bmp->infoHeader.width > bmp->infoHeader.height ?  bmp->infoHeader.width : bmp->infoHeader.height));
    uint8_t treeDepth = requireTreeDepth >= params.maxDepth ? params.maxDepth : requireTreeDepth;
    uint16_t minFragScale = pow(2, requireTreeDepth - treeDepth); 
    
    QuadTreeNode* node =_encode((QTImage_Encode_r_params){bmp, params.maxDepth, treeDepth, 0, 0, minFragScale,
                                params.encoding_fragment_params_fill_func, params.encoding_parent_color_blend_func, params.encoding_child_megre_func});

    
    QTImage* compressedImage = QTImage_Ctor();
    compressedImage->height = bmp->infoHeader.height;
    compressedImage->width = bmp->infoHeader.width;
    compressedImage->rootNode = node;
    compressedImage->depth = treeDepth; 


    BMPImage_Dctor(&bmp);
    return compressedImage;
}

void QTIMAGECALL QTImage_Decode(QTImage* qtImage, char* filename)
{
    uint8_t requireTreeDepth = (uint8_t)ceil(log2(qtImage->width > qtImage->height ?  qtImage->width : qtImage->height));
    BMPImage* bmp = BMPImage_Ctor(qtImage->width, qtImage->height, 24);
    _decode(bmp, qtImage->rootNode, qtImage->depth, requireTreeDepth, 0, 0);    
    BMPImage_Save(bmp, filename);
    BMPImage_Dctor(&bmp);    
}
