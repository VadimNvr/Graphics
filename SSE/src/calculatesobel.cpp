#include "calculatesobel.h"

/**
@file calculatesobel.cpp
*/

#include <cmath>
#include <cstring>
#include <emmintrin.h>
#include <smmintrin.h>
#include "stdint.h"

#include "matrix.h"

#define PI 3.14159

typedef Matrix<float> FMatrix;
typedef Matrix<short> SMatrix;

/**
@function CalculateVectorSSE
converts horizontal and vertical gradient components matrixes to matrixes of gradient values and directions, using SSE
@param horiz is an input matrix of horizontal gradient components.
@param vert is an input matrix of vertical gradient components.
@param dir is an output matrix of gradient directions.
@param val is an output matrix of gradient values.
*/
void CalculateVectorsSSE(SMatrix &horiz, SMatrix &vert, FMatrix &dir, FMatrix &val) {
    int height = horiz.n_rows;
    int width  = horiz.n_cols;

    __m128i v;
    __m128i h;
    __m128 res;

    int x;

    for (int y = 0; y < height; ++y) {
        for (x = 0; x+3 < width; x+=4) {
            v = _mm_setr_epi32(vert(y, x), vert(y, x+1), vert(y, x+2), vert(y, x+3));
            h = _mm_setr_epi32(horiz(y, x), horiz(y, x+1), horiz(y, x+2), horiz(y, x+3));
        
            for (int i = 0; i < 4; ++i) 
            {
                float tmp = atan2(((int*)&v)[i], ((int*)&h)[i]);
                dir(y, x+i) = (tmp > 0)? tmp: 2*PI + tmp;
            }

            res = _mm_sqrt_ps(_mm_cvtepi32_ps(_mm_add_epi32(_mm_mullo_epi32(v, v), _mm_mullo_epi32(h, h))));

            memcpy(&(val(y, x)), &res, 16);
        }

        for (; x < width; ++x)
        {
            int v2 = vert(y, x);
            int h2 = horiz(y, x);

            float tmp = atan2(v2, h2);
            dir(y, x) = (tmp > 0)? tmp: 2*PI+tmp;

            val(y, x) = sqrt( h2*h2 + v2*v2 );
        }
    }
}

/**
@function CalculateVectorSSE
converts horizontal and vertical gradient components matrixes to matrixes of gradient values and directions
@param horiz is an input matrix of horizontal gradient components.
@param vert is an input matrix of vertical gradient components.
@param dir is an output matrix of gradient directions.
@param val is an output matrix of gradient values.
*/
void CalculateVectors(SMatrix &horiz, SMatrix &vert, FMatrix &dir, FMatrix &val) {
    int height = horiz.n_rows;
    int width  = horiz.n_cols;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int v = vert(y, x);
            int h = horiz(y, x);

            float tmp = atan2(v, h);
            dir(y, x) = (tmp > 0)? tmp: 2*PI+tmp;

            val(y, x) = sqrt( h*h + v*v );
        }
    }
}