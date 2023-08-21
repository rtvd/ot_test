# What is this?

This is a toy application which processes a video and tracks the movement of a "region of interest".

The video is read from a file, a user clicks at the middle of the ROI and then the movement of the region is tracked.
The results are shown and also written into an output video file.

Also, a log file of ROI coordinates is created. The log file is written out in CSV format.


# How to build?

The code should be possible to compile with OpenCV 4.2.
Your OpenCV must have support for both trackers and QT compiled in, which means most of the builds won't work.

In order to build a custom release:
1. Download OpenCV 4.2.0 from here https://opencv.org/releases/ and extract locally.
2. Check out OpenCV contrib from here https://github.com/opencv/opencv_contrib and switch it to tag 4.2.0.
3. Create an empty directory and go into it.
4. Decide on where would the new OpenCV be installed.
5. Configure the build like this: `cmake CMAKE_BUILD_TYPE=RELEASE -DOPENCV_EXTRA_MODULES_PATH=<source code of opencv_contrib>/modules -DCMAKE_INSTALL_PREFIX=<source code of opencv> -DWITH_QT=ON <where to install to>`
6. Run the build: `make`
7. Install the build: `make install`

When you have this custom build of OpenCV, you can use choose it when building this source like this:
```
cmake -DCMAKE_PREFIX_PATH=<location of custom build of OpenCV>
```

# How to launch?

On Linux, you can start it from command line like this:

```
./ot_test <method> <input video file> <output video file.mp4v> <log file> [<ROI size>]
```

Supported Methods:

* MIL
* Boosting
* MedianFlow
* TLD
* KCF
* GOTURN
* MOSSE

It is not necessary to specify ROI size but if you do, that would be in percents of the biggest dimension of the frame.
For example, 5 means 5% of the frame's height (in case height is smaller than width).

# Known issues

When a window with the first frame is opened, its size should be the same as the size of the frame.
However, when the frame is too large, the window will be smaller than the frame and the code which identifies the ROI
would fail.