#include <stdlib.h>
#include <stdio.h>
#include "../Includes/QuadTreeNode.h"

QuadTreeNode QuadTreeNode_Ctor()
{
    QuadTreeNode node = calloc(1, sizeof(struct QuadTreeNode_t));    
    return node;
}

void QuadTreeNode_Dctor(QuadTreeNode* node)
{
    if((*node)->isLeaf == 0)
    {
        for(int i = 0; i < 4; i++)
            if(((*node)->childrens)[i] != NULL)
                QuadTreeNode_Dctor(&(*node)->childrens[i]);
    }
    free(*node);
    *node = NULL;
}

size_t QuadTreeNode_NodesCount(QuadTreeNode node)
{
    return _QuadTreeNode_NodesCount_R(node);
}

size_t QuadTreeNode_LeafsCount(QuadTreeNode node)
{
    return _QuadTreeNode_LeafsCount_R(node);    
}

QuadTreeNode QuadTreeNode_Deserialize(uint8_t* data, size_t dataSize, uint8_t* maxDepth)
{
    dataSize-=1;
    return _QuadTreeNode_Deserialize_R(data, &dataSize, 0, maxDepth);
}

size_t QuadTreeNode_Serialize(QuadTreeNode inst, uint8_t** data)
{
    size_t nodesCount = QuadTreeNode_NodesCount(inst), leafsCount = QuadTreeNode_LeafsCount(inst);
    (*data) = calloc(leafsCount * 3 + nodesCount, sizeof(uint8_t));
    _QuadTreeNode_Serialize_R(inst, (*data), 0);
    return leafsCount * 3 + nodesCount;    
}

static QuadTreeNode _QuadTreeNode_Deserialize_Leaf(uint8_t* data, size_t* dataOffset)
{
    QuadTreeNode leafNode = QuadTreeNode_Ctor();
    leafNode->color.rgba[0] = data[*dataOffset];
    leafNode->color.rgba[1] = data[*dataOffset + 1];
    leafNode->color.rgba[2] = data[*dataOffset + 2];
    leafNode->isLeaf = 1;
    return leafNode;
}

static QuadTreeNode _QuadTreeNode_Deserialize_R(uint8_t* data, size_t* dataOffset, uint8_t depth, uint8_t* maxDepth)
{
    depth++;
    if(depth > *maxDepth)
        *maxDepth = depth;

    uint8_t childmask = data[*dataOffset]; 
    QuadTreeNode node = QuadTreeNode_Ctor();
    for(int i = 3; i>= 0; i--)
    {
        if((childmask & (1 << i)) == (1 << i))
        {
            if((childmask & (1 << (4 + i))) == (1 << (4 + i)))
            {
                (*dataOffset) -= 3;
                node->childrens[i] = _QuadTreeNode_Deserialize_Leaf(data, dataOffset); 
            }
            else
            {
                (*dataOffset) -= 1;
                node->childrens[i] = _QuadTreeNode_Deserialize_R(data, dataOffset, depth, maxDepth);   
            }       
        }     
    }
    return node;   
}

static size_t _QuadTreeNode_NodesCount_R(QuadTreeNode node)
{
    if(node->isLeaf == 1 || node->childrens == NULL)
        return 0;
    size_t size =  1;
    for(int i = 0; i < 4; i++)
        if(node->childrens[i] != NULL)
            size+= _QuadTreeNode_NodesCount_R(node->childrens[i]);
    return size;
}

static size_t _QuadTreeNode_LeafsCount_R(QuadTreeNode node)
{
    if(node->isLeaf == 1 || node->childrens == NULL)
        return 1;
    size_t size =  0;
    for(int i = 0; i < 4; i++)
        if(node->childrens[i] != NULL)
            size+= _QuadTreeNode_LeafsCount_R(node->childrens[i]);
    return size;    
}

static size_t _QuadTreeNode_Serialize_R(QuadTreeNode node, uint8_t* data, size_t offset)
{
    if(node->isLeaf)
    {
        data[offset] = node->color.rgba[0];
        data[offset + 1] = node->color.rgba[1];
        data[offset + 2] = node->color.rgba[2];
        return 3;
    }
    size_t totalOffset = 0;
    uint8_t childmask = 0;
    for(int i = 0; i < 4; i++)
    {
        if(node->childrens[i] != NULL)
        {
            childmask |= (1 << i);     
            if(node->childrens[i]->isLeaf == 1)
                childmask |= (1 << (4 + i));       
            totalOffset+= _QuadTreeNode_Serialize_R(node->childrens[i], data, offset + totalOffset);            
        }
    }
    data[offset + totalOffset] = childmask; 
    return totalOffset + 1;
}



