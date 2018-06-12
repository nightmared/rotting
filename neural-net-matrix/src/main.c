#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define DIE_ERROR(str) { \
	fprintf(stderr, str); \
	fprintf(stderr, "\n"); \
	exit(1); \
}

#define rand_double(span) (((double)rand())/RAND_MAX - 0.5)*span
#define rand_int(span) ((uint64_t)rand()*span)/RAND_MAX

struct matrix {
	uint32_t lines;
	uint32_t cols;
	double* data;
};

struct matrix matrix_new(uint32_t lines, uint32_t cols) {
	double* data = calloc(1, lines*cols*sizeof(double));
	if (data == NULL)
		DIE_ERROR("Failed to allocate memory !")

	struct matrix res = { lines, cols, data };
	return res;
}

void matrix_destroy(struct matrix* m) {
	m->lines = 0;
	m->cols = 0;
	free(m->data);
}

// check wether m[i][j] exist and exit otherwise
void check_access(struct matrix *m, uint32_t line, uint32_t col) {
	if (m == NULL)
		DIE_ERROR("Null argument !")

	if (line > m->lines || col > m->cols)
		DIE_ERROR("Out of bound access in the matrix !")
}

double matrix_get(struct matrix *m, uint32_t line, uint32_t col) {
	check_access(m, line, col);
	return m->data[line*m->cols+col];
}

void matrix_set(struct matrix *m, uint32_t line, uint32_t col, double value) {
	check_access(m, line, col);
	m->data[line*m->cols+col] = value;
}

struct matrix matrix_product(struct matrix *m1, struct matrix *m2) {
	if (m1 == NULL || m2 == NULL)
		DIE_ERROR("Null argument !")

	// Does a multiplication make sense ?
	if (m1->cols != m2->lines)
		DIE_ERROR("The two matrices cannot be multiplied together !")

	struct matrix res = matrix_new(m1->lines, m2->cols);
	for(uint32_t i = 0; i < res.lines; i++) {
		for(uint32_t j = 0; j < res.cols; j++) {
			double tmp = 0.0;
			for (uint32_t k = 0; k < m1->cols; k++) {
				tmp += matrix_get(m1, i, k) * matrix_get(m2, k, j);
			}
			// (A*B)[i][j] = sum_k=0 to m_(A[i][k]*B[k][j])
			matrix_set(&res, i, j, tmp);
		}
	}
	return res;
}

struct matrix matrix_hadamard_product(struct matrix *m1, struct matrix *m2) {
	if (m1 == NULL || m2 == NULL)
		DIE_ERROR("Null argument !")

	// Does a multiplication make sense ?
	if (m1->cols != m2->cols || m1->lines != m2->lines)
		DIE_ERROR("The two matrices aren't equally sized, and thus can't be multiplied (Hadamard product) together !")

	struct matrix res = matrix_new(m1->lines, m2->cols);
	for(uint32_t i = 0; i < res.lines; i++) {
		for(uint32_t j = 0; j < res.cols; j++) {
			matrix_set(&res, i, j, matrix_get(m1, i, j) * matrix_get(m2, i, j));
		}
	}
	return res;
}

struct matrix matrix_scalar_product(struct matrix *m, double lambda) {
	if (m == NULL)
		DIE_ERROR("Null argument !")

	struct matrix res = matrix_new(m->lines, m->cols);
	for(uint32_t i = 0; i < res.lines; i++) {
		for(uint32_t j = 0; j < res.cols; j++) {
			matrix_set(&res, i, j, lambda*matrix_get(m, i, j));
		}
	}
	return res;

}

struct matrix matrix_add_scalar(struct matrix *m1, struct matrix *m2, double lambda) {
	if (m1 == NULL || m2 == NULL)
		DIE_ERROR("Null argument !")

	if (m1->cols != m2->cols || m1->lines != m2->lines)
		DIE_ERROR("The two matrices aren't equally sized, and thus can't be added together !")

