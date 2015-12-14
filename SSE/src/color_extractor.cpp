#include "color_extractor.h"

/**
@file color_extractor.cpp
*/

#include "EasyBMP.h"
#include <cmath>
#include <cstring> 
#include <emmintrin.h>
#include <smmintrin.h>
#include "stdint.h"
#include <vector>
#include <tuple>

#define L 0.5
#define PI 3.14159

using std::vector;
using std::tuple;
using std::tie;
using std::make_tuple;

typedef tuple<float, float, float> Vector3;

/**
@function GetAverageColor
calculates an average RGB color in a part of image
@param img is an input image.
@param row is a start row of subimage.
@param col is a start column of subimage.
@param h is a height of subimage.
@param w is a width of subimage.
@return tuple of RGB
*/
Vector3 GetAverageColor(BMP *img, int row, int col, int h, int w) {
    float R = 0, G = 0, B = 0;

    for (int y = row; y < row + h; ++y) {
        for (int x = col; x < col + w; ++x) {
            RGBApixel p = img->GetPixel(x, y);

            R += p.Red;
            G += p.Green;
            B += p.Blue;
        }
    }

    int cell_count = h * w;

    R /= cell_count;
    G /= cell_count;
    B /= cell_count;

    return make_tuple(R, G, B);
}

/**
@function ExtractColor
extracts color features from image and adds them to descryptor
@param img is an input image.
@param descryptor is an output vector to add color features.
*/
void ExtractColor(BMP *img, vector<float> &descryptor) {

    int cell_h = img->TellHeight() / 8;
    int cell_w = img->TellWidth() / 8;

    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {

            float R, G ,B;

            tie(R, G, B) = GetAverageColor(img, y * cell_h, x * cell_w, cell_h, cell_w);

            R /= 255;
            G /= 255;
            B /= 255;

            descryptor.push_back(R);
            descryptor.push_back(G);
            descryptor.push_back(B);
        }
}