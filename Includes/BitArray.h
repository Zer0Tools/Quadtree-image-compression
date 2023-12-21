#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H
#include <stdint.h>

#define BIT_ARRAY_BLOCK_SIZE_1 1
#define BIT_ARRAY_BLOCK_SIZE_2 3
#define BIT_ARRAY_BLOCK_SIZE_3 7
#define BIT_ARRAY_BLOCK_SIZE_4 15
#define BIT_ARRAY_BLOCK_SIZE_5 31
#define BIT_ARRAY_BLOCK_SIZE_6 63
#define BIT_ARRAY_BLOCK_SIZE_7 127
#define BIT_ARRAY_BLOCK_SIZE_8 255


typedef struct BitArray_t
{
    uint8_t* data;
    size_t size;
    size_t real_size;
}BitArray;

BitArray* BitArray_Ctor(uint64_t count);
void BitArray_Dctor(BitArray** inst);

void BitArray_InjectData(BitArray* inst, void* data, size_t dataSize);

uint8_t BitArray_Get(BitArray* inst, size_t index, uint8_t blockSize);

void BitArray_Set(BitArray* inst, size_t index, uint8_t value, uint8_t blockSize);




#endif