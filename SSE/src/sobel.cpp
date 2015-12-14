#include "sobel.h"

/**
@file sobel.cpp
*/

#include <cmath>
#include <emmintrin.h>
#include <smmintrin.h>
#include "stdint.h"

#include "matrix.h"

static const __m128i SOBEL_MULIPLIER_1 = _mm_setr_epi16(1, 2, 1, 2, 1, 2, 1, 2);
static const __m128i SOBEL_MULIPLIER_2 = _mm_setr_epi16(2, 1, 2, 1, 2, 1, 2, 1);

typedef unsigned char uchar;
typedef Matrix<uchar> CMatrix;
typedef Matrix<short> SMatrix;

/**
@function HorizontalFilterSSE
takes horizontal gradient components from image , using SSE
@param img is an input image.
@param result is an output matrix of horizontal gradient components.
*/
void HorizontalFilterSSE(CMatrix &img, SMatrix &result) {
    int width  = img.n_cols;
    int height = img.n_rows;
    __m128i col_l, col_r, mul_l, mul_r, res;

    int y;

    //Matrixes 8x3
    for (y = 0; y + 7 < height; y += 6) {
        for (int step = 0; step < 2; ++step)
        {
            col_r = _mm_setr_epi16(img(y, step), img(y+1, step), img(y+2, step), img(y+3, step),
                                           img(y+4, step), img(y+5, step), img(y+6, step), img(y+7, step));
            for (int x = step; x+2 < width; x += 2) 
            {
                col_l = _mm_loadu_si128(&col_r);

                col_r = _mm_setr_epi16(img(y, x+2), img(y+1, x+2), img(y+2, x+2), img(y+3, x+2),
                                                  img(y+4, x+2), img(y+5, x+2), img(y+6, x+2), img(y+7, x+2)); 

                /* FIRST PART {1, 2, 1, 2, 1, 2, 1, 2} */
                mul_l = _mm_mullo_epi16(col_l, SOBEL_MULIPLIER_1);
                mul_r = _mm_mullo_epi16(col_r, SOBEL_MULIPLIER_1);

                res = _mm_sub_epi16(mul_r, mul_l);

                result(y,   x) = ((int16_t*)&res)[0] + ((int16_t*)&res)[1] + ((int16_t*)&res)[2];
                result(y+2, x) = ((int16_t*)&res)[2] + ((int16_t*)&res)[3] + ((int16_t*)&res)[4];
                result(y+4, x) = ((int16_t*)&res)[4] + ((int16_t*)&res)[5] + ((int16_t*)&res)[6];

                /* SECOND PART {2, 1, 2, 1, 2, 1, 2, 1} */
                mul_l = _mm_mullo_epi16(col_l, SOBEL_MULIPLIER_2);
                mul_r = _mm_mullo_epi16(col_r, SOBEL_MULIPLIER_2);

                res = _mm_sub_epi16(mul_r, mul_l);

                result(y+1, x) = ((int16_t*)&res)[1] + ((int16_t*)&res)[2] + ((int16_t*)&res)[3];
                result(y+3, x) = ((int16_t*)&res)[3] + ((int16_t*)&res)[4] + ((int16_t*)&res)[5];
                result(y+5, x) = ((int16_t*)&res)[5] + ((int16_t*)&res)[6] + ((int16_t*)&res)[7];
            }
        }
    }

    if (y+2 == height)
        return;

    //Borders
    for (int step = 0; step < 2; ++step)
    {
        col_r = _mm_setzero_si128();
        for (int i = 0; y+i < height; ++i) 
            ((int16_t*)&col_r)[i] = img(y+i, step); 

        for (int x = step; x+2 < width; x += 2) 
        {
            col_l = _mm_loadu_si128(&col_r);

            col_r = _mm_setzero_si128();
            for (int i = 0; y+i < height; ++i) 
                ((int16_t*)&col_r)[i] = img(y+i, x+2); 

            /* FIRST PART {1, 2, 1, 2, 1, 2, 1, 2} */
            mul_l = _mm_mullo_epi16(col_l, SOBEL_MULIPLIER_1);
            mul_r = _mm_mullo_epi16(col_r, SOBEL_MULIPLIER_1);

            res = _mm_sub_epi16(mul_r, mul_l);

            for (int i = 0; y+i+2 < height; i += 2)
                result(y+i, x) = ((int16_t*)&res)[i] + ((int16_t*)&res)[i+1] + ((int16_t*)&res)[i+2];

            /* SECOND PART {2, 1, 2, 1, 2, 1, 2, 1} */
            mul_l = _mm_mullo_epi16(col_l, SOBEL_MULIPLIER_2);
            mul_r = _mm_mullo_epi16(col_r, SOBEL_MULIPLIER_2);

            res = _mm_sub_epi16(mul_r, mul_l);

            for (int i = 1; y+i+2 < height; i += 2)
                result(y+i, x) = ((int16_t*)&res)[i] + ((int16_t*)&res)[i+1] + ((int16_t*)&res)[i+2];
        }
    }
}

