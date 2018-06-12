#include "common.h"
#include "data.h"

static char inStr[3] = {'_', 'x', 'o'};
static double in[3] = {0., 1/3., 2/3.};
static char outStr[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
static double out[9] = {0., 1., 2., 3., 4., 5., 6., 7., 8.};

double inStrToFl(char str) {
	switch (str) {
		case '_': return 1/6.;
		case 'x': return 1/2.;
		case 'o': return 5/6.;
		default: return 0.;
	}
	/*
	// assume in and inStr have the same size
	char last_pos = ARRAY_SIZE(inStr)-1;
	if (str == inStr[last_pos])
		return (in[last_pos]+1)/2;

	for (int i=0; i<last_pos; i++) {
		if (str == inStr[i])
			return (in[i] + in[i+1])/2; //take the average of the interval
	}
	printf("inStrToFl Error\n");
	return 0.;*/
}

char inFlToStr(double v) {
	for (int i = ARRAY_SIZE(inStr)-1; i>-1; i--) {
		if (v >= in[i])
			return inStr[i];
	}
	printf("inFlToStr Error\n");
	return '\0';
}

double outStrToFl(char str) {
	for (int i=0; i<ARRAY_SIZE(outStr); i++) {
		if (str == outStr[i])
			return out[i];
	}
	printf("outStrToFl Error\n");
	return 0.;
}

char outFlToStr(double v) {
	for (int i=ARRAY_SIZE(out); i>-1; i--) {
		if (v >= out[i])
			return outStr[i];
	}
	printf("outFlToStr Error\n");
	return '\0';
}