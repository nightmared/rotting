#include "common.h"
#include "neuron.h"
#include "layer.h"

struct layer* layer_new(int outSize, struct layer *prevLayer) {
	struct layer* l = malloc(sizeof(struct layer));
	if (l == NULL)
		exit(1);

	// at the moment, we use the same input and output size for neurons 'in the middle'
	l->inLength = prevLayer->outLength;
	l->outLength = outSize;

	l->inNeuron = malloc(l->inLength*sizeof(void*));
	l->outNeuron = malloc(l->outLength*sizeof(void*));
	if (l->inNeuron == NULL || l->outNeuron == NULL)
		exit(1);

	for (int i=0; i<l->inLength; i++) {
		l->inNeuron[i] = neuron_new(outSize);
	}
	for (int i=0; i<l->inLength; i++) {
		prevLayer->outNeuron[i] = l->inNeuron[i];
	}
	return l;
}

struct layer* layer_from_input(int inSize, int innSize) {
	struct layer* l = malloc(sizeof(struct layer));
	if (l == NULL)
		exit(1);

	l->inLength = inSize;
	l->outLength = innSize;

	l->inNeuron = malloc(inSize*sizeof(void*));
	l->outNeuron = malloc(innSize*sizeof(void*));
	if (l->inNeuron == NULL)
		exit(1);

	for (int i=0; i<inSize; i++) {
		l->inNeuron[i] = neuron_new(innSize);
		l->inNeuron[i]->startNeuron = 1; // an input neuron don't need to do any calculation
	}
	return l;
}

void layer_free(struct layer *l) {
	// free all the neurons of the layer
	for (int i=0; i<l->inLength; i++) {
		free(l->inNeuron[i]);
	}
	free(l->inNeuron);
	free(l->outNeuron);
}

void layer_run(struct layer* l) {
	for (int i=0; i<l->inLength; i++) {
		neuron_calculation(l->inNeuron[i]); // do the calculation for the output (sig(input))
		for (int j=0; j<l->outLength; j++) {
			l->outNeuron[j]->input += l->inNeuron[i]->output*l->inNeuron[i]->w[j]; // output balanced by the weights
		}
	}
}

void layer_reset(struct layer* l) {
	for (int i=0; i<l->inLength; i++) {
		l->inNeuron[i]->input = 0;
	}
}

void layer_backPropW(struct layer *l) {
	for (int i=0; i<l->inLength; i++) {
		for (int j=0; j<l->outLength; j++) {
			l->inNeuron[i]->wVar[j] = 0;
			for (int k=0; k<l->outNeuron[j]->wSize; k++) {
				l->inNeuron[i]->wVar[j] += l->outNeuron[j]->w[k]*l->outNeuron[j]->wVar[k]; // output balanced by the weights
			}
			l->inNeuron[i]->wVar[j] *= l->inNeuron[i]->output*(1-l->outNeuron[j]->output);
			l->inNeuron[i]->wVarTotal[j] += l->inNeuron[i]->wVar[j];
		}
	}
}

void layer_backPropB(struct layer *l) {
	for (int i=0; i<l->inLength; i++) {
		l->inNeuron[i]->bVar = 0;
		for (int k=0; k<l->outLength; k++) {
			l->inNeuron[i]->bVar += l->inNeuron[i]->w[k] * l->outNeuron[k]->bVar;
		}
		l->inNeuron[i]->bVar *= l->inNeuron[i]->output*(1-l->inNeuron[i]->output);
		l->inNeuron[i]->bVarTotal += l->inNeuron[i]->bVar;
	}
}

void layer_varChange(struct layer *l, int d) {
	for (int i=0; i<l->inLength; i++) {
		neuron_varChange(l->inNeuron[i], d);
	}
}
