#include "pyramide.h"

/**
@file pyramide.cpp
*/

#include "matrix.h"
#include "hog.h"
#include <vector>

#define CELL_PYRAMID_CNT 3

using std::vector;

typedef unsigned char uchar;
typedef Matrix<uchar> CMatrix;

/**
@function Pyramide
applies a pyramide of descryptors algorithm to image
@param gs is an input greyscaled image.
@param descryptor is an output vector to add features.
*/
void Pyramide(CMatrix &gs, vector<float> &descryptor) {
    int cell_h = gs.n_rows / 2;
    int cell_w = gs.n_cols / 2;

    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            CMatrix cell = gs.submatrix( y * cell_h,
                                         x * cell_w,
                                         cell_h,
                                         cell_w );

            HOG(cell, descryptor, CELL_PYRAMID_CNT);
        }
    }
}

/**
@function PyramideSSE
applies a pyramide of descryptors algorithm to image, using SSE
@param gs is an input greyscaled image.
@param descryptor is an output vector to add features.
*/
void PyramideSSE(CMatrix &gs, vector<float> &descryptor) {
    int cell_h = gs.n_rows / 2;
    int cell_w = gs.n_cols / 2;

    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            CMatrix cell = gs.submatrix( y * cell_h,
                                         x * cell_w,
                                         cell_h,
                                         cell_w );

            HOG_SSE(cell, descryptor, CELL_PYRAMID_CNT);
        }
    }
}