	struct matrix res = matrix_new(m1->lines, m1->cols);
	for(uint32_t i = 0; i < res.lines; i++) {
		for(uint32_t j = 0; j < res.cols; j++) {
			matrix_set(&res, i, j, matrix_get(m1, i, j) + lambda * matrix_get(m2, i, j));
		}
	}
	return res;

}

struct matrix matrix_add(struct matrix *m1, struct matrix *m2) {
	return matrix_add_scalar(m1, m2, 1.);
}

struct matrix matrix_substract(struct matrix *m1, struct matrix *m2) {
	return matrix_add_scalar(m1, m2, -1.);
}

struct matrix matrix_transpose(struct matrix *m) {
	if (m == NULL)
		DIE_ERROR("Null argument !")

	struct matrix res = matrix_new(m->cols, m->lines);
	for(uint32_t i = 0; i < res.lines; i++) {
		for(uint32_t j = 0; j < res.cols; j++) {
			matrix_set(&res, i, j, matrix_get(m, j, i));
		}
	}

	return res;
}

// Data MUST be the same size than the destination buffer, no guarantees are offered otherwise.
void matrix_set_data(struct matrix *m, void* data) {
	if (m == NULL || data == NULL)
		DIE_ERROR("Null argument !")

	memcpy(m->data, data, m->lines*m->cols*sizeof(double));
}

struct matrix matrix_copy(struct matrix *m) {
	if (m == NULL)
		DIE_ERROR("Null argument !")

	struct matrix res = matrix_new(m->lines, m->cols);
	matrix_set_data(&res, m->data);
	return res;
}

struct matrix matrix_identity(uint32_t n) {
	struct matrix m = matrix_new(n, n);
	for (uint32_t k = 0; k < n; k++)
		matrix_set(&m, k, k, 1.);

	return m;
}

void matrix_show(struct matrix* m) {
	if (m == NULL)
		DIE_ERROR("Null argument !")

	for (uint32_t i = 0; i < m->lines; i++) {
		printf("| ");
		for (uint32_t j = 0; j < m->cols; j++) {
			printf("%.04f, ", matrix_get(m, i, j));
		}
		printf("\b\b |\n");
	}
}

void test_matrix() {
	struct matrix id = matrix_identity(5);
	struct matrix a = matrix_new(5, 5);
	for (uint32_t i = 0; i < a.lines; i++) {
		for (uint32_t j = 0; j < a.cols; j++) {
			matrix_set(&a, i, j, i * a.cols + j);
		}
	}
	struct matrix b = matrix_new(2, 2);
	for (uint32_t i = 0; i < b.lines; i++) {
		for (uint32_t j = 0; j < b.cols; j++) {
			matrix_set(&b, i, j, i * b.cols + j);
		}
	}
	struct matrix prodIA = matrix_product(&id, &a);
	struct matrix prodAI = matrix_product(&a, &id);
	struct matrix prodBB = matrix_product(&b, &b);
	struct matrix addBB = matrix_add(&b, &b);
	struct matrix hadamardBB = matrix_hadamard_product(&b, &b);
	struct matrix transposeA = matrix_transpose(&a);
	struct matrix transposeB = matrix_transpose(&b);


	printf("IDENTITY MATRIX (SIZE 5):\n");
	matrix_show(&id);

	printf("MATRIX A:\n");
	matrix_show(&a);

	printf("MATRIX TRANSPOSE A:\n");
	matrix_show(&transposeA);

	printf("MATRIX I * A:\n");
	matrix_show(&prodIA);

	printf("MATRIX A * I:\n");
	matrix_show(&prodAI);

	printf("MATRIX B:\n");
	matrix_show(&b);

	printf("MATRIX B * B:\n");
	matrix_show(&prodBB);

	printf("MATRIX B + B:\n");
	matrix_show(&addBB);

	printf("MATRIX B ⊙ B:\n");
	matrix_show(&hadamardBB);

	printf("MATRIX TRANSPOSE B:\n");
	matrix_show(&transposeB);
}

