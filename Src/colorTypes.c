#include "../Includes/colorTypes.h"
#include "math.h"

ColorHSV Colors_rgb2hsv(ColorRGB rgb)
{
    ColorHSV         out;
    double r = (double)rgb.rgba[0] / 255.0;
    double g = (double)rgb.rgba[1] / 255.0;
    double b = (double)rgb.rgba[2] / 255.0;
    double      min, max, delta;

    min = r < g ? r : g;
    min = min  < b ? min : b;

    max = r > g ? r : g;
    max = max  > b ? max : b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) 
    { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } 
    else 
    {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( g - b) / delta;        // between yellow & magenta
    else
    if( g >= max )
        out.h = 2.0 + ( b - r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( r - g) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;    
}
ColorRGB Colors_hsv2rgb(ColorHSV hsv)
{
    double      hh, p, q, t, ff;
    long        i;
    ColorRGB         out;

    if(hsv.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.rgba[0] = (uint8_t)(hsv.v * 255.0);
        out.rgba[1] = (uint8_t)(hsv.v * 255.0);
        out.rgba[2] = (uint8_t)(hsv.v * 255.0);
        return out;
    }
    hh = hsv.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = hsv.v * (1.0 - hsv.s);
    q = hsv.v * (1.0 - (hsv.s * ff));
    t = hsv.v * (1.0 - (hsv.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.rgba[0] = hsv.v * 255.0;
        out.rgba[1] = t * 255.0;
        out.rgba[2] = p * 255.0;
        break;
    case 1:
        out.rgba[0] = q * 255.0;
        out.rgba[1] = hsv.v * 255.0;
        out.rgba[2] = p * 255.0;
        break;
    case 2:
        out.rgba[0] = p * 255.0; 
        out.rgba[1] = hsv.v * 255.0;
        out.rgba[2] = t * 255.0;
        break;

    case 3:
        out.rgba[0] = p * 255.0;
        out.rgba[1] = q * 255.0;
        out.rgba[2] = hsv.v * 255.0;
        break;
    case 4:
        out.rgba[0] = t * 255.0;
        out.rgba[1] = p * 255.0;
        out.rgba[2] = hsv.v * 255.0;
        break;
    case 5:
    default:
        out.rgba[0] = hsv.v * 255.0;
        out.rgba[1] = p * 255.0;
        out.rgba[2] = q * 255.0;
        break;
    }
    return out;       
}