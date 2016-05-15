/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-05-14
 ******************************/
#include "Windows.h"
#include "zxing.h"

char* qrencode(char* str,int imageWidth,char* save_path){
	QRcode *qrcode;
	qrcode = QRcode_encodeString(str, 4, QR_ECLEVEL_H, QR_MODE_8, 1);//QR_MODE_8

	if(imageWidth<qrcode->width){
		imageWidth=qrcode->width;
	}
	unsigned char *pixels = qrcode->data;
    int width =  qrcode->width;
    int len = width * width;

    if (imageWidth < width)
        imageWidth = width;

	HDC hdc=::GetDC(NULL);
	HBITMAP m_hbm = CreateCompatibleBitmap(hdc, qrcode->width, qrcode->width);
	BITMAP bm;
	BYTE *m_bits;
    if (GetObject(m_hbm, sizeof(bm), &bm))
    {
        const int bytesPerPixel = bm.bmBitsPixel / 8;
        DWORD cb = (bm.bmWidthBytes + bm.bmWidthBytes % sizeof(DWORD)) * qrcode->width;
        if (m_bits = new BYTE[cb])
        memset(m_bits, 0, cb);
            for (int x = 0; x < qrcode->width; x++)
            {
                for (int scanLine = 0; scanLine < qrcode->width; scanLine++)
                {
                    const int y = bm.bmHeight - scanLine - 1;

                    if (0 == (qrcode->data[y * qrcode->width + x] & 0x1))
                    {
                        for (int ixColorByte = 0; ixColorByte < bytesPerPixel; ixColorByte++)
                            m_bits[scanLine * bm.bmWidthBytes + x * (bytesPerPixel) + ixColorByte] = 0xFF;
                    }
                }
            }
        }
    


	CxImage  image;
	image.CreateFromArray(m_bits, qrcode->width, qrcode->width,32,qrcode->width*4,false);
	image.Resample(imageWidth,imageWidth);

	if(save_path!=NULL){
			image.Save(save_path,CXIMAGE_FORMAT_JPG);
	}

	int32_t size=0;//得到图像大小
    BYTE* buffer=0;//存储图像数据的缓冲
    image.Encode(buffer,size,CXIMAGE_FORMAT_JPG);//把image对象中的图像以type类型数据copy到buffer
	const char EncodeTable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    //返回值
    std::string strEncode;
    unsigned char Tmp[4]={0};
    int LineLength=0;
    for(int i=0;i<(int)(size / 3);i++)
    {
        Tmp[1] = *buffer++;
        Tmp[2] = *buffer++;
        Tmp[3] = *buffer++;
        strEncode+= EncodeTable[Tmp[1] >> 2];
        strEncode+= EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
        strEncode+= EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
        strEncode+= EncodeTable[Tmp[3] & 0x3F];
        if(LineLength+=4,LineLength==76) {strEncode+="\r\n";LineLength=0;}
    }
    //对剩余数据进行编码
    int Mod=size % 3;
    if(Mod==1)
    {
        Tmp[1] = *buffer++;
        strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4)];
        strEncode+= "==";
    }
    else if(Mod==2)
    {
        Tmp[1] = *buffer++;
        Tmp[2] = *buffer++;
        strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
        strEncode+= EncodeTable[((Tmp[2] & 0x0F) << 2)];
        strEncode+= "=";
    }
	QRcode_free(qrcode);
	::ReleaseDC(NULL,hdc);
	free(m_bits);
	::DeleteObject(m_hbm);
	image.Destroy();
	return (char*)strEncode.c_str();
}








using namespace std;
using namespace zxing;
using namespace zxing::multi;
using namespace zxing::qrcode;

namespace {

bool more = false;
bool test_mode = false;
bool try_harder = false;
bool search_multi = false;
bool use_hybrid = false;
bool use_global = false;
bool verbose = false;

}

vector<Ref<Result> > decode(Ref<BinaryBitmap> image, DecodeHints hints) {
  Ref<Reader> reader(new MultiFormatReader);
  return vector<Ref<Result> >(1, reader->decode(image, hints));
}

vector<Ref<Result> > decode_multi(Ref<BinaryBitmap> image, DecodeHints hints) {
  MultiFormatReader delegate;
  GenericMultipleBarcodeReader reader(delegate);
  return reader.decodeMultiple(image, hints);
}

