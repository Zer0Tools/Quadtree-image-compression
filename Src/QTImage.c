#include "../Includes/QTImage.h"
#include <stdio.h>
#include <stdlib.h>


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
        // hsvColor.s = 1.0;
        // hsvColor.v = 1.0;
        // ColorRGB rgbColor = Colors_hsv2rgb(hsvColor);
        for(uint16_t fx = 0; fx < fragSizeX; fx++)
            for(uint16_t fy = 0; fy < fragSizeY; fy++)
                BMPImage_SetPixel(bmp, x + fx, y + fy, treeNode->color);
                //BMPImage_SetPixel(bmp, x + fx, y + fy, depthColor);
                //set_pixel_rgb(bmp, x + fx, y + fy, hsvColor.s * 255.0, hsvColor.s * 255.0, hsvColor.s * 255.0);
                //set_pixel_rgb(bmp, x + fx, y + fy, hsvColor.h * 0.7, hsvColor.h * 0.7, hsvColor.h * 0.7);
                //set_pixel_rgb(bmp, x + fx, y + fy, rgbColor.rgba[0], rgbColor.rgba[1], rgbColor.rgba[2]);
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
    //printf("encoding %d || %d\n", x, y);
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

void QTImage_Serialize(QTImage* inst, char* filepath)
{
    FILE* fp = fopen(filepath, "wb");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }    
    uint8_t* serializedData = NULL;
    size_t dataSize = QuadTreeNode_Serialize(inst->rootNode, &serializedData);
    printf("%d \n", dataSize);
    fwrite(inst, sizeof(uint32_t), 2, fp); // save width/height;
    fwrite(serializedData, sizeof(uint8_t), dataSize, fp); // save data;
    fclose(fp);
}
QTImage* QTImage_Deserialize(char* filepath)
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

QTImage* QTImage_Encode(QTImage_Encode_params params)
{
    uint8_t requireTreeDepth = (uint8_t)ceil(log2(params.bmp->infoHeader.width > params.bmp->infoHeader.height ?  params.bmp->infoHeader.width :params.bmp->infoHeader.height));
    uint8_t treeDepth = requireTreeDepth >= params.maxDepth ? params.maxDepth : requireTreeDepth;
    uint16_t minFragScale = pow(2, requireTreeDepth - treeDepth); 
   
    QuadTreeNode* node =_encode((QTImage_Encode_r_params){params.bmp, params.maxDepth, treeDepth, 0, 0, minFragScale,
                                params.encoding_fragment_params_fill_func, params.encoding_parent_color_blend_func, params.encoding_child_megre_func});

    QTImage* compressedImage = QTImage_Ctor();
    compressedImage->height = params.bmp->infoHeader.height;
    compressedImage->width = params.bmp->infoHeader.width;
    compressedImage->rootNode = node;
    compressedImage->depth = treeDepth; 
    return compressedImage;
}

void QTImage_Decode(QTImage* qtImage, BMPImage** bmp)
{
    uint8_t requireTreeDepth = (uint8_t)ceil(log2(qtImage->width > qtImage->height ?  qtImage->width : qtImage->height));
    *bmp = BMPImage_Ctor(qtImage->width, qtImage->height, 24);

    _decode(*bmp, qtImage->rootNode, qtImage->depth, requireTreeDepth, 0, 0);      
}
