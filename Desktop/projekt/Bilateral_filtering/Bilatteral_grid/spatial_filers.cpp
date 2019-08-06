#include "spatial_filters.h"
#include "filtering.h"

inline
RGB operator +(RGB a, RGB b) {
	RGB c;
	c.R = a.R + b.R;
	c.G = a.G + b.G;
	c.B = a.B + b.B;
	return c;
}

inline
RGB operator *(RGB a, RGB b) {
	RGB c;
	c.R = a.R * b.R;
	c.G = a.G * b.G;
	c.B = a.B * b.B;
	return c;
}

inline
RGB operator /(RGB a, RGB b) {
	RGB c;
	c.R = a.R / b.R;
	c.G = a.G / b.G;
	c.B = a.B / b.B;
	return c;
}

inline
RGB makeColor(float R,float G, float B) {
	RGB c;
	c.R = R;
	c.G = G;
	c.B = B;
	return c;
}

void CPUbilateralFiltering(RGB* data, int width, int height,int radius, float sigma_spatial, float sigma_range)
{
	int numElements = width*height;
	RGB* res_data = new RGB[numElements];
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			int array_idx = y * width + x;
			RGB currentColor = data[array_idx]; //idx

			RGB res = makeColor(0.0f,0.0f,0.0f);
			RGB normalization = makeColor(0.0f,0.0f,0.0f);


			for(int i = -radius; i <= radius; i++) {
				for(int j = -radius; j <= radius; j++) {
					int x_sample = x+i;
					int y_sample = y+j;

					//mirror edges
					if( (x_sample < 0) || (x_sample >= width ) ) {
						x_sample = x-i;
					}
			
					if( (y_sample < 0) || (y_sample >= height) ) {
						y_sample = y-j;
					}

					RGB tmpColor = data[y_sample * width + x_sample];

					float gauss_spatial = gaussian2d(i,j,sigma_spatial); //gaussian1d(i,sigma_spatial)*gaussian1d(j,sigma_spatial);//
					RGB gauss_range;
					gauss_range.R = gaussian1d(currentColor.R - tmpColor.R, sigma_range);
					gauss_range.G = gaussian1d(currentColor.G - tmpColor.G, sigma_range);
					gauss_range.B = gaussian1d(currentColor.B - tmpColor.B, sigma_range);
			
					RGB weight;
					weight.R = gauss_spatial * gauss_range.R;
					weight.G = gauss_spatial * gauss_range.G;
					weight.B = gauss_spatial * gauss_range.B;

					normalization = normalization + weight;

					res = res + (tmpColor * weight);

				}
			}
	
			res_data[array_idx] = res / normalization;
		}
	}

	for(int i = 0; i < numElements; i++)
	{
		data[i] = res_data[i];
	}
	delete[] res_data;

}