int read_image(Ref<LuminanceSource> source, bool hybrid, string expected) {
  vector<Ref<Result> > results;
  string cell_result;
  int res = -1;

  try {
    Ref<Binarizer> binarizer;
    if (hybrid) {
      binarizer = new HybridBinarizer(source);
    } else {
      binarizer = new GlobalHistogramBinarizer(source);
    }
    DecodeHints hints(DecodeHints::DEFAULT_HINT);
    hints.setTryHarder(try_harder);
    Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
    if (search_multi) {
      results = decode_multi(binary, hints);
    } else {
      results = decode(binary, hints);
    }
    res = 0;
  } catch (const ReaderException& e) {
    cell_result = "zxing::ReaderException: " + string(e.what());
    res = -2;
  } catch (const zxing::IllegalArgumentException& e) {
    cell_result = "zxing::IllegalArgumentException: " + string(e.what());
    res = -3;
  } catch (const zxing::Exception& e) {
    cell_result = "zxing::Exception: " + string(e.what());
    res = -4;
  } catch (const std::exception& e) {
    cell_result = "std::exception: " + string(e.what());
    res = -5;
  }

  if (test_mode && results.size() == 1) {
    std::string result = results[0]->getText()->getText();
    if (expected.empty()) {
      cout << "  Expected text or binary data for image missing." << endl
           << "  Detected: " << result << endl;
      res = -6;
    } else {
      if (expected.compare(result) != 0) {
        cout << "  Expected: " << expected << endl
             << "  Detected: " << result << endl;
        cell_result = "data did not match";
        res = -6;
      }
    }
  }

  if (res != 0 && (verbose || (use_global ^ use_hybrid))) {
    cout << (hybrid ? "Hybrid" : "Global")
         << " binarizer failed: " << cell_result << endl;
  } else if (!test_mode) {
    if (verbose) {
      cout << (hybrid ? "Hybrid" : "Global")
           << " binarizer succeeded: " << endl;
    }
    for (size_t i = 0; i < results.size(); i++) {
      if (more) {
        cout << "  Format: "
             << BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
             << endl;
        for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
          cout << "  Point[" << j <<  "]: "
               << results[i]->getResultPoints()[j]->getX() << " "
               << results[i]->getResultPoints()[j]->getY() << endl;
        }
      }
      if (verbose) {
        cout << "    ";
      }
      cout << results[i]->getText()->getText() << endl;
    }
  }

  return res;
}

string read_expected(string imagefilename) {
  string textfilename = imagefilename;
  string::size_type dotpos = textfilename.rfind(".");

  textfilename.replace(dotpos + 1, textfilename.length() - dotpos - 1, "txt");
  ifstream textfile(textfilename.c_str(), ios::binary);
  textfilename.replace(dotpos + 1, textfilename.length() - dotpos - 1, "bin");
  ifstream binfile(textfilename.c_str(), ios::binary);
  ifstream *file = 0;
  if (textfile.is_open()) {
    file = &textfile;
  } else if (binfile.is_open()) {
    file = &binfile;
  } else {
    return std::string();
  }
  file->seekg(0, ios_base::end);
  size_t size = size_t(file->tellg());
  file->seekg(0, ios_base::beg);

  if (size == 0) {
    return std::string();
  }

  char* data = new char[size + 1];
  file->read(data, size);
  data[size] = '\0';
  string expected(data);
  delete[] data;

  return expected;
}

std::string __read_image(Ref<LuminanceSource> source, string expected) {
  vector<Ref<Result> > results;
  string cell_result;
  int res = -1;
  try {
    Ref<Binarizer> binarizer;
    //if (hybrid) {
      //binarizer = new HybridBinarizer(source);
   // } else {
      binarizer = new GlobalHistogramBinarizer(source);
    //}
    DecodeHints hints(DecodeHints::DEFAULT_HINT);
    hints.setTryHarder(try_harder);
    Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
    if (search_multi) {
      results = decode_multi(binary, hints);
    } else {
      results = decode(binary, hints);
    }
    res = 0;
  } catch (const ReaderException& e) {
		return "";
  } catch (const zxing::IllegalArgumentException& e) {
    return "";
  } catch (const zxing::Exception& e) {
   return "";
  } catch (const std::exception& e) {
    return "";
  }

  if (test_mode && results.size() == 1) {
    std::string result = results[0]->getText()->getText();
    if (expected.empty()) {
     return "";
    } else {
      if (expected.compare(result) != 0) {
       return "";
      }
    }
  }

 
   std::string result ="";
    for (size_t i = 0; i < results.size(); i++) {    
		result.append(results[i]->getText()->getText());// << endl;
    }
  
	return result;
  
}

char* qrdecode(char* filename){

    Ref<LuminanceSource> source;
    try {
      source = ImageReaderSource::create(std::string(filename));
    } catch (const zxing::IllegalArgumentException &e) {
     return "";
    }

    string expected = read_expected(std::string(filename));
    std::string r = __read_image(source, expected);
	return (char*)r.c_str();
}



