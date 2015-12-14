#ifndef GREYSCALE
#define GREYSCALE

/**
@file greyscale.h
*/

#include "EasyBMP.h"
#include "matrix.h"

typedef unsigned char uchar;
typedef Matrix<uchar> CMatrix;
typedef Matrix<short> SMatrix;

void Get4Pixels16Bit(BMP &, int, int, __m128i *, __m128i *);
void toGrayScaleSSE(BMP &, CMatrix &);
void toGrayScale(BMP &, CMatrix &) ;

#endif //GREYSCALE