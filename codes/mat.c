#include <stdio.h>
int main(){
	double start, finish;
	int A[1000][1000];
	for (int i = 0; i < 1000; i++)
		for (int j = 0; j < 1000; j++) {
			A[i][j] = i;
		}
	int B[1000][1000];
	for (int i = 0; i < 1000; i++){
		for (int j = 0; j < 1000; j++) {
			for (int k = 0; k < 1000; k++) {
				B[i][j] = B[i][j] + A[i][k] * A[k][j];
			}
		}
	}
	return 0;
}