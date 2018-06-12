#ifndef BRAIN_H_DEF
#define BRAIN_H_DEF

#include "layer.h"
#include "brain.h"

struct brain {
	struct layer** layer;
	int inputSize;
	int innerSize;
	int outputSize;
	int layerSize;

	//learning
	int inDataLength;
	double** inData;
	int outDataLength;
	double** outData;
	double error;
};

struct brain* brain_new(int inSize, int innSize, int outSize, int layerSize);
void brain_free(struct brain* b);
void brain_learn(struct brain* b, char* dir);
void brain_loadData(struct brain* b, char* dir);
void brain_run(struct brain* b, double* startValue);
int brain_outToInt(struct brain *b);
int brain_outDataToInt(struct brain *b, int s);
void brain_backPropagation(struct brain* b, double* target);
void brain_changeVar(struct brain* b);
void brain_reset(struct brain *b);
struct brain* brain_load(char* dir);
void brain_save(struct brain* b, char* dir);

#endif