struct network {
	uint32_t layers_number;
	struct matrix *layers;
	struct matrix *activated_layers;
	struct matrix *weights;
	struct matrix *bias;
};

// It is up to the caller to ensure the sizes asked for each layer are coherent with the previous layer (regarding matrices multiplication). No checks will be done !
struct network network_new(uint32_t layers_number, uint32_t *layers_sizes) {
	if (layers_sizes == NULL)
		DIE_ERROR("Null argument !")

	struct matrix *layers = malloc(layers_number*sizeof(struct matrix));
	struct matrix *activated_layers = malloc(layers_number*sizeof(struct matrix));
	struct matrix *weights = malloc(layers_number*sizeof(struct matrix));
	struct matrix *bias = malloc(layers_number*sizeof(struct matrix));
	if (layers == NULL || weights == NULL || bias == NULL)
		DIE_ERROR("Failed to allocate memory !")

	// the first layer is the 'input one' and remain unaltered during the computation,
	// so there is no need to have a 'non activated' layer, a weight, and a bias for this specific layer
	activated_layers[0] = matrix_new(layers_sizes[0], 1);

	for (uint32_t l = 1; l < layers_number; l++) {
		layers[l] = matrix_new(layers_sizes[l], 1);
		activated_layers[l] = matrix_new(layers_sizes[l], 1);
		weights[l] = matrix_new(layers_sizes[l], layers_sizes[l-1]);
		bias[l] = matrix_new(layers_sizes[l], 1);
		// set bias and weights to random values
		for (uint32_t i = 0; i < layers_sizes[l]; i++) {
			for (uint32_t j = 0; j < layers_sizes[l-1]; j++) {
				matrix_set(&weights[l], i, j, rand_double(0.05));
			}
			matrix_set(&bias[l], i, 0, rand_double(0.05));
		}
	}
	struct network net = { layers_number, layers, activated_layers, weights, bias };
	return net;
}

void network_destroy(struct network *net) {
	matrix_destroy(&net->activated_layers[0]);
	for (uint32_t i = 1; i < net->layers_number; i++) {
		matrix_destroy(&net->layers[i]);
		matrix_destroy(&net->activated_layers[i]);
		matrix_destroy(&net->bias[i]);
		matrix_destroy(&net->weights[i]);
	}
	free(net->layers);
	free(net->activated_layers);
	free(net->bias);
	free(net->weights);
}

double sigmoid(double x) {
	return 1/(1+exp(-x));
}

double sigmoid_derivative(double x) {
	return sigmoid(x)*(1-sigmoid(x));
}

void network_compute(struct network *net, struct matrix *data) {
	if (net == NULL || data == NULL)
		DIE_ERROR("Null argument !")

	matrix_set_data(&net->activated_layers[0], data->data);
	for (int i = 1; i < net->layers_number; i++) {
		struct matrix prod = matrix_product(&net->weights[i], &net->activated_layers[i-1]);
		struct matrix sum = matrix_add(&prod, &net->bias[i]);
		matrix_set_data(&net->layers[i], sum.data);

		matrix_destroy(&prod);
		matrix_destroy(&sum);

		// apply the activation function to the results
		for (int j = 0; j < net->layers[i].lines; j++) {
			matrix_set(&net->activated_layers[i], j, 0, sigmoid(matrix_get(&net->layers[i], j, 0)));
		}
	}
}

// Quadratic error
double network_cost(struct matrix *results, struct matrix *expected_output) {
	if (results == NULL)
		DIE_ERROR("Null argument !")

	double sum = 0.;
	// The result must be stored in a column vector
	for (int i = 0; i < results->lines; i++) {
		sum += pow((matrix_get(expected_output, i, 0)-matrix_get(results, i, 0)), 2);
	}
	return 1/2*sum;
}

