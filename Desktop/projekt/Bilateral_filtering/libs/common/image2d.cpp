#include <common/image2d.h>

#include <common/error.h>

#include <stdlib.h>

unsigned int image2d_get_number_of_elements(image2d* i) { 
    return i->width * i->height;
}

unsigned int image2d_get_size(image2d* i) {
    return image2d_get_number_of_elements(i) * i->channels; 
}

void image2d_init(image2d* i, unsigned int w, unsigned int h, unsigned int c) {
    i->width = w;
    i->height = h;
    i->channels = c;
    i->data = (BYTE*) malloc( image2d_get_size(i) * sizeof(BYTE) );
}

void image2d_free(image2d* i) {
    free( i->data );
}

void image2d_reverse_vertically(image2d* i) {
    BYTE* temp = i->data;
    i->data = (BYTE*) malloc( image2d_get_size(i) * sizeof(BYTE) );
    unsigned int x, y, c;
    for (x=0; x<i->width; x++)
        for (y=1; y<=i->height; y++)
            for (c=0; c<i->channels; c++)
                i->data[(x+(y-1)*i->width)*i->channels+c] = 
                    temp[(x+(i->height-y)*i->width)*i->channels+c];
    free(temp);
}

void image2d_load_from_raw(image2d* i, const char* file) {
    FILE * pFile;
    unsigned long lSize;
    size_t result;

    pFile = fopen( file , "rb" );
    if (pFile==NULL) {
        exit_with_error("error opening file: %s", file);
    }
    
    /* obtain file size */
    fseek(pFile , 0 , SEEK_END);
    lSize = ftell(pFile);
    rewind(pFile);

    if (lSize != image2d_get_size(i)) {
        exit_with_error("image and file size does not match, file: %s", file);
    }

    /* copy the file into the buffer */
    result = fread(i->data, 1, lSize, pFile);
    if (result != lSize) {
        exit_with_error("Reading error, file: %s", file); 
    }

    fclose(pFile);
  
    image2d_reverse_vertically(i);
}

void image2d_save_to_raw(image2d* image, const char* filename) {
    image2d_reverse_vertically(image);
    FILE* file = fopen(filename,"wb");
    if (!file) {
        exit_with_error("writing file failed, file: %s", file); 
    }
    fwrite(image->data, image2d_get_size(image), sizeof(BYTE), file);
    fflush(file);
    fclose(file);
}

RGB* image2d_data_to_RGB(image2d* image)
{
	if(image->channels != 3)
	{
		exit_with_error("channel mismatch");
	}
	//Convert from bytes to floats
	int numElements = image2d_get_number_of_elements(image);
	RGB* colors = new RGB[numElements];
	for(int i = 0; i < numElements; i++)
	{
		RGB c;
		c.R = image->data[i*3] / 255.0;
		c.G = image->data[i*3+1] / 255.0;
		c.B = image->data[i*3+2] / 255.0;
		colors[i] = c;
	}
	return colors;
}

void image2d_save_RGB_data(RGB* colors, image2d* image)
{
	if(image->channels != 3)
	{
		exit_with_error("channel mismatch");
	}
	//Convert back to bytes
	int numElements = image2d_get_number_of_elements(image);
	for(int i = 0; i < numElements; i++)
	{
		RGB c = colors[i];
		//Clamp the image
		if(c.R < 0) c.R = 0;
		if(c.G < 0) c.G = 0;
		if(c.B < 0) c.B = 0;
		if(c.R > 1) c.R = 1;
		if(c.G > 1) c.G = 1;
		if(c.B > 1) c.B = 1;

		image->data[i*3] = (BYTE)(c.R * 255);
		image->data[i*3+1] = (BYTE)(c.G * 255);
		image->data[i*3+2] = (BYTE)(c.B * 255);
	}
}