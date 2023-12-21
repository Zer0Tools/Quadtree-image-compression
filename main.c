#include "Includes/QTImage.h"
int main(int argc, char* argv[])
{

    QTImage_Encode_params params = {"temp/test.bmp", 12, NULL, NULL, NULL};
    QTImage* qtImage = QTImage_Encode(params);
    
    QTImage_Serialize(qtImage, "temp/test.qti");
    QTImage* qtImageDeser = QTImage_Deserialize("temp/test.qti");
    QTImage_Decode(qtImageDeser, "temp/decoded_test_01.bmp");
    return 0;
}