struct matrix network_cost_derivative(struct matrix *output, struct matrix *expected_output) {
	if (output == NULL || expected_output == NULL)
		DIE_ERROR("Null argument !")

	return matrix_add_scalar(output, expected_output, -1.);
}

// TODO: matrices allocation/deallocation puts way too much overhead here, any idea ?
void network_backpropagation(struct network *net, struct matrix *expected_output, double learning_rate) {
	if (net == NULL || expected_output == NULL)
		DIE_ERROR("Null argument !")

	uint32_t layer = net->layers_number-1;

	struct matrix dcost = network_cost_derivative(&net->activated_layers[layer], expected_output);
	struct matrix dsigmoid = matrix_new(net->layers[layer].lines, 1);
	for (uint32_t i = 0; i < net->layers[layer].lines; i++) {
		matrix_set(&dsigmoid, i, 0, sigmoid_derivative(matrix_get(&net->layers[layer], i, 0)));
	}

	struct matrix delta = matrix_hadamard_product(&dcost, &dsigmoid);
	struct matrix transpose_previous_layer = matrix_transpose(&net->activated_layers[layer-1]);
	struct matrix nabla_w = matrix_product(&delta, &transpose_previous_layer);

	matrix_destroy(&transpose_previous_layer);
	matrix_destroy(&dcost);
	matrix_destroy(&dsigmoid);

	struct matrix new_w = matrix_add_scalar(&net->weights[layer], &nabla_w, -learning_rate);
	matrix_set_data(&net->weights[layer], new_w.data);
	struct matrix new_b = matrix_add_scalar(&net->bias[layer], &delta, -learning_rate);
	matrix_set_data(&net->bias[layer], new_b.data);
	matrix_destroy(&new_b);
	matrix_destroy(&new_w);

	for (layer = layer-1; layer > 0; layer--) {
		struct matrix transpose_weights_next_layer = matrix_transpose(&net->weights[layer+1]);
		struct matrix tmp = matrix_product(&transpose_weights_next_layer, &delta);
		struct matrix dsigmoid = matrix_new(net->layers[layer].lines, 1);
		for (uint32_t i = 0; i < net->layers[layer].lines; i++) {
			matrix_set(&dsigmoid, i, 0, sigmoid_derivative(matrix_get(&net->layers[layer], i, 0)));
		}

		struct matrix old_delta = delta;
		delta = matrix_hadamard_product(&tmp, &dsigmoid);
		matrix_destroy(&tmp);
		matrix_destroy(&old_delta);

		struct matrix nabla_b = delta;
		struct matrix transpose_previous_layer = matrix_transpose(&net->activated_layers[layer-1]);
		struct matrix nabla_w = matrix_product(&delta, &transpose_previous_layer);

		matrix_destroy(&transpose_previous_layer);

		struct matrix new_w = matrix_add_scalar(&net->weights[layer], &nabla_w, -learning_rate);
		matrix_set_data(&net->weights[layer], new_w.data);
		struct matrix new_b = matrix_add_scalar(&net->bias[layer], &nabla_b, -learning_rate);
		matrix_set_data(&net->bias[layer], new_b.data);
		matrix_destroy(&new_b);
		matrix_destroy(&new_w);
		matrix_destroy(&nabla_w);
	}
}

struct mnist_data {
	uint8_t value;
	uint8_t pixels[784];
};

void read_entire_buffer(void* dest, int fd, int size) {
	int nb_read = 0;
	int res = 1;
	while (nb_read != size && res > 0) {
		res = read(fd, dest+nb_read, size-nb_read);
		nb_read += res;
	}
	if (res <= 0) {
		close(fd);
		DIE_ERROR("Couldn't read enought data")
	}
}

struct mnist_data* mnist_read_file(char* image_file, char* index_file) {
	struct mnist_data* out = malloc(60000 * sizeof(struct mnist_data));
	if (out == NULL)
		DIE_ERROR("Cannot allocate memory");

	// copy images into memory
	int fd = open(image_file, O_RDONLY);
	if (fd == -1)
		DIE_ERROR("Impossible to read file")

