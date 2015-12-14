#ifndef KERNEL
#define KERNEL

/**
@file kernel.h
*/

#include <vector>

using std::vector;

double k(double);
void X2KernelSSE(vector<float> &);
void X2Kernel(vector<float> &);

#endif //KERNEL