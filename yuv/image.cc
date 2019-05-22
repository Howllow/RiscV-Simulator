#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <cstdint>
#include <ctime>
#include <nmmintrin.h>
#include <immintrin.h>
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
	double timecnt;
	clock_t start, end;
	start = clock();
	for (int alpha = 1; alpha < 255; alpha = alpha + 3) {
	    for (int i = 0; i < width; i ++)
	        for (int j = 0; j < height; j ++) {
	            int index = i + j * width;
	            int uindex = width * height + (i >> 1) + (j >> 1) * (width >> 1);
	            int vindex = uindex + ((width * height) >> 2);
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
	end = clock();
	timecnt = (double)(end - start);
	cout << "Processing Time:" << timecnt / CLOCKS_PER_SEC << "s" << endl;
}

void sseYuv(char* yuv)
{
	double timecnt;
	clock_t start, end;
	start = clock();
		for (int alpha = 1; alpha < 255; alpha = alpha + 3) {
	    for (int i = 0; i < width; i ++)
	        for (int j = 0; j < height; j ++) {
	            int index = i + j * width;
	            int uindex = width * height + (i >> 1) + (j >> 1) * (width >> 1);
	            int vindex = uindex + ((width * height) >> 2);
				short y = (uint8_t)yuv[index];
				short u = (uint8_t)yuv[uindex];
				short v = (uint8_t)yuv[vindex];
				__m128i part1 = _mm_set_epi16(0, 0, u, y, u, y, u, y);
				__m128i op = _mm_set_epi16(0, 0, 128, 16, 128, 16, 128, 16);
				__m128i eight = _mm_set_epi64x(0, 8);
				part1 = _mm_sub_epi16(part1, op);
				op = _mm_set_epi16(0, 0, 519, 298, -101, 298, 0, 298);
				part1 = _mm_madd_epi16(part1, op);
				__m128i part2 = _mm_set_epi16(0, 0, 1, v, 1, v, 1, v);
				op = _mm_set_epi16(0, 0, 0, 128, 0, 128, 0, 128);
				part2 = _mm_sub_epi16(part2, op);
				op = _mm_set_epi16(0, 0, 83, 0, -429, -211, 32, 411);
				part2 = _mm_madd_epi16(part2, op);
				__m128i RGB = _mm_add_epi32(part1, part2);
				RGB = _mm_srl_epi32(RGB, eight); // get RGB
				op = _mm_set_epi32(0, alpha, alpha ,alpha);
				RGB = _mm_madd_epi16(op, RGB); // mul alpha
				RGB = _mm_srl_epi32(RGB, eight); // div 256
				int R = _mm_cvtsi128_si64(RGB) & 0x00000000;
				int G = int(_mm_cvtsi128_si64(RGB) >> 32);
				int B = _mm_cvtsi128_si64(_mm_shuffle_epi32(RGB, 0x36)) & 0x00000000;
				__m128i RG = _mm_set_epi16(0, 0, G, R, G, R, G, R);
				__m128i B1 = _mm_set_epi16(0, 0, 0, B, 0, B, 0, B);
				op = _mm_set_epi16(0, 0, -94, 112, -74, -38, 129, 66);
				RG = _mm_madd_epi16(RG, op);
				op = _mm_set_epi16(0, 0, 0, -18, 0, 112, 0, 25);
				B1 = _mm_madd_epi16(B1, op);
				op = _mm_set_epi32(0, 128, 128, 16);
				__m128i newyuv = _mm_srl_epi32(_mm_add_epi32(RG, B1), eight);
				newyuv = _mm_add_epi32(newyuv, op);
				output[cnt][index] = (char)(_mm_cvtsi128_si64(newyuv) & 0x00000000);
				output[cnt][uindex] = (char)(_mm_cvtsi128_si64(newyuv) >> 32);
				output[cnt][vindex] = (char)(_mm_cvtsi128_si64(_mm_shuffle_epi32(newyuv, 0x36)) & 0x00000000);
	        }
	        cnt++;
	    }
		end = clock();
		timecnt = (double)(end - start);
		cout << "Processing Time:" << timecnt / CLOCKS_PER_SEC << "s" << endl;
}

void mmxYuv(char* yuv)
{
	double timecnt;
	clock_t start, end;
	start = clock();
	for (int alpha = 1; alpha < 255; alpha = alpha + 3) {
	    for (int i = 0; i < width; i ++)
	        for (int j = 0; j < height; j ++) {
	            int index = i + j * width;
	            int uindex = width * height + (i >> 1) + (j >> 1) * (width >> 1);
	            int vindex = uindex + ((width * height) >> 2);
				short y = (uint8_t)yuv[index];
				short u = (uint8_t)yuv[uindex];
				short v = (uint8_t)yuv[vindex];
				__m64  part1 = _mm_set_pi16(v, u, y, 1);
				__m64  op = _mm_set_pi16(128, 128, 16, 0);
				__m64 eight = _mm_set_pi16(0, 0, 0, 8);
				part1 = _mm_sub_pi16(part1, op);
				op = _mm_set_pi16(411, 0, 298, 32);
                part1 = _mm_madd_pi16(part1, op);
                int R = (int(_mm_cvtm64_si64(part1) >> 32) + int(_mm_cvtm64_si64(part1) & 0x00000000)) >> 8;
                R *= alpha;
                R /= 255;
                part1 = _mm_set_pi16(y, u, y, u);
                __m64 part2 = _mm_set_pi16(v, 1, v, 1);
                op = _mm_set_pi16(16, 128, 16, 128);
                part1 = _mm_sub_pi16(part1, op);
                op = _mm_set_pi16(128, 0, 128, 0);
                part2 = _mm_sub_pi16(part2, op);
                op = _mm_set_pi16(298, -101, 298, 519);
                part1 = _mm_madd_pi16(part1, op);
                op = _mm_set_pi16(0, 83, -211, -429);
                part2 = _mm_madd_pi16(part2, op);
                part1 = _mm_add_pi32(part1, part2);
                part1 = _mm_srl_pi32(part1, eight);
                op = _mm_set_pi32(alpha, alpha);
                part1 = _mm_madd_pi16(op, part1);
                part1 = _mm_srl_pi32(part1, eight);
                int B = int((_mm_cvtm64_si64(part1)) >> 32);
                int G = int(_mm_cvtm64_si64(part1)) & 0x00000000;

                part1 = _mm_set_pi16(0, R, G, B);
                op = _mm_set_pi16(0, 66, 129, 25);
                part1 = _mm_madd_pi16(part1, op);
                int newy = (int(_mm_cvtm64_si64(part1) >> 32) + int(_mm_cvtm64_si64(part1) & 0x00000000)) >> 8;
                newy += 16;
                part1 = _mm_set_pi16(R, G, R, G);
                part2 = _mm_set_pi32(B, B);
                op = _mm_set_pi16(-32, -74, 112, -94);
                part1 = _mm_madd_pi16(part1, op);
                op = _mm_set_pi32(112, -18);
                part2 = _mm_madd_pi16(part2, op);
                part1 = _mm_add_pi32(part1, part2);
                part1 = _mm_srl_pi32(part1, eight);
                op = _mm_set_pi32(128, 128);
                part1 = _mm_add_pi32(part1, op);
                int newu = int(_mm_cvtm64_si64(part1) >> 32);
                int newv = int(_mm_cvtm64_si64(part1)) & 0x00000000;

                output[cnt][index] = (char)newy;
	            output[cnt][uindex] = (char)newu;
	            output[cnt][vindex] = (char)newv;
	        }
	        cnt++;
	    }
	end = clock();
	timecnt = (double)(end - start);
	cout << "Processing Time:" << timecnt / CLOCKS_PER_SEC << "s" << endl;
}

void avxYuv(char* yuv)
{
	double timecnt;
	clock_t start, end;
	start = clock();
	for (int alpha = 1; alpha < 255; alpha = alpha + 3) {
	    for (int i = 0; i < height; i ++)
	        for (int j = 0; j < width; j += 2) {
	            int index = j + i * width;
	            int uindex = width * height + (j >> 1) + (i >> 1) * (width >> 1);
	            int vindex = uindex + ((width * height) >> 2);
				short y = (uint8_t)yuv[index];
				short y1 = (uint8_t)yuv[index + 1];
				short u = (uint8_t)yuv[uindex];
				short v = (uint8_t)yuv[vindex];
				__m256 max = _mm256_set_ps(255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f, 255.0f);
				__m256 min = _mm256_set_ps(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
				__m256 fyuv = _mm256_set_ps(0.0f, v, u, y1, 0.0f, v, u, y);
				__m256 op = _mm256_set_ps(0.0f, 128.0f, 128.0f, 16.0f, 0.0f, 128.0f, 128.0f, 16.0f);
				fyuv = _mm256_sub_ps(fyuv, op);
				op = _mm256_set_ps(0.0f, 1.596027f, 0.0f, 1.164383f, 0.0f, 1.596027f, 0.0f, 1.164383f);
				__m256 R = _mm256_dp_ps(fyuv, op, 0b11110001);
				op = _mm256_set_ps(0.0f, -0.812968f, -0.391762f, 1.164383f, 0.0f, -0.812968f, -0.391762f, 1.164383f);
				__m256 G = _mm256_dp_ps(fyuv, op, 0b11110010);
				op = _mm256_set_ps(0.0f, 0.0f, 2.017232f, 1.164383f, 0.0f, 0.0f, 2.017232f, 1.164383f);
				__m256 B = _mm256_dp_ps(fyuv, op, 0b11110100);
				__m256 RGB = _mm256_min_ps(_mm256_max_ps(_mm256_add_ps(_mm256_add_ps(R, G), B), min), max);
				op = _mm256_set_ps(0.0f, alpha, alpha, alpha, 0.0f, alpha, alpha, alpha);
				op = _mm256_div_ps(op, max);
				RGB = _mm256_mul_ps(RGB, op);
				op = _mm256_set_ps(0.0f, 0.097906f, 0.504129f, 0.256788f, 0.0f, 0.097906f, 0.504129f, 0.256788f);
				__m256 newy = _mm256_dp_ps(RGB, op, 0b11110001);
				op = _mm256_set_ps(0.0f, 0.439126f, -0.290993f, -0.148223f, 0.0f, 0.439126f, -0.290993f, -0.148223f);
				__m256 newu = _mm256_dp_ps(RGB, op, 0b11110010);
				op = _mm256_set_ps(0.0f, -0.071427f, -0.367788f, 0.439216f, 0.0f, -0.071427f, -0.367788f, 0.439216f);
				__m256 newv = _mm256_dp_ps(RGB, op, 0b11110100);
				op = _mm256_set_ps(0.0f, 128.0f, 128.0f, 16.0f, 0.0f, 128.0f, 128.0f, 16.0f);
				fyuv = _mm256_add_ps(_mm256_add_ps(newy, _mm256_add_ps(newu, newv)), op);
				__m256i iyuv = _mm256_cvtps_epi32(fyuv);
				output[cnt][index] = _mm256_extract_epi32(iyuv, 0);
				output[cnt][uindex] = _mm256_extract_epi32(iyuv, 1);
				output[cnt][vindex] = _mm256_extract_epi32(iyuv, 2);
				output[cnt][index + 1] = _mm256_extract_epi32(iyuv, 3);	 
				
	        }
	        cnt++;
	    }
	end = clock();
	timecnt = (double)(end - start);
	cout << "Processing Time:" << timecnt / CLOCKS_PER_SEC << "s" << endl;
}

void idealYuv(char* yuv)
{
	double timecnt;
	clock_t start, end;
	start = clock();
	for (int alpha = 1; alpha < 255; alpha = alpha + 3) {
	    for (int i = 0; i < width; i ++)
	        for (int j = 0; j < height; j ++) {
	            int index = i + j * width;
	            int uindex = width * height + (i >> 1) + (j >> 1) * (width >> 1);
	            int vindex = uindex + ((width * height) >> 2);
	            int y = (uint8_t)yuv[index];
	            int u = (uint8_t)yuv[uindex];
	            int v = (uint8_t)yuv[vindex];
	            int R = (298 * (y - 16) + 411 * (v - 128) + 32) >> 8;
	            R = alpha * R / 255;
	            int B = (298 * (y - 16) + 519 * (u - 128) + 83) >> 8;
	            B = alpha * B / 255;
	            int G = (298 * (y - 16) - 101 * (u - 128) - 211 * (v - 128)  - 429) >> 8;
	            G = alpha * G / 255;
	            int newy = ((66 * R + 129 * G + 25 * B ) >> 8) + 16;
	        	int newu = ((-38 * R - 74 * G + 112 * B) >> 8) + 128;
	            int newv = ((112 * R - 94 * G - 18 * B) >> 8) + 128;
	            output[cnt][index] = (char)newy;
	            output[cnt][uindex] = (char)newu;
	            output[cnt][vindex] = (char)newv;
	        }
	        cnt++;
	    }
	end = clock();
	timecnt = (double)(end - start);
	cout << "Processing Time:" << timecnt / CLOCKS_PER_SEC << "s" << endl;
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
	else if (isa == "int")
		idealYuv(yuv);
    else if (isa == "sse")
		sseYuv(yuv);
    else if (isa == "mmx")
        mmxYuv(yuv);
	else if (isa == "avx")
		avxYuv(yuv);
    if (do_out) {
		printf("doing output!\n");
	    for (int i = 0; i < cnt; i++) {
	    	ofstream os(string("./") + isa + filename + "/" + string("a") + to_string(i) + string(".yuv"));
	        os.write(output[i], length);
		}
	}
	for (int i = 0; i < 85; i++)
		delete output[i];
	delete yuv;
    return 0;
}
