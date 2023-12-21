#include <stdlib.h>
#include <stdio.h>
#include "../Includes/QuadTreeNode.h"

QuadTreeNode* QuadTreeNode_ctor()
{
    QuadTreeNode* node = calloc(1, sizeof(QuadTreeNode));    
    return node;
}

void QuadTreeNode_dctor(QuadTreeNode** nodePtr)
{
    if((*nodePtr)->isLeaf == 0)
    {
        for(int i = 0; i < 4; i++)
            if(((*nodePtr)->childrens)[i] != NULL)
                QuadTreeNode_dctor(&(*nodePtr)->childrens[i]);
    }
    if((*nodePtr)->dataPtr != NULL)
        free((*nodePtr)->dataPtr);
    free(*nodePtr);
    *nodePtr = NULL;
}

size_t _QuadTreeNode_AllocSize(QuadTreeNode* nodePtr)
{
    if(nodePtr->isLeaf == 1)
        return sizeof(QuadTreeNode);
    size_t size = 0;
    for(int i = 0; i < 4; i++)
        if(nodePtr->childrens[i] != NULL)
            size+= _QuadTreeNode_AllocSize(nodePtr->childrens[i]);
    return size;
}

size_t QuadTreeNode_AllocSize(QuadTreeNode* nodePtr)
{
    return _QuadTreeNode_AllocSize(nodePtr);
}

size_t QuadTreeNode_NodesCount(QuadTreeNode* nodePtr)
{
    return _QuadTreeNode_NodesCount_R(nodePtr);
}

size_t QuadTreeNode_LeafsCount(QuadTreeNode* nodePtr)
{
    return _QuadTreeNode_LeafsCount_R(nodePtr);    
}

QuadTreeNode* QuadTreeNode_Deserialize(uint8_t* data, size_t dataSize)
{
    dataSize-=1;
    return _QuadTreeNode_Deserialize_R(data, &dataSize);
}

size_t QuadTreeNode_Serialize(QuadTreeNode* inst, uint8_t** data)
{
    size_t nodesCount = QuadTreeNode_NodesCount(inst), leafsCount = QuadTreeNode_LeafsCount(inst);
    (*data) = calloc(leafsCount * 3 + nodesCount, sizeof(uint8_t));
    _QuadTreeNode_Serialize_R(inst, (*data), 0, 0);
    return leafsCount * 3 + nodesCount;    
}

static QuadTreeNode* _QuadTreeNode_Deserialize_Leaf(uint8_t* data, size_t* dataOffset)
{
    QuadTreeNode* leafNode = QuadTreeNode_ctor();
    leafNode->color.rgba[0] = data[*dataOffset];
    leafNode->color.rgba[1] = data[*dataOffset + 1];
    leafNode->color.rgba[2] = data[*dataOffset + 2];
    leafNode->isLeaf = 1;
    return leafNode;
}

static QuadTreeNode* _QuadTreeNode_Deserialize_R(uint8_t* data, size_t* dataOffset)
{
    uint8_t childmask = data[*dataOffset]; 
    QuadTreeNode* node = QuadTreeNode_ctor();
    if((childmask & 8) == 8)
    {
        if((childmask & 128) == 128)
        {
            (*dataOffset) -= 3;
            node->childrens[3] = _QuadTreeNode_Deserialize_Leaf(data, dataOffset); 
        }
        else
        {
            (*dataOffset) -= 1;
            node->childrens[3] = _QuadTreeNode_Deserialize_R(data, dataOffset);   
        }       
    }      
    if((childmask & 4) == 4)
    {
        if((childmask & 64) == 64)
        {
            (*dataOffset) -= 3;
            node->childrens[2] = _QuadTreeNode_Deserialize_Leaf(data, dataOffset); 
        }
        else
        {
            (*dataOffset) -= 1;
            node->childrens[2] = _QuadTreeNode_Deserialize_R(data, dataOffset);   
        }      
    }       
    if((childmask & 2) == 2)
    {
        if((childmask & 32) == 32)
        {
            (*dataOffset) -= 3;
            node->childrens[1] = _QuadTreeNode_Deserialize_Leaf(data, dataOffset); 
        }
        else
        {
            (*dataOffset) -= 1;
            node->childrens[1] = _QuadTreeNode_Deserialize_R(data, dataOffset);   
        }       
    }  
    if((childmask & 1) == 1)
    {
        if((childmask & 16) == 16)
        {
            (*dataOffset) -= 3;
            node->childrens[0] = _QuadTreeNode_Deserialize_Leaf(data, dataOffset); 
        }
        else
        {
            (*dataOffset) -= 1;
            node->childrens[0] = _QuadTreeNode_Deserialize_R(data, dataOffset);   
        }            
    }              
    return node;   
}

static size_t _QuadTreeNode_NodesCount_R(QuadTreeNode* nodePtr)
{
    if(nodePtr->isLeaf == 1 || nodePtr->childrens == NULL)
        return 0;
    size_t size =  1;
    for(int i = 0; i < 4; i++)
        if(nodePtr->childrens[i] != NULL)
            size+= _QuadTreeNode_NodesCount_R(nodePtr->childrens[i]);
    return size;
}

static size_t _QuadTreeNode_LeafsCount_R(QuadTreeNode* nodePtr)
{
    if(nodePtr->isLeaf == 1 || nodePtr->childrens == NULL)
        return 1;
    size_t size =  0;
    for(int i = 0; i < 4; i++)
        if(nodePtr->childrens[i] != NULL)
            size+= _QuadTreeNode_LeafsCount_R(nodePtr->childrens[i]);
    return size;    
}

static size_t _QuadTreeNode_Serialize_R(QuadTreeNode* nodePtr, uint8_t* data, size_t offset, size_t depth)
{
    if(nodePtr->isLeaf)
    {
        data[offset] = nodePtr->color.rgba[0];
        data[offset + 1] = nodePtr->color.rgba[1];
        data[offset + 2] = nodePtr->color.rgba[2];
        return 3;
    }
    size_t totalOffset = 0;
    uint8_t childmask = 0;
    if(nodePtr->childrens[0] != NULL)
    {
        childmask |= 1;
        if(nodePtr->childrens[0]->isLeaf == 1)
            childmask |= 16;
        totalOffset+= _QuadTreeNode_Serialize_R(nodePtr->childrens[0], data, offset, depth + 1); // 3        
    }
    if(nodePtr->childrens[1] != NULL)
    {
        childmask |= 2;
        if(nodePtr->childrens[1]->isLeaf == 1)
            childmask |= 32;        
        totalOffset+= _QuadTreeNode_Serialize_R(nodePtr->childrens[1], data, offset + totalOffset, depth + 1); // 6        
    }
    if(nodePtr->childrens[2] != NULL)
    {
        childmask |= 4;
        if(nodePtr->childrens[2]->isLeaf == 1)
            childmask |= 64;             
        totalOffset+= _QuadTreeNode_Serialize_R(nodePtr->childrens[2], data, offset + totalOffset, depth + 1); // 9       
    }  
    if(nodePtr->childrens[3] != NULL)
    {
        childmask |= 8;
        if(nodePtr->childrens[3]->isLeaf == 1)
            childmask |= 128;            
        totalOffset+= _QuadTreeNode_Serialize_R(nodePtr->childrens[3], data, offset + totalOffset, depth + 1); // 12        
    } 
    data[offset + totalOffset] = childmask; 
    return totalOffset + 1;
}


static uint8_t _QuadTreeNode_ReadMask(uint8_t* data, size_t offset)
{
    return 0;    
}

