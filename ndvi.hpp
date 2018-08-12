/** 2018, Felix Niederwanger
  * This is my own implementation of the NDVI analysis according to https://publiclab.org/wiki/ndvi
  */

#ifndef _INFRAGRAM_NDVI_HPP_
#define _INFRAGRAM_NDVI_HPP_

#include <math.h>
#include "jpeg.hpp"

float min(float* array, size_t size) {
	if(size == 0) return 0.0F;
	float f_min = array[0];
	for(size_t i=1;i<size;i++) 
		f_min = fminf(f_min, array[i]);
	return f_min;
}
float max(float* array, size_t size) {
	if(size == 0) return 0.0F;
	float f_max = array[0];
	for(size_t i=1;i<size;i++) 
		f_max = fmaxf(f_max, array[i]);
	return f_max;
}

static rgb_t colormap(const float x, const float fmin, const float fmax) {
	const float y = (x-fmin)/(fmax-fmin);
	rgb_t rgb;
	
	rgb.r = (1.0-y)*255;
	rgb.g = y*255;
	rgb.b = 0;
	
	return rgb;
}

/** Create ndvi jpeg out of the given jpeg */
static Jpeg ndvi(const Jpeg &jpeg) {
	
	// Processing image using NDVI analysis
	// Infragram: red channel = infrared, blue channel = visible
	// NDVI = (IR-RGB)/(IR+RGB) where IR is the near IR and RGB is the visible light
	// So for each pixel we are doing (R-B)/(R+B)
	const int width = jpeg.width;
	const int height= jpeg.height;
	const size_t size = width*height;
	
	Jpeg dest(width, height);
	float *buffer = new float[size];
	rgb_t rgb;
	
	int i=0;
	for(int x=0;x<width;x++) {
		for(int y=0;y<height;y++) {
			rgb = jpeg.rgb(x,y);
			
			const unsigned char ir = rgb.r;
			const unsigned char vis = rgb.b;
			buffer[i++] = (float)(ir-vis)/(float)(ir+vis);
		}
	}
	
	
	
	// Apply colormap
	const float fmin = min(buffer, size);
	const float fmax = max(buffer, size);
	i = 0;
	for(int x=0;x<width;x++) {
		for(int y=0;y<height;y++) {
			rgb = colormap(buffer[i++], fmin, fmax);
			dest.set(x,y, rgb);
		}
	}
	
	delete[] buffer;
	return dest;
	
}




#endif
