#include "../Includes/BitArray.h"
#include <malloc.h>
#include <math.h>
#include <memory.h>


BitArray* BitArray_Ctor(uint64_t count)
{
    BitArray* inst = calloc(1, sizeof(BitArray));
    if(count == 0)
        exit(-2);
    inst->size = count;
    inst->real_size = (size_t)ceil((float)sizeof(uint8_t) / 8.0);
    inst->data = calloc(count, inst->real_size);
    return inst;
}
void BitArray_Dctor(BitArray** inst)
{
    free((*inst)->data);
    free(*inst);
    *inst = NULL;
}

void BitArray_InjectData(BitArray* inst, void* data, size_t dataSize)
{
    if(inst->real_size < dataSize)
        exit(-2);
    memcpy(inst->data, data, dataSize);
}

uint8_t BitArray_Get(BitArray* inst, size_t index, uint8_t blockSize)
{
    size_t byteIndex = index / 8;
    uint8_t bitIndex = index - byteIndex;
    uint8_t val = inst->data[byteIndex];
    return (val & (blockSize << bitIndex)) >> index;
}

void BitArray_Set(BitArray* inst, size_t index, uint8_t value, uint8_t blockSize)
{
    size_t byteIndex = index / 8;
    uint8_t bitIndex = index - byteIndex;
    uint8_t val = inst->data[byteIndex];
    uint8_t mask = blockSize << bitIndex; 
    val = val & ~(mask);
    val+= (value << bitIndex) & mask; 
    inst->data[byteIndex] = val;
    
}