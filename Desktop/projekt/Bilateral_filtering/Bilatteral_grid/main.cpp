#include <stdlib.h>

#include "filtering.h"
#include "spatial_filters.h"
#include <common/error.h>
#include <common/image2d.h>

#include <iostream>

#include "hr_time.h"

unsigned int radius;
float sigma_spatial;
float sigma_range;

double timeBilateralFuction(image2d* img, const char* input_file, int iterations,void (*filter)(RGB* data, int width, int height,int radius, float sigma_spatial, float sigma_range))
{
	double result = 0.0;
	for(int i = 0; i < iterations; i++)
	{
		image2d_load_from_raw(img, input_file);
		RGB* data = image2d_data_to_RGB(img);
		CStopWatch watch;

		watch.startTimer();
		filter(data,img->width,img->height,radius,sigma_spatial,sigma_range);
		watch.stopTimer();

		image2d_save_RGB_data(data,img);
		delete[] data;
		result +=  watch.getElapsedTime();
	}
	return result / iterations;
}

int main(int argc, char** argv) {
    if (argc != 9)
        exit_with_error("invalid parameters, use: <input file> <width> <height> <channels> <neighborhood radius> <spatial sigma> <range sigma> <iterations>\n");

    const char* input_file = argv[1];
    const unsigned int width = atoi(argv[2]);
    const unsigned int height = atoi(argv[3]);
	const unsigned int channels = atoi(argv[4]);
	
	radius = atoi(argv[5]);
	sigma_spatial = (float)atof(argv[6]);
	sigma_range = (float)atof(argv[7]);

	int iterations = atoi(argv[8]);

    image2d* image = (image2d*)malloc(sizeof(image2d));
    image2d_init(image, width, height, channels);

	double result = 0.0;

	std::cout << std::endl << "Filtering on the image:" << input_file << " of size: " << width << "x" << height << " Radius: " << radius << std::endl;
	
	//Golden reference, CPU filtering
	result = timeBilateralFuction(image,input_file,1,CPUbilateralFiltering);
	std::cout << "CPU\t" << result << std::endl;
	image2d_save_to_raw(image, "cpu.raw");
	
	
	//GPU  Filtering
	result = timeBilateralFuction(image,input_file,iterations,bilateralFiltering_v);
	std::cout << "GPU\t" << result << std::endl;
	image2d_save_to_raw(image, "gpu_v.raw");

	

    image2d_free(image);
    return EXIT_SUCCESS;
}
