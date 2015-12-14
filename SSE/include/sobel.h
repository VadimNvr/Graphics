#ifndef SOBEL
#define SOBEL

/**
@file sobel.h
*/

#include "matrix.h"

typedef unsigned char uchar;
typedef Matrix<uchar> CMatrix;
typedef Matrix<short> SMatrix;

void HorizontalFilterSSE(CMatrix &img, SMatrix &result);
void VerticalFilterSSE(CMatrix &img, SMatrix &result);
void HorizontalFilter(CMatrix &img, SMatrix &result);
void VerticalFilter(CMatrix &img, SMatrix &result);

#endif //SOBEL