/**
@function VerticalFilterSSE
takes vertical gradient components from image , using SSE
@param img is an input image.
@param result is an output matrix of vertical gradient components.
*/
void VerticalFilterSSE(CMatrix &img, SMatrix &result) {
    int width  = img.n_cols;
    int height = img.n_rows;
    __m128i row_t, row_b, mul_t, mul_b, res;

    int x;

    //Matrixes 3x8
    for (x = 0; x + 7 < width; x += 6) {
        for (int step = 0; step < 2; ++step)
        {
            row_b = _mm_setr_epi16(img(step, x), img(step, x+1), img(step, x+2), img(step, x+3),
                                           img(step, x+4), img(step, x+5), img(step, x+6), img(step, x+7));

            for (int y = step; y+2 < height; y += 2) 
            {
                row_t = _mm_loadu_si128(&row_b); 

                row_b = _mm_setr_epi16(img(y+2, x), img(y+2, x+1), img(y+2, x+2), img(y+2, x+3),
                                           img(y+2, x+4), img(y+2, x+5), img(y+2, x+6), img(y+2, x+7)); 

                /* FIRST PART {1, 2, 1, 2, 1, 2, 1, 2} */
                mul_t = _mm_mullo_epi16(row_t, SOBEL_MULIPLIER_1);
                mul_b = _mm_mullo_epi16(row_b, SOBEL_MULIPLIER_1);

                res = _mm_sub_epi16(mul_b, mul_t);

                result(y, x  ) = ((int16_t*)&res)[0] + ((int16_t*)&res)[1] + ((int16_t*)&res)[2];
                result(y, x+2) = ((int16_t*)&res)[2] + ((int16_t*)&res)[3] + ((int16_t*)&res)[4];
                result(y, x+4) = ((int16_t*)&res)[4] + ((int16_t*)&res)[5] + ((int16_t*)&res)[6];

                /* SECOND PART {2, 1, 2, 1, 2, 1, 2, 1} */
                mul_t = _mm_mullo_epi16(row_t, SOBEL_MULIPLIER_2);
                mul_b = _mm_mullo_epi16(row_b, SOBEL_MULIPLIER_2);

                res = _mm_sub_epi16(mul_b, mul_t);

                result(y, x+1) = ((int16_t*)&res)[1] + ((int16_t*)&res)[2] + ((int16_t*)&res)[3];
                result(y, x+3) = ((int16_t*)&res)[3] + ((int16_t*)&res)[4] + ((int16_t*)&res)[5];
                result(y, x+5) = ((int16_t*)&res)[5] + ((int16_t*)&res)[6] + ((int16_t*)&res)[7];
            }
        }
    }

    if (x+2 == width)
        return;

    //Borders
    for (int step = 0; step < 2; ++step)
    {
        row_b = _mm_setzero_si128();
        for (int i = 0; x+i < width; ++i) 
            ((int16_t*)&row_b)[i] = img(step, x+i); 

        for (int y = step; y+2 < height; y += 2) 
        {
            row_t = _mm_loadu_si128(&row_b); 

            row_b = _mm_setzero_si128();
            for (int i = 0; x+i < width; ++i) 
                ((int16_t*)&row_b)[i] = img(y+2, x+i); 

            /* FIRST PART {1, 2, 1, 2, 1, 2, 1, 2} */
            mul_b = _mm_mullo_epi16(row_b, SOBEL_MULIPLIER_1);
            mul_t = _mm_mullo_epi16(row_t, SOBEL_MULIPLIER_1);

            res = _mm_sub_epi16(mul_b, mul_t);

            for (int i = 0; x+i+2 < width; i += 2)
                result(y, x+i) = ((int16_t*)&res)[i] + ((int16_t*)&res)[i+1] + ((int16_t*)&res)[i+2];

            /* SECOND PART {2, 1, 2, 1, 2, 1, 2, 1} */
            mul_t = _mm_mullo_epi16(row_t, SOBEL_MULIPLIER_2);
            mul_b = _mm_mullo_epi16(row_b, SOBEL_MULIPLIER_2);

            res = _mm_sub_epi16(mul_b, mul_t);

            for (int i = 1; x+i+2 < width; i += 2)
                result(y, x+i) = ((int16_t*)&res)[i] + ((int16_t*)&res)[i+1] + ((int16_t*)&res)[i+2];
        }
    }
}

/**
@function HorizontalFilter
takes horizontal gradient components from image
@param img is an input image.
@param result is an output matrix of horizontal gradient components.
*/
void HorizontalFilter(CMatrix &img, SMatrix &result) {
    int width  = img.n_cols;
    int height = img.n_rows;

    for (int y = 1; y < height-1; ++y) {
        for (int x = 1; x < width-1; ++x) {
            result(y-1, x-1) = -img(y-1, x-1) -2*img(y, x-1) -img(y+1, x-1) + img(y-1, x+1) +2*img(y, x+1) +img(y+1, x+1);
        }
    }
}

/**
@function VerticalFilter
takes vertical gradient components from image
@param img is an input image.
@param result is an output matrix of vertical gradient components.
*/
void VerticalFilter(CMatrix &img, SMatrix &result) {
    int width  = img.n_cols;
    int height = img.n_rows;

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            result(y-1, x-1) = -img(y-1, x-1) -2*img(y-1, x) - img(y-1, x+1) + img(y+1, x-1) + 2*img(y+1, x) + img(y+1, x+1);
        }
    }
}