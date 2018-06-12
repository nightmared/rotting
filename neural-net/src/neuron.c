#include "common.h"
#include "neuron.h"
#include <math.h> // for exp()

struct neuron* neuron_new(int wSize) {
	struct neuron *n = malloc(sizeof(struct neuron));
	if (n == NULL)
		exit(1);
	n->w = malloc(sizeof(double)*wSize);
	n->wVar = malloc(sizeof(double)*wSize);
	n->wVarTotal = malloc(sizeof(double)*wSize);
	n->wSize = wSize;
	if (n->w == NULL || n->wVar == NULL || n->wVarTotal == NULL)
		exit(1);

	for (int i=0; i<wSize; i++) {
		n->w[i] = RANDOM_WEIGHT(); //random weights
	}
	n->b = RANDOM_WEIGHT();
	n->bVar = 0;
	n->bVarTotal = 0;
	n->input = 0;
	n->output = 0;
	n->startNeuron = 0;
	return n;
}

void neuron_free(struct neuron *n) {
	free(n->w);
	free(n->wVar);
	free(n->wVarTotal);
}

double sigmoid(double x) {
	return 1/(1+exp(-x));
}

void neuron_calculation(struct neuron* n) {
	if (!n->startNeuron)
		n->output = sigmoid(n->input+n->b);
}

void neuron_varChange(struct neuron* n, int d) {
	for (int j=0; j<n->wSize; j++) {
			n->w[j] -= learnRate/d*n->wVarTotal[j];
			n->wVarTotal[j] = 0;
	}
	n->b -= learnRate/d*n->bVarTotal;
	n->bVarTotal = 0;
}
