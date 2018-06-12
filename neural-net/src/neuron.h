#ifndef NEURON_H_DEF
#define NEURON_H_DEF

struct neuron {
	int startNeuron;
	double input;
	double output;
	int wSize;
	double* w;
	double* wVar;
	double* wVarTotal;
	double b;
	double bVar;
	double bVarTotal;
};

struct neuron* neuron_new(int wSize);
void neuron_free(struct neuron *n);
void neuron_calculation(struct neuron* n);
void neuron_varChange(struct neuron* n, int d);

#endif