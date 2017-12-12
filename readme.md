# Kino
> A research platform for wearable augmented reality devices

Kino is a software and hardware platform that greatly eases the process of developing
projects for wearable augmented reality devices (WARDs), particularly for research purposes.
The code in this repository compiles into a runtime which drives a custom reference WARD, which was
designed to be useful for research and comparatively inexpensive to build. As of now, Kino is 
being used at the VEMILab as a platform for exploring the efficacy of various AR techniques 
in aiding human spatial navigation.


## Features
### Modules
Modules are black boxes that accept two images, modify them, and send them out to the next step in the pipeline. 
These inherit from a [common interface](src/modules/ModuleCommon.hpp). If you are a researcher looking to
create a new module, all you need to do is create a new deriving class and implement its methods, then
instantiate it in the Core and register it with the Core's `modulePipeline` object. The system will handle the rest.

#### Edge Detector
Those who have Age-Related Macular Degeneration have trouble with visual search tasks due to
central field loss and degradation of contrast sensitivity. This module uses high performance 
edge detection to enhance the contours of objects, which improves the user's performance in these tasks.

#### Image Classifier
A work in progress. This is an attempt at visually augmenting objects in the user's field of view. 
These are detected through Darknet's YOLO, a CNN-based image classifier. I will be making various 
efforts to increase both the speed and integrity of the object detection, as these are important for comfort
on a pass-through configuration of WARD such as our reference device.

### Camera Support
Kino is designed to use a pair of PS3 Eye cameras connected via USB. However, if you want to use it with 
your webcam, you can do this by setting `DEMO_SETTINGS.ACTIVE` to `true` in the 
[config file](/bin/data/config/config.json), in which case the first registered webcam on your 
computer will be used.

### Utilities
Kino gives you many tools to simplify common tasks when writing code for WARDs. We provide a camera calibration module
that allows you to correct for lens distortion with a one-time calibration process. Visual performance measurement tools 
are also available for identifying trouble spots in code without needing to break out the debugger.


## Installation
Requirements:
- Nvidia GPU
- Windows 10 (8 will probably work, but is untested)

See the releases page for builds.

For the PS3 Eye cameras to be properly used, you'll need to use Zadig and set their drivers 
to "libusb-win32". Other drivers will cause the camera to lock up after a few seconds of use. 
You may have to do this again after restarting your computer. If it isn't working, try rebooting. 
Zadig will likely list the cameras as using "libusb0" after changing the drivers -- this is fine.

## How to Build
Requirements:
- Visual Studio 2015
- [CUDA 8.0 64-bit](https://developer.nvidia.com/cuda-downloads) (This means you need an Nvidia GPU!)
- [OpenFrameworks 0.9.8](http://openframeworks.cc/versions/v0.9.8/of_v0.9.8_vs_release.zip)

First, download OpenFrameworks, and clone this project into the `apps/myApps/` directory.
Then run [setup.bat](/setup.bat). This downloads a few large files as well as pulling the
submodules in the [addons](/addons) directory that the program needs to compile.

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