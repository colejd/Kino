# Overview

This document will serve as a broad overview of program functionality and how each piece interacts.

## main.h
Launch point for the program. Creates the OpenFrameworks instance (`ofApp`) and launches it.

## `ofApp`
The de facto main portion of the code. Responsible for managing every other part of the program.
Has hooks for OpenFrameworks' `Update`/`Draw` calls, which are done repeatedly in that order as 
often as possible. It's also responsible for drawing the GUI.

Contains the Core, Compositor, and GUI.

## Core
This is responsible for fetching images from the cameras (through `CameraCapture` objects), as
well as processing them through each individual Lens.

### Lenses
Lenses are the in-program term for self-contained image processing routines. They inherit from
`LensCommon`. Broadly, they have a `ProcessFrame` function that takes an input image, and
writes its results to an output image. They are enabled and disabled individually so that
you can mix and match what processing you want to do. Each one manages its own GUI (though
the code that draws it is in the Core).

#### Edge Detector
Processes the input image using Canny edge detection (and optionally, contour filtering)
to highlight the edges of objects.

#### Image Classifier
Processes the input image using a neural network (YOLO, part of Darknet) to perform object
detection and tracking. The output image is the input image with alterations to make these
objects more obvious. This constitutes my thesis project, so contact me if you have any
questions!

## Compositor
The Compositor takes the images from each camera and displays them on screen. It creates a buffer
that is large enough for both images, and then aspect scales and fits it to the window. It converges
the images toward the center of the screen depending on user settings, to accomodate for various 
interpupillary distances.

The Compositor is meant as an abstraction for drawing the images to an HMD, similar to how SteamVR
uses a Compositor class to display a game on various VR devices (formatting it for each device is
the Compositor's job).

It can be set to a demo mode, where only the left webcam draws and it fills the screen completely.
This is useful for testing on a computer with a webcam or some similar situation.


## Camera Capture
These are made as an abstraction for various cameras and the APIs that are used to access their
frame data. Right now, we support PS3 Eye cameras, as well as any camera recognized as a webcam
by the OS (via OpenCV). If you have another type of camera that doesn't have native support but 
has an API, you'll want to make something inheriting from `CaptureBase` and update `CameraCapture` 
to match. 

You can also use `StartFakeCapture()` to emulate a camera capture session with a video file. This
can be useful for testing or demo purposes.

-------------------------

## GUI Notes
This program uses ofxImGui, which itself uses Dear Imgui internally. I outright avoid
ofxImGui's interface since I only use it for its ready-to-go integration with OpenFrameworks,
and I instead use Imgui directly. If you're modifying this code, I recommend that you do the 
same.