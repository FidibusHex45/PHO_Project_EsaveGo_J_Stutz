# Semesterprojekt esave Go

## CameraSettings
### Image Formats
The Image formats for the IDS camera get basicly interpreted as predefinded areas of interest (AOI).
Hereinafter is a list of souported formats for the this [camera](https://www.1stvision.com/cameras/IDS/IDS-manuals/uEye_Manual/camera-data-ui-125x.html):

| Format ID | Resolution                   | Name         |
| --------- | ---------------------------- | ------------ |
| 5         | 2048 x 1536                  | 3M           |
| 6         | 1920 x 1080                  | Full HD 16:9 |
| 8         | 1280 x 960                   | 1.2M 4:3     |
| 9         | 1280 x 720                   | HD 16:9      |
| 13        | 640 x 480                    | VGA          |
| 18        | 320 x 240                    | QVGA         |
| 20        | 1600 x 1200                  | UXGA         |
| 25        | 1280 x 1024                  | 1.3M SXGA    |
| 26        | 2448 x 2048                  | 5M           |
| 27        | 1024 x 768                   | XGA          |
| 28        | 1024 x 1024                  | 1M           |
| 29        | 800 x 600                    | SVGA         |
| 35        | 1920 x 1200                  | WUXGA        |
| 36        | Sensor maximum (2456 x 2054) |              |
| 40        | 2048 x 2048                  | 4M           |

The image format can be selected by inserting the corresponding format ID in the [cameraSettings.json](./prop/cameraSettings.json)-file e.g.
~~~json
"imageFormat": 20
~~~

### Color modes
The [camera](https://www.1stvision.com/cameras/IDS/IDS-manuals/uEye_Manual/camera-data-ui-125x.html) offers plenty of different color modes. These are listed in the table underneath.

| ID 	| ID name               	| Description             |
|----	|-----------------------	|-------------------------|
| 28 	| IS_CM_MONO16          	| Grayscale (16)          |
| 26 	| IS_CM_MONO12          	| Grayscale (12)          |
| 34 	| IS_CM_MONO10          	| Grayscale (10)          |
| 6  	| IS_CM_MONO8           	| Grayscale (8)       	  |
| 29 	| IS_CM_SENSOR_RAW16    	| Raw sensor data (16) 	  |
| 27 	| IS_CM_SENSOR_RAW12    	| Raw sensor data (12) 	  |
| 33 	| IS_CM_SENSOR_RAW10    	| Raw sensor data (10) 	  |
| 11 	| IS_CM_SENSOR_RAW8     	| Raw sensor data (8)  	  |
| 30 	| IS_CM_BGR12_UNPACKED  	| Unpacked BGR (12 12 12) |
| 35 	| IS_CM_BGR10_UNPACKED  	| Unpacked BGR (10 10 10) |
| 25 	| IS_CM_BGR10_PACKED    	| BGR (10 10 10)          |
| 1  	| IS_CM_BGR8_PACKED     	| BGR (8 8 8)             |
| 31 	| IS_CM_BGRA12_UNPACKED 	| Unpacked BGR (12 12 12) |
| 0  	| IS_CM_BGRA8_PACKED    	| BGR (8 8 8)             |
| 24 	| IS_CM_BGRY8_PACKED    	| BGRY (8 8 8)            |
| 2  	| IS_CM_BGR565_PACKED   	| BGR (5 6 5)             |
| 3  	| IS_CM_BGR5_PACKED     	| BGR (5 5 5)         	  |

`Note:`
Please be aware, that not all colorformats are soupported by the openCV [cv::Mat](https://docs.opencv.org/4.x/d3/d63/classcv_1_1Mat.html) class. Valid colorformats for cv::Mat can be looked up [here](https://gist.github.com/yangcha/38f2fa630e223a8546f9b48ebbb3e61a). Formats that will work are therefore:
* IS_CM_MONO8
* IS_CM_MONO16
* IS_CM_SENSOR_RAW8
* IS_CM_SENSOR_RAW16

Since the [camera](https://www.1stvision.com/cameras/IDS/IDS-manuals/uEye_Manual/camera-data-ui-125x.html) is greyscale only, all colorformats will not work with this setting. We recommend using the `IS_CM_MONO8` color mode, since it's the least computationlly intensive mode e.g.
~~~json
"colorMode": 6
~~~

### Exposure time / integration time
The exposure time can be adjusted freely in the range from 0.02 - 10000ms e.g.
~~~json
"exposureTime": 15.0
~~~

### Shutter mode
The [camera](https://www.1stvision.com/cameras/IDS/IDS-manuals/uEye_Manual/camera-data-ui-125x.html) offers two shuttermodes: rolling and global shutter.
The shutter mode can also be selected in the [cameraSettings.json](./prop/cameraSettings.json)-file.
* rolling shutter by setting the `shutterMode` to `1`
* global shutter by setting the `shutterMode` to `2`
e.g.
~~~json
"shutterMode": 2
~~~

### Pixelclock 
The pixelclick can only be set to specific values, these values are shown below:
* 118 (MHz)
* 237 (MHz)
* 474 (MHz)
~~~json
"pixelCock": 237
~~~

### Framerate
The [camera](https://www.1stvision.com/cameras/IDS/IDS-manuals/uEye_Manual/camera-data-ui-125x.html) can handle framerates in `freerun` mode from 1 up to 52fps. e.g.
~~~json
"fps": 30
~~~

### Offset
The offset or blacklevel is adustable in the range of 0-255. Under normal conditions we recommend setting the `offset` to `0`. e.g.
~~~json
"offset": 0
~~~

### Gain
The gain can be adjusted in the range from 1 to 4. e.g.
~~~json
"gain": 3
~~~

### Gamma



### Binning


### Region of interest (AOI)
The region of interest (uEye notation: AOI - area of interest) can activated/deactivated by setting the `enableRoi` [cameraSettings.json](./prop/cameraSettings.json)-file either to `0` or to `1`:
~~~json
"enableRoi": 1
~~~
or
~~~json
"enableRoi": 0
~~~
The boundary conditions for the AOI are listed in the table below:
|                          |            |
| ------------------------ | ---------- |
| min roi width            | 256 (pxl)  |
| max roi width            | 2456 (pxl) |
| step width               | 8 (pxl)    |
|                          |            |
| min roi height           | 2 (pxl)    |
| max roi height           | 2054 (pxl) |
| step width               | 2 (pxl)    |
|                          |            |
| position grid horizontal | 4 (pxl)    |
| position grid vertical   | 2 (pxl)    |

Example:
~~~json
"enableRoi": 1,
    "roi": {
      "x": 100,
      "y": 50,
      "width": 512,
      "height": 1024
    }
~~~

~~~mermaid
---
title: C++
---
classDiagram
  Camera ..> MemHandler
  Camera ..> TrackHandler
  Camera ..> CarController
  MemHandler ..> TrackHandler
  MemHandler ..> CarController
  CSVHandler ..> TrackHandler
  CSVHandler ..> CarController
  SerialSTM32 ..> TrackHandler
  SerialSTM32 ..> CarController
  class Camera {
    +configureCam(string)
    +startAqusition(int)
    +getFPS()
    +getImageFormat()
    +getColorMode()
    +getAvailableFormats()
    +stopAquisition()
  }
  class MemHandler {
    +allocRingBuffer()
    +allocSingleBuffer()
    +getOpenCVMatRingImg()
    +getOpenCVMatSingeImg()
    +drawFpsOnImg(Mat)
    +drawDroppedFramesOnImg(Mat)
    +drawTimeStamp(Mat)
    +saveSingleImg(string, optional~Mat~)
    +getActiveBufferID()
    +getRealFps()
    +getTimeStamp()
    +int activeBufferIdx
    +int imgWidth
    +int imgHeight
  }
  class TrackHandler {
    +getTrackData(string, string)
  }
  class CSVHandler {
    +save2csv(string, vector~Point~)
    +loadSplineData(string, int, int)
    +loadSplineDataConfig(string)
    +saveSplineData(string, vector~splineData_proc~)
  }
  class SerialSTM32 {
    +writeSerialPort(int)
    +closeSerialPort()
    +bool connected_
  }
  class CarController {
    +run()
  }
~~~

~~~mermaid
classDiagram
  CarreraTrackAnalysisApp <.. CSVHAndler
  CarreraTrackAnalysisApp <.. PointSorter
  CarreraTrackAnalysisApp <.. SequentialPCA
  CarreraTrackAnalysisApp <.. SplineInterpolation
  CarreraTrackAnalysisApp <.. VelocityEditor
  class CSVHAndler {
    +__init__(self)
    +loadCSV_Points(self, path)
    +saveData_2CSV(self, data, path)
  }
  class PointSorter {
    +__init__(self, points)
    +sortPoints(self, refPoint)
    +points
    +tree
  }
  class SequentialPCA {
    +__init__(self, points)
    +seqPCA(self, nPCA, nAvg)
    +points
    +pcaS
    +pcaS_diff
    +pcaS_diff_avg
  }
  class SplineInterpolation {
    +__init__(self, points, numKnots, smoothing, periodic, derivative)
    +interpolate(self)
    +points
    +numKnots
    +smoothing
    +periodic
    +derivative
    +bInt_SplinePoints
  }
  class VelocityEditor {
    +__init__(self, data, color_scheme)
    +adjust(self)
    +x_data
    +y_data
    +v_data
    +cmap
    +color_original
    +color_adjust
    +points
    +v_adjust
    +indices_picked_points
    +rotB
  }
  class CarreraTrackAnalysisApp {
    +__init__(self, path_load, path_write, ref_point)
    +run(self)
    +path_write
    +path_load
    +ref_point
    +csvHandler
  }

~~~




