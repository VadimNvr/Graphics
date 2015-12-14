#include "kernel.h"

/**
@file color_extractor.cpp
*/

#include <cmath>
#include <cstring> 
#include <emmintrin.h>
#include <smmintrin.h>
#include "stdint.h"
#include <vector>

#define L 0.5
#define PI 3.14159

using std::vector;

/**
@function k
calculates a sech
@param lambda is an input value to sech.
@return sech of lambda.
*/
double k(double lambda) {
    return (2*exp(PI*lambda)) / (exp(2*PI*lambda) + 1);
}

/**
@function X2KernelSSE
converts features to X2 kernel, using SSE
@param descryptor is a vector of features to convert.
*/
void X2KernelSSE(vector<float> &descryptor) {
    vector<float> x2_descryptor;

    uint cur_size = 0;
    __m128 vect;
    __m128 sqrt_vect;
    __m128 log_vect;
    __m128 in_cos_vect;
    __m128 cos_vect;
    __m128 sin_vect;
    __m128 lambda_vect = _mm_setr_ps(-2*L, -L, L, 2*L);
    __m128 k_vect = _mm_setr_ps(k(-2*L), k(-L), k(L), k(2*L));

    for (auto elem: descryptor)
    {
        x2_descryptor.resize(cur_size+10);

        if (elem < 0.0001)
        {
            memset(x2_descryptor.data()+cur_size, 0, 40);
            cur_size += 10;
            continue;
        }

        vect = _mm_set1_ps(elem);

        //log(vect)
        log_vect = _mm_set1_ps(log(elem));

        //lambda*log(vect)
        in_cos_vect = _mm_mul_ps(lambda_vect, log_vect);

        //cos(lambda*log(vect))
        cos_vect = _mm_setr_ps(
            cos(in_cos_vect[0]), 
            cos(in_cos_vect[1]), 
            cos(in_cos_vect[2]), 
            cos(in_cos_vect[3])
        );

        //-sin(lambda*log(vect))
        sin_vect = _mm_setr_ps(
            -sin(in_cos_vect[0]), 
            -sin(in_cos_vect[1]), 
            -sin(in_cos_vect[2]), 
            -sin(in_cos_vect[3])
        );

        //sqrt(vect*k(lambda))
        sqrt_vect = _mm_sqrt_ps(_mm_mul_ps(vect, k_vect));

        //RE
        cos_vect = _mm_mul_ps(cos_vect, sqrt_vect);
        
        //IM
        sin_vect = _mm_mul_ps(sin_vect, sqrt_vect);

        //RE[0], IM[0], RE[1], IM[1]
        vect = _mm_unpacklo_ps(cos_vect, sin_vect);

        memcpy(x2_descryptor.data() + cur_size, &vect, 16);

        x2_descryptor[cur_size+4] = sqrt(elem);
        x2_descryptor[cur_size+5] = 0;

        //RE[2], IM[2], RE[3], IM[3]
        vect = _mm_unpackhi_ps(cos_vect, sin_vect); 

        memcpy(x2_descryptor.data() + cur_size + 6, &vect, 16);

        cur_size += 10;
    }

    descryptor = x2_descryptor;
}

/**
@function X2Kernel
converts features to X2 kernel
@param descryptor is a vector of features to convert.
*/
void X2Kernel(vector<float> &descryptor) {
    vector<float> x2_descryptor;

    for (auto vect: descryptor) {

        float Re = 0, Im = 0;

        for (int i = -2; i <= 2; ++i) {
            if (vect >= 0.0001) {
                
                float lambda = i*L;
                
                Re = cos(lambda*log(vect))*sqrt(vect * k(lambda));
                Im = -sin(lambda*log(vect))*sqrt(vect * k(lambda));
            }

            x2_descryptor.push_back(Re);
            x2_descryptor.push_back(Im);
        }
    }

    descryptor = x2_descryptor;
}