/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-05-14
 ******************************/
#ifndef __ZXING___
#define __ZXING___
#include <iostream>
#include <fstream>
#include <string>
#include "ImageReaderSource.h"
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <exception>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/multi/qrcode/QRCodeMultiReader.h>
#include <zxing/multi/ByQuadrantReader.h>
#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/multi/GenericMultipleBarcodeReader.h>
#pragma comment(lib,"lib/zxing/build/Release/libzxing.lib")

#include "lib/libqrencode/qrencode.h"
#pragma comment( lib, "Release/libqrencode.lib" )

#include "lib/cximage/CxImage/ximage.h"
#pragma comment( lib, "lib/cximage/CxImage/Release/cximage.lib" )
#pragma comment( lib, "lib/cximage/Release/jasper.lib" )
#pragma comment( lib, "lib/cximage/Release/jbig.lib" )
#pragma comment( lib, "lib/cximage/Release/jpeg.lib" )
#pragma comment( lib, "lib/cximage/Release/libdcr.lib" )
#pragma comment( lib, "lib/cximage/Release/libpsd.lib" )
#pragma comment( lib, "lib/cximage/Release/mng.lib" )
#pragma comment( lib, "lib/cximage/Release/png.lib" )
#pragma comment( lib, "lib/cximage/Release/tiff.lib" )
#pragma comment( lib, "lib/cximage/Release/zlib.lib" )

char* qrdecode(char* filename);
char* qrencode(char* str,int imageWidth,char* save_path);
#endif