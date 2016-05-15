
#include "Windows.h"
#include "library.h"


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

std::string qrdecode(std::string filename){

    Ref<LuminanceSource> source;
    try {
      source = ImageReaderSource::create(filename);
    } catch (const zxing::IllegalArgumentException &e) {
     return "";
    }

    string expected = read_expected(filename);
    return __read_image(source, expected);
}

void _get_command_path(const char *name,char *output){

	int len		= GetEnvironmentVariable("PATH",NULL,0)+1;
	char *var	= new char[len];
	GetEnvironmentVariable("PATH",var,len);

	char *temp;
	char *start,*var_begin;
	start		= var;
	var_begin	= var;
	char t[MAX_PATH];

	while(temp=strchr(var_begin,';')){
		long len_temp	= temp-start;
		long _len_temp	= len_temp+sizeof("\\")+sizeof(".exe")+1;
		
		memset(t,0,sizeof(t));
		
		strncpy_s(t,_len_temp,var_begin,len_temp);

	   // if(t[len-1]=='\\')
		//	t[len-1]='\0';
		//else 
		//	t[len]='\0';

		memset(output,0,sizeof(t));
		sprintf_s(output,MAX_PATH,"%s\\%s.exe\0",t,name);
		if(PathFileExists(output)){
			free(var);
		
			return;
		}

		memset(output,0,sizeof(t));
	sprintf_s(output,MAX_PATH,"%s\\%s.bat\0",t,name);

		if(PathFileExists(output)){
			free(var);
			
			return;
		}
		var_begin	= temp+1;
		start		= temp+1;
	}
	free(var);
	return;
}