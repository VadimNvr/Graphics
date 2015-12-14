#ifndef PYRAMIDE
#define PYRAMIDE

/**
@file pyramide.h
*/

#include "matrix.h"
#include "hog.h"
#include <vector>

using std::vector;

typedef unsigned char uchar;
typedef Matrix<uchar> CMatrix;

void Pyramide(CMatrix &gs, vector<float> &descryptor);
void PyramideSSE(CMatrix &gs, vector<float> &descryptor);

#endif //PYRAMIDE