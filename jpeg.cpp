/* Jpeg wrapper test program
 */

#include <stdio.h>
#include <unistd.h>
#include <jpeglib.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "jpeg.hpp"



using namespace std;

static void rgb_prescription(const int x, const int y, const int width, const int height, rgb_t &rgb) {
	rgb.r = 255*((float)x/(float)width);
	rgb.g = 255*((float)y/(float)height);
	rgb.b = 0;
}

static inline int diff(const rgb_t &a, const rgb_t &b) {
	return abs(a.r-b.r) + abs(a.g-b.g) + abs(a.b-b.b);
}


int main() {
	const char* filename = "test.jpeg";
	
	// Create and write a jpeg
	Jpeg jpeg(255,255);
	rgb_t rgb;
	
	for (int x=0;x<jpeg.width;x++) {
		for(int y=0;y<jpeg.height;y++) {
			rgb_prescription(x,y, jpeg.width, jpeg.height, rgb);
			jpeg.set(x,y,rgb);
		}
	}
	jpeg.write(filename);
	
	// Read in and check values
	try {
	Jpeg input(filename);
		if(input.width != jpeg.width) throw "Width mismatch";
		if(input.height != jpeg.height) throw "Width mismatch";
		
		
		rgb_t rgb1, rgb2;
		int errs = 0;
		for (int x=0;x<input.width;x++) {
			for(int y=0;y<input.height;y++) {
				rgb1 = input.rgb(x, y);
				rgb2 = jpeg.rgb(x, y);
				
				const int x = diff(rgb1, rgb2);
				if (x > 5) {
					cerr << "Pixel (" << x << "," << y << ") differs " << x << " value points!" << endl;
					errs++;
				}
			}
		}
		
		if(errs > 0) throw "Pixel errors";
	} catch (const char* msg) {
		cerr << "Error: " << msg << endl;
		return EXIT_FAILURE;
	}
	
	::unlink(filename);
	cout << "All good" << endl;
	return EXIT_SUCCESS;
}


