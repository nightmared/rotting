#include "common.h"
#include "data.h"
#include "brain.h"
#include <time.h> // for timing the training process

struct brain* brain_new(int inSize, int innSize, int outSize, int layerSize) {
	struct brain* b = malloc(sizeof(struct brain));
	if (b == NULL)
		exit(1);
	b->layer = malloc(sizeof(void*)*layerSize);
	if (b->layer == NULL)
		exit(1);
    b->inData = NULL;
    b->outData = NULL;
	b->inputSize = inSize;
	b->innerSize = innSize;
	b->outputSize = outSize;
	b->layerSize = layerSize;
	b->error = 0;

	b->layer[0] = layer_from_input(inSize, innSize);
	for (int i=1; i<layerSize-2; i++) {
		b->layer[i] = layer_new(innSize, b->layer[i-1]);
	}
	b->layer[layerSize-2] = layer_new(outSize, b->layer[layerSize-3]);
	b->layer[layerSize-1] = layer_new(0, b->layer[layerSize-2]);
	return b;
}

void brain_free(struct brain* b) {
	for (int i = 0; i < b->layerSize; i++) {
		layer_free(b->layer[i]);
		free(b->layer[i]);
	}
	free(b->layer);
    if (b->inData != NULL) {
        for (int i = 0; i < b->inDataLength; i++) {
            free(b->inData[i]);
        }
        free(b->inData);
    }
    if (b->outData != NULL) {
        for (int i = 0; i < b->outDataLength; i++) {
            free(b->outData[i]);
        }

        free(b->outData);
    }
}

void brain_learn(struct brain* b, char* dir) {
	brain_loadData(b, dir);
	// learn

	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	double eps = (double)(b->outputSize*b->inDataLength)/(2.*data_out_length);
	double lastError = eps+1;
	int batchNumber = b->inputSize/(batchSize*1.0)+1;
	printf("batch number: %i\n", batchNumber);
	b->error = eps+1;
	printf("Start learning with a learning rate of: %f\nDataSet size: %i\nEPS = %f\n", learnRate, b->inDataLength, eps);
	int c = 0;
	while (b->error > eps) {	// learn loop
		if (c%30000== 1) {
			printf("Error: %f - learnRate: %f\n", b->error, learnRate);
		}
		b->error = 0;
		for (int i=0; i<batchNumber; i++) {	// batch itr
			int s;
			int d = batchSize*(i%batchNumber);
			for (s=d; s<b->inDataLength && s<d+batchSize; s++) {
				brain_run(b, b->inData[s]);
				brain_backPropagation(b, b->outData[s]);
			}
			if (s != d) {
				// weight and biases changes
				for (int l=0; l<b->layerSize-1; l++) {
					layer_varChange(b->layer[l], s-d);
				}
			}
		}
		if (c%1000 == 1) {
			if (lastError == b->error) {
				// the error stagnates, time to break the loop
				break;
			}
			lastError = b->error;
		}
		c++;
	}

	struct timespec stop;
	clock_gettime(CLOCK_MONOTONIC, &stop);

	printf("Final Error: %f\n", b->error);
	printf("It took locally %f ms\n", (double)(stop.tv_sec-start.tv_sec)*.001/c);
	printf("It took %f s and %i dataset iterations\n", (double)(stop.tv_sec-start.tv_sec), (int)c);
	int test = 0;
	for (int s=0; s<b->inDataLength; s++) {
		brain_run(b, b->inData[s]);
		if (brain_outToInt(b) == brain_outDataToInt(b, s)) {
			test++;
		}
	}
	printf("%f%% de reussite\n", test*100./b->inDataLength);
}

void brain_run(struct brain* b, double* startValue) {
	brain_reset(b); // make all input equal to 0
	for (int i=0; i<b->inputSize; i++) {
		b->layer[0]->inNeuron[i]->input = startValue[i]; // set start values
	}
	for (int l=0; l<b->layerSize; l++) {
		layer_run(b->layer[l]);
	}
}

