#ifndef CALCULATESOBEL
#define CALCULATESOBEL

/**
@file calculatesobel.h
*/

#include "matrix.h"

typedef Matrix<float> FMatrix;
typedef Matrix<short> SMatrix;

void CalculateVectorsSSE(SMatrix &horiz, SMatrix &vert, FMatrix &dir, FMatrix &val);
void CalculateVectors(SMatrix &horiz, SMatrix &vert, FMatrix &dir, FMatrix &val);

#endif // CALCULATESOBEL