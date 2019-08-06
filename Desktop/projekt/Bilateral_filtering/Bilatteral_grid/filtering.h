#ifndef FILTERING
#define FILTERING

#include <common/image2d.h>

#ifdef __cplusplus
extern "C" {
#endif

float gaussian1d(float x, float sigma);
float gaussian2d(float x, float y, float sigma);


/*
The gaussian kernel for the spatial domain is precalculated before the launch
The gaussian for the range is using the hardware implementations
The input data is cached through a 3 x 1D textures, output is 3 different areas, for r g and b individually
*/
void bilateralFiltering_v(RGB* data, int width, int height,int radius, float sigma_spatial, float sigma_range);




#ifdef __cplusplus
}
#endif

#endif /* _BINARY_THRESHOLD_ */
