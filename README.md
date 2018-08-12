# Infragram

This little project aims to provide an easy-to-use solution for the Infragram DIY Plant camera kit from [PublicLab](https://publiclab.org/).

The program reads out the Infragram camera and processes the image using the [NDVI Analysis](https://publiclab.org/wiki/ndvi), assuming that the infrared is in the red channel, and the total visible light is in the blue channel.
This is valid for the Infragram camera.

## Installation

Prerequirements: `libjpeg`, and `Video4Linux` (by default included in most Kernels)

`libjpeg` is also included in most distros by default (Tested in Ubuntu 18.04 and Arch Linux)

Compile the programs using

    make -j2

Installation via `make install` is not (yet) supported, because it's still experimental.


## Usage

This repository comes with two main programs:

* `ndvi` - Process a given JPEG image
* `infragram` - Read picture from camera and process the image

### Infragram

The most basic usage is

    # Capture picture, process is via NDVI and write it to 'ndvi.jpeg'
    ./infragram ndvi.jpeg

You can keep the camera picture by providing a second jpeg file as program parameter

    # Same as above, but keep the campera picture as 'camera.jpeg.
    ./infragram ndvi.jpeg camera.jpeg

If you are having more than one camera devices (e.g. a webcam) you will need to configure the right input device via `-i`

    # Configure input device via '-i DEVICE'
    ./infragram -i /dev/video1 ndvi.jpeg

Putting all together:

    # Read from /dev/video1, store NDVI image as 'ndvi.jpeg', the camera image as 'camera.jpeg' and provide verbose output
    ./infragram -v -i /dev/video1 ndvi.jpeg camera.jpeg

For more informations type

    ./infragram -h

### NVDI

This simple program takes a JPEG picture and performs the NDVI analysis on it

    # Take 'camera.jpeg' and write result to default file ('ndvi.jpeg')
    ./ndvi camera.jpeg
    
    # Process 'camera.jpeg' but write result to 'output.jpeg'
    ./ndvi camera.jpeg output.jpeg


# Remarks

## Colormaps 

Currently only one colormap is supported: Red to green, where red is the minimum and green is the maximum


# License

This program is licensed under the (WTFPL)[http://www.wtfpl.net/] License.
