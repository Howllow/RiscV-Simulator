#include <stdio.h>
#include <stdlib.h>

int partitionsed (int a[], int left, int right)
{
	int pivot = a[left];
	int i = left,j = right;
	int k;
	while (i < j)
	{
		while (i < j && a[j] >= pivot)
			j--;
		if (i < j)
			a[i] = a[j];
		while (i < j && a[i] < pivot)
			i++;
		if (i < j)
			a[j] = a[i];	
	}
	a[i] = pivot;
        return i;
}

void quicksort(int a[],int left,int right)
{
	if (left < right)
	{
		int insert = partitionsed(a,left,right);
		quicksort (a,left,insert-1);
		quicksort (a,insert+1, right);
	}
}

int main() {
	double start, finish;
	int a[100000];
	for (int i = 0; i < 700; i++) {
		a[700 - i - 1] = i;
	}
	quicksort(a, 0, 700 - 1);
	return 0;
}


