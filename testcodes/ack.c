#include "print.h"
int ack(int m, int n)
{
	if (m == 0)
		return n + 1;
	else if (n == 0)
		return ack(m - 1, 1);
	else return ack(m - 1, ack(m, n - 1));
}
int main() {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			int x = ack(i, j);
			PrintChar('A');
			PrintChar('(');
			PrintInt(i);
			PrintChar(',');
			PrintInt(j);
			PrintChar(')');
			PrintChar('=');
			PrintInt(x);
			PrintChar('\n');
		}
	return 0;
}