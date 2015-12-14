#include "hog.h"

/**
@file grayscale.cpp
*/

#include "matrix.h"
#include "sobel.h"
#include "calculatesobel.h"
#include <vector>
#include <cmath>

#define SECT_CNT 16
#define NORM_PWR 2
#define PI 3.14159

using std::vector;

typedef unsigned char uchar;
typedef Matrix<uchar> CMatrix;
typedef Matrix<short> SMatrix;
typedef Matrix<float> FMatrix;

/**
@function MakeHist
takes matrixes of values and directions and makes a histogram
@param val is a float input matrix of values.
@param dir is a float input matrix of directions.
@param histogram is an output vector, which stores summary value in each direction.
*/
void MakeHist(FMatrix &val, FMatrix &dir, vector<float> &histogram) {
    int height = val.n_rows;
    int width  = val.n_cols;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = dir(y, x) / (2*PI) * SECT_CNT;

            histogram[idx] += val(y, x);
        }
    }
}

/**
@function Norm
calculates a norm of vector
@param vect is an input vector
@param power is an input
@return double value of vector norm
*/
double Norm(vector<float> &vect, int power) {

    double result = 0;

    for (auto val: vect)
        result += abs(pow(val, power));

    return pow(result, 1.0/power);
}

/**
@function NormalizeVector
performs a vector narmalization
@param vect is an input vector
@param power is an input
*/
void NormalizeVector(vector<float> &vect, int power) {

    double norm = Norm(vect, power);

    for (auto it = vect.begin(); it != vect.end(); ++it){
        if (norm >= 0.0001)
            *it /= norm;
    }
}

/**
@function Concat
concatenates a histogram to descryptor
@param descryptor is an input vector
@param histogram is an input histogram
*/
void Concat(vector<float> &descryptor, float *histogram) {
    for (int i = 0; i < SECT_CNT; ++i)
        descryptor.push_back(histogram[i]);
}

/**
@function HOG
performs a HOG algorithm to input greyscales image 
@param gs is an input greyscaled image
@param descryptor is an output vector to add a set of histograms
@param cell_count is a count of cells to split an image
*/
void HOG(CMatrix &gs, vector<float> &descryptor, int cell_count) {
    SMatrix grad_horiz(gs.n_rows-2, gs.n_cols-2);
    SMatrix grad_vert(gs.n_rows-2, gs.n_cols-2);

    HorizontalFilter(gs, grad_horiz);
    VerticalFilter(gs, grad_vert);

    FMatrix dir_matr(grad_horiz.n_rows, grad_horiz.n_cols), 
            val_matr(grad_horiz.n_rows, grad_horiz.n_cols);

    CalculateVectors(grad_horiz, grad_vert, dir_matr, val_matr);

    int cell_height = grad_horiz.n_rows / cell_count;
    int cell_width  = grad_horiz.n_cols / cell_count;

    vector<float> big_hist;

    for (int cell_y = 0; cell_y < cell_count; ++cell_y) {
        for (int cell_x = 0; cell_x < cell_count; ++cell_x) {

            vector<float> histogram;
            for (int it = 0; it < SECT_CNT; ++it)
                histogram.push_back(0);

            FMatrix dir_cell = dir_matr.submatrix( cell_y * cell_height,
                                                   cell_x * cell_width,
                                                   cell_height,
                                                   cell_width );

            FMatrix val_cell = val_matr.submatrix( cell_y * cell_height,
                                                   cell_x * cell_width,
                                                   cell_height,
                                                   cell_width );

            MakeHist(val_cell, dir_cell, histogram);

            for (auto val: histogram)
                big_hist.push_back(val);
        }
    }

    NormalizeVector(big_hist, NORM_PWR);

    for (auto val: big_hist) 
        descryptor.push_back(val);
}

/**
@function HOG
performs a HOG algorithm to input greyscales image, with use of SSE 
@param gs is an input greyscaled image
@param descryptor is an output vector to add a set of histograms
@param cell_count is a count of cells to split an image
*/
void HOG_SSE(CMatrix &gs, vector<float> &descryptor, int cell_count) {
    SMatrix grad_horiz(gs.n_rows-2, gs.n_cols-2);
    SMatrix grad_vert(gs.n_rows-2, gs.n_cols-2);

    HorizontalFilterSSE(gs, grad_horiz);
    VerticalFilterSSE(gs, grad_vert);

    FMatrix dir_matr(grad_horiz.n_rows, grad_horiz.n_cols), 
            val_matr(grad_horiz.n_rows, grad_horiz.n_cols);

    CalculateVectorsSSE(grad_horiz, grad_vert, dir_matr, val_matr);

    int cell_height = grad_horiz.n_rows / cell_count;
    int cell_width  = grad_horiz.n_cols / cell_count;

    vector<float> big_hist;

    for (int cell_y = 0; cell_y < cell_count; ++cell_y) {
        for (int cell_x = 0; cell_x < cell_count; ++cell_x) {

            vector<float> histogram;
            for (int it = 0; it < SECT_CNT; ++it)
                histogram.push_back(0);

            FMatrix dir_cell = dir_matr.submatrix( cell_y * cell_height,
                                                   cell_x * cell_width,
                                                   cell_height,
                                                   cell_width );

            FMatrix val_cell = val_matr.submatrix( cell_y * cell_height,
                                                   cell_x * cell_width,
                                                   cell_height,
                                                   cell_width );

            MakeHist(val_cell, dir_cell, histogram);

            for (auto val: histogram)
                big_hist.push_back(val);
        }
    }

    NormalizeVector(big_hist, NORM_PWR);

    for (auto val: big_hist) 
        descryptor.push_back(val);
}