#ifndef QUAD_TREE_NODE_H
#define QUAD_TREE_NODE_H

#include "colorTypes.h"


#define QuadtreeNode_SetChildren(node, index, child) ((node)->childrens[index] = child)

typedef struct quadtreeNode_t
{
    uint8_t isLeaf;
    struct quadtreeNode_t* childrens[4];
    ColorRGB color;
    uint32_t dataSize;
    void* dataPtr;
} QuadTreeNode;


QuadTreeNode* QuadTreeNode_ctor();
void QuadTreeNode_dctor(QuadTreeNode** nodePtr);
size_t QuadTreeNode_AllocSize(QuadTreeNode* nodePtr);
size_t QuadTreeNode_NodesCount(QuadTreeNode* nodePtr);
size_t QuadTreeNode_LeafsCount(QuadTreeNode* nodePtr);
size_t QuadTreeNode_Serialize(QuadTreeNode* nodePtr, uint8_t** data);
QuadTreeNode* QuadTreeNode_Deserialize(uint8_t* data, size_t dataSize);

static size_t _QuadTreeNode_AllocSize(QuadTreeNode* nodePtr);
static size_t _QuadTreeNode_NodesCount_R(QuadTreeNode* nodePtr);
static size_t _QuadTreeNode_LeafsCount_R(QuadTreeNode* nodePtr);
static size_t _QuadTreeNode_Serialize_R(QuadTreeNode* nodePtr, uint8_t* data, size_t offset, size_t depth);
static QuadTreeNode*   _QuadTreeNode_Deserialize_R(uint8_t* data, size_t* dataSize);

static uint8_t _QuadTreeNode_ReadMask(uint8_t* data, size_t offset);

#endif