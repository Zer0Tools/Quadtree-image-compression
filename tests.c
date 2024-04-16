#include "Src/BMPImage.c"
#include "Includes/QTImage.h"
#include "stdio.h"


ColorRGB ReadRaw(QTImage inst, uint8_t* rawData, uint32_t x, uint32_t y)
{
    ColorRGB color;
    memcpy(&color, rawData + (x + inst->width * y) * 3, 3);
    return color;
}

int main(int argc, char* argv[])
{

    struct QTImage_FileReader_t fileReader = {BMPImage_FromFile, BMPImage_Dctor, BMPImage_GetWidth, BMPImage_GetHeight, BMPImage_GetPixel};    
    QTImage qtImage = QTImage_Ctor(); 
    qtImage->depth = 0;
    QTImage_CreateFromFile(qtImage, "tests/Test.bmp", &fileReader, NULL);
    QTImage_Save(qtImage, "tests/Test.qti");
    QTImage_Dctor(&qtImage);

    QTImage loadedImage1 = QTImage_Ctor(); 
    QTImage_Load(loadedImage1, "tests/Test.qti");  

    uint8_t* data1 = NULL;
    QTImage_GetColorData(loadedImage1, &data1);  

    BMPImage* bmp1 = BMPImage_Ctor(loadedImage1->width, loadedImage1->height, 24);
    bmp1->pixelBuffer = data1;
    BMPImage_Save(bmp1, "tests/DecodedTest1.bmp");

    BMPImage_Dctor(&bmp1);  
    QTImage_Dctor(&loadedImage1);  

    qtImage = QTImage_Ctor(); 
    qtImage->depth = 0;
    BMPImage* testBmp = BMPImage_FromFile("tests/Test.bmp");
    qtImage->width = testBmp->infoHeader.width;
    qtImage->height = testBmp->infoHeader.height;

    struct QTImage_RawColorDataReader_t rawReader = {ReadRaw};
    QTImage_CreateFromRaw(qtImage, testBmp->pixelBuffer , &rawReader, NULL);
    BMPImage_Dctor(&testBmp);

    QTImage_Save(qtImage, "tests/TestRaw.qti");
    QTImage_Dctor(&qtImage);

    QTImage loadedImage = QTImage_Ctor(); 
    QTImage_Load(loadedImage, "tests/TestRaw.qti");
    uint8_t* data = NULL;
    QTImage_GetColorData(loadedImage, &data);
    BMPImage* bmp = BMPImage_Ctor(loadedImage->width, loadedImage->height, 24);
    bmp->pixelBuffer = data;
    BMPImage_Save(bmp, "tests/DecodedTest.bmp");
}