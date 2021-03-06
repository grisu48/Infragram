/** 2018, Felix Niederwanger
  * This is my infragram code for taking pictures coming from the infragram camera
  */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <fstream>
#include <string>

// Jpeg reading routines
#include "jpeg.hpp"
#include "ndvi.hpp"


using namespace std;



/*
 * This routine read a picture from V4L and stores it as JPEG in the given filename
 * This part is based on https://gist.github.com/mike168m/6dd4eb42b2ec906e064d
 */
static int read_video(const char* device, int width, int height, const char* jpg_filename) {
	int fd; // A file descriptor to the video device
    fd = open(device, O_RDWR);
    if(fd < 0) 
    	return -1;


    // 2. Ask the device if it can capture frames
    v4l2_capability capability;
    if(ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0){
        // something went wrong... exit
        cerr << "Failed to get device capabilities, VIDIOC_QUERYCAP" << endl;
	    close(fd);
        return -2;
    }


    // 3. Set Image format
    v4l2_format imageFormat;
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = width;
    imageFormat.fmt.pix.height = height;
    imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;  //V4L2_PIX_FMT_DV
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    // tell the device you are using this format
    if(ioctl(fd, VIDIOC_S_FMT, &imageFormat) < 0){
        cerr << "Device could not set format, VIDIOC_S_FMT" << endl;
	    close(fd);
        return -3;
    }


    // 4. Request Buffers from the device
    v4l2_requestbuffers requestBuffer;// = {0};
    requestBuffer.count = 1; // one request buffer
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // request a buffer wich we can use for capturing frames
    requestBuffer.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd, VIDIOC_REQBUFS, &requestBuffer) < 0){
        cerr << "Could not request buffer from device, VIDIOC_REQBUFS" << endl;
	    close(fd);
        return -4;
    }


    // 5. Quety the buffer to get raw data ie. ask for the you requested buffer
    // and allocate memory for it
    v4l2_buffer queryBuffer;// = {0};
    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0;
    if(ioctl(fd, VIDIOC_QUERYBUF, &queryBuffer) < 0) {
        cerr << "Device did not return the buffer information, VIDIOC_QUERYBUF" << endl;
	    close(fd);
        return -5;
    }
    // use a pointer to point to the newly created buffer
    // mmap() will map the memory address of the device to
    // an address in memory
    char* buffer = (char*)mmap(NULL, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, queryBuffer.m.offset);
    memset(buffer, 0, queryBuffer.length);


    // 6. Get a frame
    // Create a new buffer type so the device knows whichbuffer we are talking about
    v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;

    // Activate streaming
    int type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        cerr << "Could not start streaming, VIDIOC_STREAMON" << endl;
	    close(fd);
        return -6;
    }

/***************************** Begin looping here *********************/
    // Queue the buffer
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        cerr << "Could not queue buffer, VIDIOC_QBUF" << endl;
	    close(fd);
        return -7;
    }

    // Dequeue the buffer
    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
        cerr << "Could not dequeue the buffer, VIDIOC_DQBUF" << endl;
	    close(fd);
        return -8;
    }
    // Frames get written after dequeuing the buffer

    //cout << "Buffer has: " << (double)bufferinfo.bytesused / 1024 << " KBytes of data" << endl;

    // Write the data out to file
    ofstream outFile;
    outFile.open(jpg_filename, ios::binary| ios::app);

    int bufPos = 0, outFileMemBlockSize = 0;  // the position in the buffer and the amoun to copy from
                                        // the buffer
    int remainingBufferSize = bufferinfo.bytesused; // the remaining buffer size, is decremented by
                                                    // memBlockSize amount on each loop so we do not overwrite the buffer
    char* outFileMemBlock = NULL;  // a pointer to a new memory block
    //int itr = 0; // counts thenumber of iterations
    while(remainingBufferSize > 0) {
        bufPos += outFileMemBlockSize;  // increment the buffer pointer on each loop
                                        // initialise bufPos before outFileMemBlockSize so we can start
                                        // at the begining of the buffer

        outFileMemBlockSize = 1024;    // set the output block size to a preferable size. 1024 :)
        outFileMemBlock = new char[sizeof(char) * outFileMemBlockSize];

        // copy 1024 bytes of data starting from buffer+bufPos
        memcpy(outFileMemBlock, buffer+bufPos, outFileMemBlockSize);
        outFile.write(outFileMemBlock,outFileMemBlockSize);

        // calculate the amount of memory left to read
        // if the memory block size is greater than the remaining
        // amount of data we have to copy
        if(outFileMemBlockSize > remainingBufferSize)
            outFileMemBlockSize = remainingBufferSize;

        // subtract the amount of data we have to copy
        // from the remaining buffer size
        remainingBufferSize -= outFileMemBlockSize;

        // display the remaining buffer size
        //cout << itr++ << " Remaining bytes: "<< remainingBufferSize << endl;
    }

    // Close the file
    outFile.close();


