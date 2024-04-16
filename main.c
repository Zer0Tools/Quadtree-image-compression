#include "Src/BMPImage.c"
#include "Includes/QTImage.h"

int main(int argc, char* argv[])
{
    struct QTImage_FileReader_t fileReader = {BMPImage_FromFile, BMPImage_Dctor, BMPImage_GetWidth, BMPImage_GetHeight, BMPImage_GetPixel};    
    QTImage qtImage = QTImage_Ctor(); 
    qtImage->depth = 6;
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
    return 0;
}