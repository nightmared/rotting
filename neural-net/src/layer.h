#ifndef LAYER_H_DEF
#define LAYER_H_DEF

#include "neuron.h"

struct layer {
	int inLength;
	int outLength;
	struct neuron** inNeuron;
	struct neuron** outNeuron;
};

#include "common.h"
#include "neuron.h"
#include "layer.h"

struct layer* layer_new(int outSize, struct layer *prevLayer);
struct layer* layer_from_input(int inSize, int innSize);
void layer_free(struct layer *l);
void layer_run(struct layer* l);
void layer_reset(struct layer* l);
void layer_backPropW(struct layer *l);
void layer_backPropB(struct layer *l);
void layer_varChange(struct layer *l, int d);

#endif