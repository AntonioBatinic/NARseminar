#include "filtering.h"

#include <device_launch_parameters.h>
#include <cuda_runtime.h>

#include <cuda/cuda_utilities.h>
#include <cuda/uint_util.hcu>
#include <cuda/float_util.hcu>

#include <common/error.h>

#include <math.h>

#define PI 3.14159265

//Textures
texture<float, 1> tex;
texture<float, 1> tex_red;
texture<float, 1> tex_blue;
texture<float, 1> tex_green;


__host__ __device__
float gaussian1d(float x, float sigma)
{
	float variance = pow(sigma,2);
	float exponent = -pow(x,2)/(2*variance);
	return expf(exponent) / sqrt(2 * PI * variance);
}

inline __device__
float gaussian1d_gpu(float x, float sigma)
{
	float variance = __powf(sigma,2);
	float power = pow(x,2);
	float exponent = -power/(2*variance);
	return __expf(exponent) / sqrt(2 * PI * variance);
}

inline __device__
float gaussian1d_gpu_reg(float x, float variance, float sqrt_pi_variance)
{
	float gaussian1d = -(x*x)/(2*variance);
	gaussian1d = __expf(gaussian1d);
	gaussian1d /= sqrt_pi_variance;
	return gaussian1d;
}

__host__ __device__
float gaussian2d(float x, float y, float sigma)
{
	float variance = pow(sigma,2);
	float exponent = -(pow(x,2) + pow(y,2))/(2*variance);
	return expf(exponent) / (2 * PI * variance);
}


float* generateGaussianKernel(int radius, float sigma)
{
	int area = (2*radius+1)*(2*radius+1);
	float* res = new float[area];

	for(int x = -radius; x <= radius; x++)
		for(int y = -radius; y <= radius; y++)
		{
			
			int position = (x+radius)*(radius*2+1) + y+radius; 
			res[position] = gaussian2d(x,y,sigma);
		}
	return res;
}



__global__
void bilateralFilterGPU_v(float* output, uint2 dims, int radius, float* kernel, float variance, float sqrt_sigma)
{
	const unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;

	uint2 pos = idx_to_co(idx,dims);

	if(pos.x >= dims.x || pos.y >= dims.y) return;

	float currentColor = tex1Dfetch(tex, idx);

	float res = 0.0f;
	float normalization = 0.0f;

	for(int i = -radius; i <= radius; i++) {
		for(int j = -radius; j <= radius; j++) {
			
			int x_sample = pos.x+i;
			int y_sample = pos.y+j;

			
			if( x_sample < 0) x_sample = -x_sample;
			if( y_sample < 0) y_sample = -y_sample;
			if( x_sample > dims.x - 1) x_sample = dims.x - 1 - i;
			if( y_sample > dims.y - 1) y_sample = dims.y - 1 - j;

			float tmpColor = tex1Dfetch(tex, y_sample*dims.x + x_sample);
			
			float gaussian1d = -((currentColor - tmpColor)*(currentColor - tmpColor))/(2*variance);
			gaussian1d = __expf(gaussian1d);
			gaussian1d /= sqrt_sigma;
			float gauss_spatial = kernel[co_to_idx(make_uint2(i+radius,j+radius),make_uint2(radius*2+1,radius*2+1))];
			
			float weight = gauss_spatial * gaussian1d;

			normalization = normalization + weight;
			res = res + (tmpColor * weight);

		}
	}
	
	output[idx] = res / normalization;
}