	lseek(fd, 16, SEEK_SET);
	for (uint32_t i = 0; i < 60000; i++) {
		read_entire_buffer(out[i].pixels, fd, 784);
	}

	close(fd);

	// copy values into memory
	fd = open(index_file, O_RDONLY);
	if (fd == -1)
		DIE_ERROR("Impossible to read file")

	lseek(fd, 8, SEEK_SET);
	for (uint32_t i = 0; i < 60000; i++) {
		read_entire_buffer(&out[i].value, fd, 1);
	}
	close(fd);
	return out;
}

int main(int argc, char *argv[]) {
	// seed the random generator
	srand(time(NULL));
	/*
	struct mnist_data *mnist = mnist_read_file("/home/randomness/Downloads/train-images-idx3-ubyte", "/home/randomness/Downloads/train-labels-idx1-ubyte");
	uint32_t sizes[3] = {784, 500, 10};
	struct network brain = network_new(3, sizes);

	for (uint32_t i = 0; i < 50; i++) {
		// pick a random number from the training set
		struct mnist_data test = mnist[rand_int(60000)];

		// convert ints to doubles
		double test_pixels[784] = {0};
		for (uint32_t i = 0; i < 784; i++) {
			test_pixels[i] = (double)test.pixels[i];
		}

		struct matrix m = matrix_new(784, 1);
		matrix_set_data(&m, test_pixels);
		network_compute(&brain, &m);
		matrix_destroy(&m);

		struct matrix expected_output = matrix_new(10, 1);
		for (uint32_t j = 0; j < 9; j++) {
			matrix_set(&expected_output, j, 0, 0.);
		}
		matrix_set(&expected_output, test.value, 0, 0.);

		network_backpropagation(&brain, &expected_output, 0.05);
		matrix_destroy(&expected_output);
	test = mnist[rand_int(60000)];

	// convert ints to doubles
	for (uint32_t i = 0; i < 784; i++) {
		test_pixels[i] = (double)test.pixels[i];
	}

	m = matrix_new(784, 1);
	matrix_set_data(&m, test_pixels);
	network_compute(&brain, &m);
	matrix_destroy(&m);

	printf("expected value: %i\n", test.value);
	matrix_show(&brain.activated_layers[2]);


	}

	struct mnist_data test = mnist[rand_int(60000)];

	// convert ints to doubles
	double test_pixels[784] = {0};
	for (uint32_t i = 0; i < 784; i++) {
		test_pixels[i] = (double)test.pixels[i];
	}

	struct matrix m = matrix_new(784, 1);
	matrix_set_data(&m, test_pixels);
	network_compute(&brain, &m);
	matrix_destroy(&m);

	printf("expected value: %i\n", test.value);
	matrix_show(&brain.activated_layers[2]);

	network_destroy(&brain);
	free(mnist);
	*/
	// Simple test: a AND door
	double values[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
	double results[4] = {0, 0, 0, 1};
	struct matrix m = matrix_new(2, 1);

	uint32_t sizes[2] = {2, 1};
	struct network brain = network_new(2, sizes);

	for (uint32_t i = 0; i < 50; i++) {
		int index = rand_int(4);

		double test[2] = {values[index][0], values[index][1]};
		matrix_set_data(&m, test);
		network_compute(&brain, &m);

		struct matrix expected_output = matrix_new(1, 1);
		matrix_set(&expected_output, 0, 0, results[index]);
		network_backpropagation(&brain, &expected_output, 0.05);
		//matrix_show(&brain.weights[1]);
		//matrix_show(&brain.bias[1]);
		matrix_destroy(&expected_output);
		printf("test values : %f - %f | expected value: %f\n", values[index][0], values[index][1], results[index]);
		matrix_show(&brain.activated_layers[1]);
	}

	matrix_destroy(&m);
	network_destroy(&brain);
	return EXIT_SUCCESS;
}
