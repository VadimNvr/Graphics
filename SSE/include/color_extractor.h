#ifndef COLOR_EXTRACTOR
#define COLOR_EXTRACTOR

/**
@file color_extractor.h
*/

#include <vector>
#include "EasyBMP.h"
#include <emmintrin.h>
#include <smmintrin.h>

using std::vector;
using std::tuple;

typedef tuple<float, float, float> Vector3;

__m128 GetAverageColorSSE(BMP *, int row, int col, int h, int w);
Vector3 GetAverageColor(BMP *img, int row, int col, int h, int w);
void ExtractColorSSE(BMP *img, vector<float> &descryptor);
void ExtractColor(BMP *img, vector<float> &descryptor);

#endif //COLOR_EXTRACTOR