/*
The gaussian kernel for the spatial domain is precalculated before the launch
The gaussian for the range is using the hardware implementations
The input data is cached through a 3 x 1D textures, output is 3 different areas, for r g and b individually
*/
void bilateralFiltering_v(RGB* data, int width, int height,int radius, float sigma_spatial, float sigma_range)
{
	unsigned int numElements = width * height;

    //Input data
    float* d_red;
	float* d_blue;
	float* d_green;
    cudaMalloc( (void**) &d_red, numElements*sizeof(float));
	cudaMalloc( (void**) &d_blue, numElements*sizeof(float));
	cudaMalloc( (void**) &d_green, numElements*sizeof(float));

	//Output data
    float* d_red_out;
	float* d_blue_out;
	float* d_green_out;
    cudaMalloc( (void**) &d_red_out, numElements*sizeof(float));
	cudaMalloc( (void**) &d_blue_out, numElements*sizeof(float));
	cudaMalloc( (void**) &d_green_out, numElements*sizeof(float));

	float* red = new float[numElements];
	float* green = new float[numElements];
	float* blue = new float[numElements];

	for(int i = 0; i < numElements; i++)
	{
		red[i] = data[i].R;
		green[i] = data[i].G;
		blue[i] = data[i].B;
	}

	//Copy image to device
    cudaMemcpy(	d_red, red, numElements*sizeof(float), cudaMemcpyHostToDevice );

	//Set up kernel
	float* kernel = generateGaussianKernel(radius,sigma_spatial);
	float* d_Kernel;
	cudaMalloc( (void**) &d_Kernel, (2*radius+1)*(2*radius+1) * sizeof(float));
	cudaMemcpy( d_Kernel, kernel, (2*radius+1)*(2*radius+1)* sizeof(float), cudaMemcpyHostToDevice);

    // setup dimensions of grid/blocks.
    dim3 blockDim(192,1,1);
    dim3 gridDim((unsigned int) ceil((double)(numElements/blockDim.x)), 1, 1 );

	const cudaChannelFormatDesc desc = cudaCreateChannelDesc<float>();

	cudaBindTexture(0, &tex, d_red, &desc, numElements*sizeof(float));

    // invoke kernel
	bilateralFilterGPU_v<<< gridDim, blockDim >>>
		( d_red_out, make_uint2(width,height), radius, d_Kernel, sigma_range*sigma_range,sqrt(2*PI*sigma_range*sigma_range)); //sqrt(2 * PI * variance)

	cudaMemcpy( d_green, green, numElements*sizeof(float), cudaMemcpyHostToDevice );
	
	//Wait before starting the next
	cudaDeviceSynchronize();
	
	cudaUnbindTexture(tex);
	cudaBindTexture(0, &tex, d_green, &desc, numElements*sizeof(float));

    // invoke kernel
	bilateralFilterGPU_v<<< gridDim, blockDim >>>
		( d_green_out, make_uint2(width,height), radius, d_Kernel, sigma_range*sigma_range,sqrt(2*PI*sigma_range*sigma_range));

	cudaMemcpy( d_blue, blue, numElements*sizeof(float), cudaMemcpyHostToDevice );
	//Wait before starting the next
	cudaDeviceSynchronize();
	
	cudaUnbindTexture(tex);
	cudaBindTexture(0, &tex, d_blue, &desc, numElements*sizeof(float));

    // invoke kernel
	bilateralFilterGPU_v<<< gridDim, blockDim >>>
		( d_blue_out, make_uint2(width,height), radius, d_Kernel, sigma_range*sigma_range,sqrt(2*PI*sigma_range*sigma_range));

	cudaDeviceSynchronize(); //sync before freeing
	cudaUnbindTexture(tex);

    // copy data to host
    cudaMemcpy(	red, d_red_out, numElements*sizeof(float), cudaMemcpyDeviceToHost );
	cudaMemcpy(	green, d_green_out, numElements*sizeof(float), cudaMemcpyDeviceToHost );
	cudaMemcpy(	blue, d_blue_out, numElements*sizeof(float), cudaMemcpyDeviceToHost );

	cudaFree(d_red);
	cudaFree(d_green);
	cudaFree(d_blue);
	cudaFree(d_red_out);
	cudaFree(d_green_out);
	cudaFree(d_blue_out);

	for(int i = 0; i < numElements; i++)
	{
		data[i].R = red[i];
		data[i].G = green[i];
		data[i].B = blue[i];
	}

	//Error handling
    CHECK_FOR_CUDA_ERROR();
}


