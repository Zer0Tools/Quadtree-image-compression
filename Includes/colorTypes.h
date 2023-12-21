#ifndef COLOR_H
#define COLOR_H
#include <ctype.h>
#include <stdint.h>

typedef struct color_rgb_t  
{
    union
    {
        uint8_t rgba[4];
        uint32_t ecolor;
    };
} ColorRGB;

typedef struct color_hsv_t  
{
    double h;
    double s;
    double v;
} ColorHSV;


ColorHSV Colors_rgb2hsv(ColorRGB rgb);
ColorRGB Colors_hsv2rgb(ColorHSV hsv);

#endif