#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdint>
using namespace std;

#define width 1920
#define height 1080

int length;
char *output[85];
int cnt;

char* readImage(const char* path)
{
    // load the image
    ifstream is(path, ifstream::in);
    is.seekg(0, is.end);
    length = is.tellg();
    is.seekg(0, is.beg);
    char* buffer = new char[length];
    is.read(buffer, length);
    return buffer;
}

void fdealYuv(char* yuv)
{
	for (int alpha = 1; alpha < 255; alpha = alpha + 3) {
	    for (int i = 0; i < width; i ++)
	        for (int j = 0; j < height; j ++) {
	            int index = i + j * width;
	            int uindex = width * height + (i / 2) + (j / 2) * (width / 2);
	            int vindex = uindex + (width * height) / 4;
	            int y = (uint8_t)yuv[index];
	            int u = (uint8_t)yuv[uindex];
	            int v = (uint8_t)yuv[vindex];
	            int R = max(min(1.164383 * (y - 16) + 1.596027 * (v - 128), 255.0), 0.0);
	            R = alpha * R / 255.0;
	            int B = max(min(1.164383 * (y - 16) + 2.017232 * (u - 128), 255.0), 0.0);
	            B = alpha * B / 255.0;
	            int G = max(min(1.164383 * (y - 16) - 0.391762 * (u - 128) - 0.812968 * (v - 128), 255.0), 0.0);
	            G = alpha * G / 255.0;
	            int newy = 0.256788 * R + 0.504129 * G + 0.097906 * B + 16;
	            int newu = -0.148223 * R - 0.290993 * G + 0.439216 * B + 128;
	            int newv = 0.439216 * R - 0.367788 * G - 0.071427 * B + 128;
	            output[cnt][index] = (char)newy;
	            output[cnt][uindex] = (char)newu;
	            output[cnt][vindex] = (char)newv;
	        }
	        cnt++;
	    }
}

int main(int argc, char** argv)
{
	bool do_out = false;
	string isa;
	
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1])
			{
				case 'i':
					isa = argv[i + 1];
					break;
				case 'o':
					do_out = true;
					break;
				default:
					break;
			}
		}
	}
	
    char* yuv;
    cnt = 0;
    int x = 0;
    string filename = string("dem1");
    string filepath = filename + string(".yuv");
    yuv = readImage(filepath.data());
    for (int i = 0; i < 85; i++)
    	output[i] = new char[length];
    if (isa == "float")
    	fdealYuv(yuv);
    
    if (do_out) {
	    for (int i = 0; i < cnt; i++) {
	    	ofstream os(string("./") + isa + filename + "/" + string("a") + to_string(i) + string(".yuv"));
	        os.write(output[i], length);
		}
	}
	delete output;
	delete yuv;
    return 0;
}
