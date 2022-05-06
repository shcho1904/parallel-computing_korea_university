#ifndef __project_sub_code__c__
#define __project_sub_code__c__

#include <stdio.h>
#include <stdlib.h>
#include "project_sub.h"

int cal_pixel(complex c)
{
	int count, max;
	complex z;
	float temp, lengthsq;
	max = 255;
	count = 0;
	z.real = 0; z.imag = 0; count = 0;
	do {
		temp = z.real * z.real - z.imag * z.imag + c.real;
		z.imag = 2 * z.real * z.imag + c.imag;
		z.real = temp;
		lengthsq = z.real * z.real + z.imag * z.imag;
		count++;
	} while (lengthsq < 4.0 && count < max);
	return count;
}



#endif
