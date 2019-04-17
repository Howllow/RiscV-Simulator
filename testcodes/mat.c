#include <stdio.h>
int main(){
	double start, finish;
	int A[100][100];
	for (int i = 0; i < 100; i++)
		for (int j = 0; j < 100; j++) {
			A[i][j] = i;
		}
	int B[100][100];
	for (int i = 0; i < 100; i++){
		for (int j = 0; j < 100; j++) {
			for (int k = 0; k < 100; k++) {
				B[i][j] = B[i][j] + A[i][k] * A[k][j];
			}
		}
	}
	return 0;
}