#pragma once
#ifndef __project_sub__h__
#define __project_sub__h__

typedef struct _coordinate {
	int x;
	int y;
	int color;
} coop;

typedef struct complex {
	float real;
	float imag;
} complex;

int cal_pixel(complex c);


#endif