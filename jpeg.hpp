/* 2018, Felix Niederwanger
 * This is my own libjpeg wrapper
 * Do not mind the crappy quality
 */

#include <stdio.h>
#include <unistd.h>
#include <jpeglib.h>
#include <string.h>
#include <errno.h>

#include <iostream>


struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} typedef rgb_t;

class Jpeg {
public:
	int width = 0;
	int height = 0;
	int depth = 0;
private:
	unsigned char *bmap = NULL;

public:
	Jpeg(const Jpeg &src) {
		this->width = src.width;
		this->height = src.height;
		this->depth = src.depth;
		
		const size_t size = 3*width*height;
		this->bmap = new unsigned char[size];
		memcpy(this->bmap, src.bmap, sizeof(unsigned char)*size);
	}
	Jpeg(const int width, const int height) {
		this->width = width;
		this->height = height;
		this->depth = 32;
		
		const size_t size = 3*width*height;
		this->bmap = new unsigned char[size];
		bzero(this->bmap, sizeof(unsigned char)*size);
	
	}
	
	Jpeg(const char *filename)  {
	  unsigned char r, g, b;
	  struct jpeg_decompress_struct cinfo;
	  struct jpeg_error_mgr jerr;

	  FILE * infile;        /* source file */
	  JSAMPARRAY pJpegBuffer;       /* Output row buffer */
	  int row_stride;       /* physical row width in output buffer */
	  if ((infile = fopen(filename, "rb")) == NULL) {
	  	throw "Error opening jpeg file";
	  }
	  cinfo.err = jpeg_std_error(&jerr);
	  jpeg_create_decompress(&cinfo);
	  jpeg_stdio_src(&cinfo, infile);
	  (void) jpeg_read_header(&cinfo, TRUE);
	  (void) jpeg_start_decompress(&cinfo);
	  width = cinfo.output_width;
	  height = cinfo.output_height;

	  unsigned char * pDummy = new unsigned char [width*height*3];
	  unsigned char * pTest = pDummy;
	  if (!pDummy) {
	  	throw "Out of memory";
	  }
	  row_stride = width * cinfo.output_components;
	  pJpegBuffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	  while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
		for (int x = 0; x < width; x++) {
		  r = pJpegBuffer[0][cinfo.output_components * x];
		  if (cinfo.output_components > 2) {
		    g = pJpegBuffer[0][cinfo.output_components * x + 1];
		    b = pJpegBuffer[0][cinfo.output_components * x + 2];
		  } else {
		    g = r;
		    b = r;
		  }
		  *(pDummy++) = r;
		  *(pDummy++) = g;
		  *(pDummy++) = b;
		}
	  }
	  fclose(infile);
	  (void) jpeg_finish_decompress(&cinfo);
	  jpeg_destroy_decompress(&cinfo);

	  bmap = (unsigned char*)pTest; 
	  depth = 32;
	}
	
	void write(const char* filename, int quality=100) {
		FILE *ofp;
	  struct jpeg_compress_struct cinfo;   /* JPEG compression struct */
	  struct jpeg_error_mgr jerr;          /* JPEG error handler */
	  JSAMPROW row_pointer[1];             /* output row buffer */
	  int row_stride;                      /* physical row width in output buf */

	  if ((ofp = fopen(filename, "wb")) == NULL)
		throw "Error opening file";

	  cinfo.err = jpeg_std_error(&jerr);
	  jpeg_create_compress(&cinfo);
	  jpeg_stdio_dest(&cinfo, ofp);

	  cinfo.image_width = this->width;
	  cinfo.image_height = this->height;
	  cinfo.input_components = 3;
	  cinfo.in_color_space = JCS_RGB;

	  jpeg_set_defaults(&cinfo);
	  jpeg_set_quality(&cinfo, quality, 0);

	  jpeg_start_compress(&cinfo, TRUE);

	  /* Calculate the size of a row in the image */
	  row_stride = cinfo.image_width * cinfo.input_components;

	  /* compress the JPEG, one scanline at a time into the buffer */
	  while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &(bmap[(this->height - cinfo.next_scanline - 1)*row_stride]);
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	  }

	  jpeg_finish_compress(&cinfo);
	  jpeg_destroy_compress(&cinfo);

	  fclose(ofp);
	  
	  // Everything good so far :-)
	}
	
	virtual ~Jpeg() {
		if(this->bmap != NULL) {
			delete[] this->bmap;
			this->bmap = NULL;
		}
	}
	
	rgb_t operator()(int x, int y) {
		return this->rgb(x,y);
	}
	rgb_t rgb(int x, int y) {
		//if(bmap == NULL) throw "No data";
		rgb_t ret;
		
		const size_t index = 3*(width*y + x);
		ret.r = this->bmap[index];
		ret.g = this->bmap[index+1];
		ret.b = this->bmap[index+2];
		
		return ret;
	}
	/** Set pixel */
	void set(int x, int y, rgb_t rgb) {
		const size_t index = 3*(width*y + x);
		this->bmap[index  ] = rgb.r;
		this->bmap[index+1] = rgb.g;
		this->bmap[index+2] = rgb.b;
	}
};



#if 0
using namespace std;


int main() {
	try {
		Jpeg jpeg("webcam_output.jpeg");
		cout << "image read (" << jpeg.width << "x" << jpeg.height << ")" << endl;
		
		rgb_t rgb;
		
		rgb = jpeg(0,0);
		cout << "(0,0) = " << (int)rgb.r << ',' << (int)rgb.g << ',' << (int)rgb.b << endl;
		rgb = jpeg(0,1);
		cout << "(0,1) = " << (int)rgb.r << ',' << (int)rgb.g << ',' << (int)rgb.b << endl;
		rgb = jpeg(1,0);
		cout << "(1,0) = " << (int)rgb.r << ',' << (int)rgb.g << ',' << (int)rgb.b << endl;
		rgb = jpeg(1,1);
		cout << "(1,1) = " << (int)rgb.r << ',' << (int)rgb.g << ',' << (int)rgb.b << endl;
		rgb = jpeg(824,432);
		cout << "(824,432) = " << (int)rgb.r << ',' << (int)rgb.g << ',' << (int)rgb.b << endl;
		
	} catch (const char* msg) {
		cerr << "Error: " << msg << endl;
		return EXIT_FAILURE;
	} catch (...) {
		cerr << "Unknown error" << endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}


#endif
