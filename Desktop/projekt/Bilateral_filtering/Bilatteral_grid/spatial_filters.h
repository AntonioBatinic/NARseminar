#pragma once

#include "common\image2d.h"

float* generateBoxFilter(int radius);
//float* generateGaussianKernel(int radius, float sigma);

//A is an addition to the center element
//A = 0 : simple sharpening
//A = 1 : original image + sharpening
//A > 1 : original image dominates, gets brigther
float* generateSharpeningFilter(int radius, unsigned int A);

void CPUbilateralFiltering(RGB* data, int width, int height,int radius, float sigma_spatial, float sigma_range);