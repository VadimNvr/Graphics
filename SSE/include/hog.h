#ifndef HOG_H_
#define HOG_H_

/**
@file hog.h
*/

#include "matrix.h"
#include <vector>

using std::vector;

typedef unsigned char uchar;
typedef Matrix<uchar> CMatrix;
typedef Matrix<float> FMatrix;

void MakeHist(FMatrix &val, FMatrix &dir, vector<float> &histogram);
double Norm(vector<float> &vect, int power);
void NormalizeVector(vector<float> &vect, int power);
void Concat(vector<float> &descryptor, float *histogram);
void HOG_SSE(CMatrix &gs, vector<float> &descryptor, int cell_cnt);
void HOG(CMatrix &gs, vector<float> &descryptor, int cell_cnt);

#endif //HOG_H_