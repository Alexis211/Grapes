#include <stdlib.h>

void printk_int(int number) {
	if (number == 0) {
		printk("0");
		return;
	}
	int negative = 0;
	if (number < 0) {
		negative = 1;
		number = 0 - number;
	}
	int order = 0, temp = number, i;
	char numbers[] = "0123456789";
	while (temp > 0) {
		order++;
		temp /= 10;
	}

	char *s, *r;
	s = malloc(order + (negative ? 2 : 1));
	if (negative) {
		s[0] = '-';
		r = s + 1;
	} else {
		r = s;
	}

	for (i = order; i > 0; i--) {
		r[i - 1] = numbers[number % 10];
		number /= 10;
	}
	r[order] = 0;
	printk(s);
	free(s);
}

void printk_hex(unsigned v) {
	char s[11] = {'0', 'x', 0};

	int i;

	char hexdigits[] = "0123456789ABCDEF";

	for (i = 0; i < 8; i++) {
		s[i + 2] = (hexdigits[v >> 28]);
		v = v << 4;
	}
	s[11] = 0;
	printk(s);
}
