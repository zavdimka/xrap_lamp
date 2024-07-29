#pragma once
#include <QtCore>

class Pixel{
    protected:
        uint8_t *ptr;

    public:
        uint8_t *R;
        uint8_t *G;
        uint8_t *B;

        Pixel(uint8_t *ptr): 
            ptr(ptr),
            R(ptr + 1),
            G(ptr + 0),
            B(ptr + 2)
            {
        }

        uint32_t getColor(){
            uint32_t t = *((uint32_t*)ptr) & 0xFFFFFF;
            return (t & 0xFF00) >> 8 | (t & 0xFF) << 8 | (t & 0xFF0000);
        }

        void setColor(uint32_t t){
            t &= 0xFFFFFF;
            t  = (t & 0xFF00) >> 8 | (t & 0xFF) << 8 | (t & 0xFF0000);
            *((uint32_t*)ptr) &= ~0xFFFFFF;
            *((uint32_t*)ptr) |= t;
        }

        void fade(uint8_t step){
            if (*R > 4 || *G > 4 || *B > 4){
                nscale8x3(R, G, B, 255 - step);
            } else {
                setColor(0);
            }
        }

        void setHSV(uint8_t h, uint8_t s, uint8_t v){
            uint8_t r, g, b;
            uint8_t region, remainder, p, q, t;
            if (s == 0)
            {
                r = v;
                g = v;
                b = v;
            } else {
                region = h / 43;
                remainder = (h - (region * 43)) * 6; 
                
                p = ((uint16_t)v * (255 - s)) >> 8;
                q = ((uint16_t)v * (255 - (((uint16_t)s * remainder) >> 8))) >> 8;
                t = ((uint16_t)v * (255 - (((uint16_t)s * (255 - remainder)) >> 8))) >> 8;
                
                switch (region)
                {
                    case 0:
                        r = v; g = t; b = p;
                        break;
                    case 1:
                        r = q; g = v; b = p;
                        break;
                    case 2:
                        r = p; g = v; b = t;
                        break;
                    case 3:
                        r = p; g = q; b = v;
                        break;
                    case 4:
                        r = t; g = p; b = v;
                        break;
                    default:
                        r = v; g = p; b = q;
                        break;
                }
            }
            setColor( r | g << 8 | b << 16);
        }

        void nscale8x3( uint8_t* r, uint8_t* g, uint8_t* b,  uint8_t scale){
            *r = ((int)*r * (int)(scale) ) >> 8;
            *g = ((int)*g * (int)(scale) ) >> 8;
            *b = ((int)*b * (int)(scale) ) >> 8;
        }

    
};

class Line{
    protected:
        uint8_t *ptr;
        int width;
        int row;

    public:
        Line(int width, int row, uint8_t *ptr) : 
            ptr(ptr),
            width(width),
            row(row) {  
        }

        Pixel operator[](int i){
            return Pixel(ptr + ((row & 1) ? ((i) + row * width) * 3 : (width - 1 - i + row * width) * 3));
        }
};

class Image{
    protected:
        

    public:
        uint8_t *ptr;
        int width;
        int height;

        Image(int width, int height, uint8_t *ptr):
            ptr(ptr),
            width(width),
            height(height){
        };
    
        Line operator[](int i) const {
            return Line(width, i, ptr);
        }

        Pixel operator()(int x, int y) const {
            return Pixel(ptr + ((x & 1) ? ((y) + x * width) * 3 : (width - 1 - y + x * width) * 3));
        }

        void clean(){
            memset(ptr, 0, width*height*3);
        }
};