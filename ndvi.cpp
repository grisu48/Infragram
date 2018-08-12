/* 2018, Felix Niederwanger
 * Standalone ndvi program
 */

#include <iostream>
#include <string.h>

#include "ndvi.hpp"


#define VERSION "0.1"

int main(int argc, char** argv) {
	if(argc < 2) {
		cerr << "Missing arguments" << endl;
		cerr.flush();
		cout << "Usage: " << argv[0] << " INPUT [OUTPUT]" << endl;
		cout << "  Type " << argv[0] << " --help for help" << endl;
		return EXIT_FAILURE;
	}
	
	// Input/output files from command line
	char* input = argv[1];
	char* output = (char*)"ndvi.jpeg";
	
	if(!strncmp(input, "-h" , 2) || !strncmp(input, "--help", 6)) {
		cout << "NDVI processor, Version " << VERSION << endl;
		cout << "  This programm applies the NDVI analysis on a given JPEG file," << endl;
		cout << "  assuming the IR is in the RED channel, and the integrated VISIBLE light is in the BLUE channel" << endl;
		cout << "  (This is the default for the infragram camera for which it was developed for)" << endl;
		cout << endl;
		cout << "Usage: " << argv[0] << " INPUT [OUTPUT]" << endl;
		cout << "  INPUT       defined the input JPEG file" << endl;
		cout << "  OUTPUT      is optional and defined the destination JPEG file. If not set, the program will use '" << output << "'" << endl;
		cout << endl;
		cout << "  2018, Felix Niederwanger" << endl;
		cout << "        Have fun! :-)" << endl;
		return EXIT_SUCCESS;
	}
	if(argc > 2) output = argv[2];
	
	
	// Process ndvi
	try {
		// Jep, that's all :-)
		Jpeg jpg(input);
		Jpeg j_ndvi = ndvi(jpg);
		j_ndvi.write(output);
	} catch (const char* err) {
		cerr << "Error: " << err << endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