int brain_outToInt(struct brain *b) {
	double m = b->layer[b->layerSize-1]->inNeuron[0]->output;
	int index = 0;
	for (int i = 0; i<b->outputSize; i++) {
		int tmp = b->layer[b->layerSize-1]->inNeuron[i]->output;
		if (tmp > m) {
			m = tmp;
			index = i;
		}
	}
	return index;
}

int brain_outDataToInt(struct brain *b, int s) {
	for (int h = 0; h < b->outputSize; h++) {
		if (b->outData[s][h] == 1) {
			return h;
		}
	}
	return 0;
}

void brain_backPropagation(struct brain* b, double* target) {
	// first iteration weights
	int l = b->layerSize-2; // before last layer
	for (int i=0; i<b->layer[l]->inLength; i++) {
		for (int j=0; j<b->layer[l]->outLength; j++) {
			double neuron_out = b->layer[l]->outNeuron[j]->output;
			struct neuron* ni = b->layer[l]->inNeuron[i];
			b->error += abs(target[j] - neuron_out);
			ni->wVar[j] = (neuron_out - target[j])*neuron_out*(1-neuron_out)*ni->output;
			ni->wVarTotal[j] += ni->wVar[j];
		}
	}
	// first iteration biases
	l = b->layerSize-1;
	for (int i=0; i<b->layer[l]->inLength; i++) {
		struct neuron* ni = b->layer[l]->inNeuron[i];
		ni->bVar = (ni->output - target[i])*ni->output*(1-ni->output);
		ni->bVarTotal += ni->bVar;
	}
	// recurring iteration
	for (l=b->layerSize-3; l>=0; l--) {
		layer_backPropW(b->layer[l]);
	}
	for (l=b->layerSize-2; l>=0; l--) {
		layer_backPropB(b->layer[l]);
	}
}

void brain_changeVar(struct brain* b) {
	srand(time(NULL));
	for (int l=0; l<b->layerSize; l++) {
		for (int i=0; i<b->layer[l]->inLength; i++) {
			for (int j=0; j<b->layer[l]->outLength; j++) {
				b->layer[l]->inNeuron[i]->w[j] = RANDOM_WEIGHT();
			}
			b->layer[l]->inNeuron[i]->b = RANDOM_WEIGHT();
		}
	}
}

void brain_reset(struct brain *b) {
	for (int l=0; l<b->layerSize; l++) {
		layer_reset(b->layer[l]);
	}
}

void brain_loadData(struct brain* b, char* dir) {
	struct LinesArray lines = readFileToLinesArray(dir);
	b->inData = malloc(lines.number/2*sizeof(void*));
	b->outData = malloc(lines.number/2*sizeof(void*));
	b->inDataLength = lines.number/2;
	b->outDataLength = lines.number/2;
	if (b->inData == NULL || b->outData == NULL)
		exit(1);
	for (int i = 0; i < lines.number/2; i++) {
		b->inData[i] = malloc(sizeof(double)*b->inputSize);
		b->outData[i] = malloc(sizeof(double)*b->outputSize);
		if (b->inData[i] == NULL || b->outData[i] == NULL)
			exit(1);
	}

	for (int i=0; i<lines.number/2; i++) {
		for (int j=0; j<b->inputSize; j++) {
			if (strlen(lines.lines[2*i]) != b->inputSize) {
				printf("Length Error at line %i\n", 2*i);
				exit(1);
			}
			b->inData[i][j]=inStrToFl(lines.lines[2*i][j]);
		}
		for (int j=0; j<b->outputSize; j++) {
			if (outStrToFl(lines.lines[2*i+1][0]) == j) {
				b->outData[i][j] = 1;
			} else {
				b->outData[i][j] = 0;
			}
		}
	}
	LinesArray_free(&lines);
}

struct brain* brain_load(char* dir) {
	struct LinesArray lines = readFileToLinesArray(dir);
	char** cuts = cut_on_delim(lines.lines[0], '/');
	int inputSize = atoi(cuts[0]);
	int innerSize = atoi(cuts[1]);
	int outputSize = atoi(cuts[2]);
	int layerSize = atoi(cuts[3]);
	struct brain* b = brain_new(inputSize, innerSize, outputSize, layerSize);

