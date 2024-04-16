#ifndef QUAD_TREE_NODE_H
#define QUAD_TREE_NODE_H

#include "colorTypes.h"


#define QuadtreeNode_SetChildren(node, index, child) ((node)->childrens[index] = child)

struct QuadTreeNode_t
{
    uint8_t isLeaf;
    struct QuadTreeNode_t* childrens[4];
    ColorRGB color;
    uint32_t dataSize;
    void* dataPtr;
};
typedef struct QuadTreeNode_t* QuadTreeNode;




QuadTreeNode QuadTreeNode_Ctor();
void QuadTreeNode_Dctor(QuadTreeNode* node);
size_t QuadTreeNode_NodesCount(QuadTreeNode node);
size_t QuadTreeNode_LeafsCount(QuadTreeNode node);
size_t QuadTreeNode_Serialize(QuadTreeNode node, uint8_t** data);
QuadTreeNode QuadTreeNode_Deserialize(uint8_t* data, size_t dataSize, uint8_t* maxDepth);

static size_t _QuadTreeNode_NodesCount_R(QuadTreeNode node);
static size_t _QuadTreeNode_LeafsCount_R(QuadTreeNode node);
static size_t _QuadTreeNode_Serialize_R(QuadTreeNode node, uint8_t* data, size_t offset);
static QuadTreeNode _QuadTreeNode_Deserialize_R(uint8_t* data, size_t* dataSize, uint8_t depth, uint8_t* maxDepth);

#endif