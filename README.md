# Esave Go

## CameraSettings
### Image Formats
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

### Color modes
| ID 	| ID name               	| Description                                                                  	|
|----	|-----------------------	|------------------------------------------------------------------------------	|
| 28 	| IS_CM_MONO16          	| Grayscale (16), for monochrome and color cameras, LUT/gamma active           	|
| 26 	| IS_CM_MONO12          	| Grayscale (12), for monochrome and color cameras, LUT/gamma active           	|
| 34 	| IS_CM_MONO10          	| Grayscale (10), for monochrome and color cameras, LUT/gamma active           	|
| 6  	| IS_CM_MONO8           	| Grayscale (8), for monochrome and color cameras, LUT/gamma active            	|
| 29 	| IS_CM_SENSOR_RAW16    	| Raw sensor data (16), for monochrome and color cameras, LUT/gamma not active 	|
| 27 	| IS_CM_SENSOR_RAW12    	| Raw sensor data (12), for monochrome and color cameras, LUT/gamma not active 	|
| 33 	| IS_CM_SENSOR_RAW10    	| Raw sensor data (10), for monochrome and color cameras, LUT/gamma not active 	|
| 11 	| IS_CM_SENSOR_RAW8     	| Raw sensor data (8), for monochrome and color cameras, LUT/gamma not active  	|
| 30 	| IS_CM_BGR12_UNPACKED  	| Unpacked BGR (12 12 12), for monochrome and color cameras, LUT/gamma active  	|
| 35 	| IS_CM_BGR10_UNPACKED  	| Unpacked BGR (10 10 10), for monochrome and color cameras, LUT/gamma active  	|
| 25 	| IS_CM_BGR10_PACKED    	| BGR (10 10 10), for monochrome and color cameras, LUT/gamma active           	|
| 1  	| IS_CM_BGR8_PACKED     	| BGR (8 8 8), for monochrome and color cameras, LUT/gamma active              	|
| 31 	| IS_CM_BGRA12_UNPACKED 	| Unpacked BGR (12 12 12), for monochrome and color cameras, LUT/gamma active  	|
| 0  	| IS_CM_BGRA8_PACKED    	| BGR (8 8 8), for monochrome and color cameras, LUT/gamma active              	|
| 24 	| IS_CM_BGRY8_PACKED    	| BGRY (8 8 8), for monochrome and color cameras, LUT/gamma active             	|
| 2  	| IS_CM_BGR565_PACKED   	| BGR (5 6 5), for monochrome and color cameras, LUT/gamma active              	|
| 3  	| IS_CM_BGR5_PACKED     	| BGR (5 5 5), for monochrome and color cameras, LUT/gamma active              	|
| 12 	| IS_CM_UYVY_PACKED     	| YUV 4:2:2 (8 8), for monochrome and color cameras, LUT/gamma active          	|
| 23 	| IS_CM_CBYCRY_PACKED   	| YCbCr 4:2:2 (8 8), for monochrome and color cameras, LUT/gamma active        	|

### Valid pixelclock params
* 118 (MHz)
* 237 (MHz)
* 474 (MHz)

### Valid Roi's
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