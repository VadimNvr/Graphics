#include "greyscale.h"

/**
@file grayscale.cpp
*/

#include <cmath>
#include <emmintrin.h>
#include <smmintrin.h>
#include "stdint.h"

#include "EasyBMP.h"
#include "matrix.h"

#define RED_WEIGHT    0.2125f
#define GREEN_WEIGHT  0.7154f
#define BLUE_WEIGHT   0.0721f

/// Weight of red channel stored as 4 equal floating point values
static const __m128 CONST_RED = _mm_set1_ps(RED_WEIGHT);

/// Weight of green channel stored as 4 equal floating point values
static const __m128 CONST_GREEN = _mm_set1_ps(GREEN_WEIGHT);

/// Weight of blue channel stored as 4 equal floating point values
static const __m128 CONST_BLUE = _mm_set1_ps(BLUE_WEIGHT);

typedef unsigned char uchar;
typedef Matrix<uchar> CMatrix;

/**
@function Get4Pixels16Bit
reads four consecutive pixels of the specified row started from given column and writes they to the
two registers out_BG and out_RA. Uses 16 bit per channel
@param in_img is a input image
@param in_row_idx is an index of a row to read pixels
@param in_col_idx ia an index of a column with a first pixel
@param out_BG is a pointer to a 128bit register to store blue and green channels for the pixels four
consecutive pixels in format BBBB GGGG. Order of pixels is [0, 1, 2, 3]
@param out_RA is a pointer to a 128bit register to store red and alpha channels for the pixels four
consecutive pixels in format RRRR AAAA. Order of pixels is [0, 1, 2, 3]
*/
void Get4Pixels16Bit(BMP &in_img, int in_row_idx, int in_col_idx,
                            __m128i *out_BG, __m128i *out_RA)
{
  // read four consecutive pixels
  RGBApixel pixel0 = in_img.GetPixel(in_col_idx, in_row_idx);
  RGBApixel pixel1 = in_img.GetPixel(in_col_idx + 1, in_row_idx);
  RGBApixel pixel2 = in_img.GetPixel(in_col_idx + 2, in_row_idx);
  RGBApixel pixel3 = in_img.GetPixel(in_col_idx + 3, in_row_idx);

  // write two pixel0 and pixel2 to the p02 and other to the p13
  __m128i p02 = _mm_setr_epi32(*(int*)&pixel0, *(int*)&pixel2, 0, 0);
  __m128i p13 = _mm_setr_epi32(*(int*)&pixel1, *(int*)&pixel3, 0, 0);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  * convert BGRA BGRA BGRA BGRA
  * to BBBB GGGG RRRR AAAA
  * order of pixels corresponds to the digits in the name of variables
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  // BGRA BGRA 0000 0000 + BGRA BGRA 0000 0000 -> BBGG RRAA BBGG RRAA
  __m128i p0123 = _mm_unpacklo_epi8(p02, p13);
  // extract BBGG RRAA 0000 0000 of pixel2 and pixel3
  __m128i p23xx = _mm_unpackhi_epi64(p0123, _mm_setzero_si128());
  // BBGG RRAA XXXX XXXX + BBGG RRAA 0000 0000 -> BBBB GGGG RRRR AAAA
  // X denotes unused characters
  __m128i p0123_8bit = _mm_unpacklo_epi16(p0123, p23xx);

  // extend to 16bit 
  *out_BG = _mm_unpacklo_epi8(p0123_8bit, _mm_setzero_si128());
  *out_RA = _mm_unpackhi_epi8(p0123_8bit, _mm_setzero_si128());
}

/**
@function toGrayScaleSSE
utilizes SSE to realize precise approach to convert RGBA image to grayscale.
@param in_input is an input image.
@param out_mat is an output image. Each pixel is represented by a single unsigned char value.
*/
void toGrayScaleSSE(BMP &in_input, CMatrix &out_mat)
{
  // number of elements to process at a time
  const int block_size = 4;
  // number of elements that will not be processed block-wise
  const int left_cols = out_mat.n_cols % block_size;
  // number of elements that will be processed block-wise
  const int block_cols = (int)out_mat.n_cols - left_cols;

  __m128i BG, RA, pB, pG, pR, color, zero = _mm_setzero_si128();
  __m128 red, green, blue;
 
  for (int row_idx = 0; row_idx < out_mat.n_rows; ++row_idx)
  {
    // process block_size elements at a time.
    for (int col_idx = 0; col_idx < block_cols; col_idx += block_size)
    {
      // read four pixels
      Get4Pixels16Bit(in_input, row_idx, col_idx, &BG, &RA);

      // extend to 32bit 
      pB = _mm_unpacklo_epi8(BG, zero);  //000B 000B 000B 000B
      pG = _mm_unpackhi_epi8(BG, zero);  //000G 000G 000G 000G
      pR = _mm_unpacklo_epi8(RA, zero);  //000R 000R 000R 000R

      // convert to float
      blue = _mm_cvtepi32_ps(pB);   //B B B B
      green = _mm_cvtepi32_ps(pG);  //G G G G
      red = _mm_cvtepi32_ps(pR);    //R R R R

      // multiply channels by weights
      red = _mm_mul_ps(red, CONST_RED);
      green = _mm_mul_ps(green, CONST_GREEN);
      blue = _mm_mul_ps(blue, CONST_BLUE);

      // sum up channels
      red = _mm_add_ps(red, green);
      red = _mm_add_ps(red, blue); //С С С С

      // convert to 32bit integer
      color = _mm_cvttps_epi32(red);
      // convert to 16bit
      color = _mm_packs_epi32(color, zero);
      // convert to 8bit
      color = _mm_packus_epi16(color, zero); // 000С 000С 000С 000С

      // write results to the out_mat
      *((int32_t*)(&out_mat(row_idx, col_idx))) = _mm_cvtsi128_si32(color);
      //for (int i = 0; i < 4)
      //  out_mat(row_idx, col_idx+i) = 
    }
    // process left elements in the row
    for (int col_idx = block_cols; col_idx < (int)out_mat.n_cols; ++col_idx)
    {
      RGBApixel pixel = in_input.GetPixel(col_idx, row_idx);
      out_mat(row_idx, col_idx) = (uchar)(RED_WEIGHT*pixel.Red + GREEN_WEIGHT*pixel.Green + BLUE_WEIGHT*pixel.Blue);     
    }
  }
}

/**
@function toGrayScale
realizes naive approach to convert RGBA image to grayscale
@param in_input is an input image.
@param out_mat is an output image. Each pixel is represented by a single unsigned char value.
*/
void toGrayScale(BMP &in_input, CMatrix &out_mat) 
{
    int width = out_mat.n_cols;
    int height = out_mat.n_rows;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            RGBApixel *pixel = in_input(x, y);
            out_mat(y, x) = (uchar)(RED_WEIGHT*pixel->Red + GREEN_WEIGHT*pixel->Green + BLUE_WEIGHT*pixel->Blue);
        }
    }
}