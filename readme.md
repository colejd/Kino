# Kino
> An AR platform for enhancing navigational awareness

Kino is a software platform for real-time AR on a custom-built headset. As of now, Kino 
is being used at the VEMILab as a platform for exploring the efficacy of various
AR techniques for aiding human spatial navigation.

*Please note that this is a work in progress!*


## Features
### Lenses
Each lens modifies an image using various processing techniques to improve
navigational awareness.

#### Edge Detector
Those who lose contrast sensitivity in their eyes have trouble perceiving depth.
High performance edge detection is used to convey depth information in a way that
they can see.

#### Image Classifier
A work in progress. This is an attempt at visually augmenting important indooor
scene features that are in the user's field of vision. Right now, these are
light switches, doorknobs, and exit signs. These are detected through Darknet's YOLO,
a CNN-based image classifier. I will be making various changes to speed it up
further, as a baseline of 60 FPS is very important for wearable AR.

Please note that this code is in a very early state.

### Camera Support
Kino is designed to use a pair of PS3 Eye cameras connected via USB.
However, if you want to use it with your webcam, you can do this
by setting `WEBCAM_DEMO_MODE` to `true` in the [config file](/bin/data/config/config.json), in which
case the first registered webcam on your computer will be used.


## Installation
See the releases page for builds.

For the PS3 Eye cameras to be properly used, you'll need to use Zadig and set their drivers 
to "libusb-win32". Other drivers will cause the camera to lock up after a few seconds of use. 
You may have to do this again after restarting your computer. If it isn't working, try rebooting. 
Zadig will likely list the cameras as using "libusb0" after changing the drivers -- this is fine.

## How to Build
Requirements:
- Visual Studio 2015
- [CUDA 8.0 64-bit](https://developer.nvidia.com/cuda-downloads)
- OpenFrameworks 0.9.8

First, download OpenFrameworks, and clone this project into the `apps/myApps/` directory.
Then run [setup.bat](/setup.bat). This downloads a few large files as well as some 
addons that will need to be downloaded for compilation.

You should then be able to open the project in Visual Studio and build.


## Credits
This project uses:
- [ofxImGui](https://github.com/jvcleave/ofxImGui)
- [ofxFontStash](https://github.com/armadillu/ofxFontStash)
- [ofxTimeMeasurements](https://github.com/armadillu/ofxTimeMeasurements)
- [ofxDarknet](https://github.com/mrzl/ofxDarknet)
- [OpenCV](http://opencv.org/)
- [PS3EyeDriver](https://github.com/inspirit/PS3EYEDriver)
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
- [ProggyClean font](http://www.proggyfonts.net/)