	for (int i=0; i<inputSize; i++) { // set all inputNeuron weights and biases
		char** lineN = cut_on_delim(lines.lines[i+1], '/');
		for (int j=0; j<innerSize; j++) {
			b->layer[0]->inNeuron[i]->w[j] = strtod(lineN[j], NULL); // weight
		}
		b->layer[0]->inNeuron[i]->b = strtod(lineN[innerSize], NULL); // bias
		free(lineN);
	}
	for (int l=1; l<layerSize-2; l++) {	//set all innerNeuron weights biases
		for (int i=0; i<innerSize; i++) {
			char** lineN = cut_on_delim(lines.lines[1+inputSize+i+(l-1)*innerSize], '/');
			for (int j=0; j<innerSize; j++) {
				b->layer[l]->inNeuron[i]->w[j] = strtod(lineN[j], NULL);	//weigth
			}
			b->layer[l]->inNeuron[i]->b = strtod(lineN[innerSize], NULL);	//bias
			free(lineN);
		}
	}
	for (int i=0; i<innerSize; i++) {	//set all outputNeuron weights biases
		char** lineN = cut_on_delim(lines.lines[1+inputSize+innerSize*(layerSize-3)+i], '/');
		for (int j=0; j<outputSize; j++) {
			b->layer[layerSize-2]->inNeuron[i]->w[j] = strtod(lineN[j], NULL);	//weight
		}
		b->layer[layerSize-2]->inNeuron[i]->b = strtod(lineN[outputSize], NULL);	//bias
		free(lineN);
	}
	char** lineN = cut_on_delim(lines.lines[1+inputSize+innerSize*(layerSize-2)], '/');
	for (int j=0; j<outputSize; j++) {
		b->layer[layerSize-1]->inNeuron[j]->b = strtod(lineN[j], NULL);	//weight
	}
	free(lineN);
	free(cuts);
	LinesArray_free(&lines);
    return b;
}

void brain_save(struct brain* b, char* dir) {
	int len = 1+b->inputSize+b->innerSize*(b->layerSize-2)+b->outputSize;
	int size = 4096;
	char* lines = malloc(size*sizeof(void*));
	if (lines == NULL)
		exit(1);

	// the size is arbitrary !!!
	char buf[50];
	sprintf(buf, "%i/%i/%i/%i\n", b->inputSize, b->innerSize, b->outputSize, b->layerSize);
	size=append_text(lines, buf, size);

	for (int i=0; i<b->inputSize; i++) {		//set all inputNeuron weights biases
		for (int j=0; j<b->innerSize; j++) {
			sprintf(buf, "%f/", b->layer[0]->inNeuron[i]->w[j]); // weight
			size=append_text(lines, buf, size);
		}
		sprintf(buf, "%f/\n", b->layer[0]->inNeuron[i]->b); // bias
		size=append_text(lines, buf, size);
	}
	for (int l=1; l<b->layerSize-2; l++) {	//set all innerNeuron weights biases
		for (int i=0; i<b->innerSize; i++) {
			for (int j=0; j<b->innerSize; j++) {
				sprintf(buf, "%f/", b->layer[l]->inNeuron[i]->w[j]); // weight
				size=append_text(lines, buf, size);
			}
			sprintf(buf, "%f/\n", b->layer[l]->inNeuron[i]->b); // bias
			size=append_text(lines, buf, size);
		}
	}
	for (int i=0; i<b->innerSize; i++) {	//set all outputNeuron weights
		for (int j=0; j<b->outputSize; j++) {
			sprintf(buf, "%f/", b->layer[b->layerSize-2]->inNeuron[i]->w[j]); // weight
			size=append_text(lines, buf, size);
		}
		sprintf(buf, "%f/\n", b->layer[b->layerSize-2]->inNeuron[i]->b); // bias
		size=append_text(lines, buf, size);
	}
	for (int j=0; j<b->outputSize; j++) {
		sprintf(buf, "%f/", b->layer[b->layerSize-1]->inNeuron[j]->b); // bias last layer
		size=append_text(lines, buf, size);
	}
	writeStringToFile(dir, lines);
}