/******************************** end looping here **********************/

    // end streaming
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        cerr << "Could not end streaming, VIDIOC_STREAMOFF" << endl;
	    close(fd);
        return -9;
    }

    close(fd);
    return 0;
}



int main(int argc, char** argv) {
    const char* device = "/dev/video0";
    const char* c_filename = "camera.jpeg";
    const char* d_filename = "ndvi.jpeg";
    bool delete_camera = true;
    bool verbose = false;
    
    if(argc < 2) {
    	cout << "Infragram NDVI processing program" << endl;
    	cout << "  2018, Felix Niederwanger" << endl;
    	cout << endl;
    	cout << "This program reads a picture from a INFRAGRAM camera and writes the NDVI processed jpeg picture to a file" << endl;
				cout << endl;
    	cout << "Usage: " << argv[0] << " JPEG" << endl;
    	cout << "  For help type " << argv[0] << " --help" << endl;
		cout << endl;
		cout << "Example usage" << endl;
		cout << "  " << argv[0] << " ndvi.jpeg              - Read default camera and writes NDVI picture to 'ndvi.jpeg'" << endl;
    	return EXIT_SUCCESS;
    }
    
    int iarg = 0;
    for(int i=1; i<argc; i++) {
    	string arg(argv[i]);
    	if(arg.size() == 0) continue;
    	if(arg.at(0) == '-') {		// Command
    		if (arg == "-h" || arg == "--help") {
    			cout << "Infragram NDVI processing program" << endl;
				cout << "  2018, Felix Niederwanger" << endl;
				cout << endl;
				cout << "This program reads a picture from a INFRAGRAM camera and writes the NDVI processed jpeg picture to a file" << endl;
				cout << endl;
				cout << "Usage: " << argv[0] << " [OPTIONS] NDVI-JPEG [CAMERA-JPEG]" << endl;
				cout << endl;
				cout << "Examples" << endl;
				cout << "  " << argv[0] << " my_awesome_picture.jpg                  Writes NDVI JPEG to 'my_awesome_picture.jpg'" << endl;
				cout << "  " << argv[0] << " -i /dev/video1 my_awesome_picture.jpg   Reads from /dev/video1 and writes NDVI JPEG to 'my_awesome_picture.jpg'" << endl;
				cout << "  " << argv[0] << " -i /dev/video1 ndvi.jpg raw.jpg         Reads from /dev/video1, writes camera file to 'raw.jpg' and the ndvi file to 'ndvi.jpg'" << endl;
				cout << endl;
				cout << "OPTIONS" << endl;
				cout << "   -i,-dev DEVICE                     Set device where to read from (see 'ls /dev/video*')" << endl;
				cout << "   -v, --verbose                      Verbose mode" << endl;
    		} else if(arg == "-v" || arg == "--verbose") {
    			verbose = true;
    		} else if(arg == "-i" || arg == "--device" || arg == "--dev") {
    			device = argv[++i];
    		} else {
    			cerr << "Illegal option: " << arg << endl;
    			return EXIT_FAILURE;
    		}
    	} else {
    		if (iarg == 0) {
    			d_filename = argv[i];
    		} else if(iarg == 1) {
    			c_filename = argv[i];
    			delete_camera = false;
    		} else {
    			cerr << "Too many arguments given" << endl;
    			return EXIT_FAILURE;
    		}
    		iarg++;
    	}
    }
    
    
    ::unlink(c_filename);
    if(verbose) { cout << "Reading image (" << device << ") ... " << endl; }
 	int ret = read_video(device, 1600, 1200, c_filename);
 	if(ret != 0) {
 		cerr << "Error reading from camera: (" << ret << ") - " << strerror(errno) << endl;
 		return EXIT_FAILURE;
 	}
 	if(verbose)
 		cout << "Image received from camera (" << c_filename << ")" << endl;
 
 	try {
 		Jpeg jpeg(c_filename);
 		if(verbose) cout << "NDVI-processing image ... " << endl;
 		Jpeg j_ndvi = ndvi(jpeg);
 		
 		if(delete_camera)
 			::unlink(c_filename);
 		::unlink(d_filename);
 		j_ndvi.write(d_filename);
 		
 	} catch (const char* err) {
 		cerr << "Error processing image: " << err;
 		if(errno != 0) cerr << " (" << strerror(errno) << ")";
 		cerr << endl;
 		return EXIT_FAILURE;
 	}
 
 	if(verbose) {
 		cout << "Process completed" << endl;
 		if(!delete_camera) cout << "  Camera picture written to " << c_filename << endl;
 		cout << "  NDVI image written to " << d_filename << endl;
 	}	
 	
    return EXIT_SUCCESS;
}
