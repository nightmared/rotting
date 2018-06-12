#include "common.h"

char* readFileToString(char* file) {
	FILE *fp = fopen(file, "r");
	if(!fp) {
		printf("Unable to open file %s\n", file);
		exit(1);
	}

	// get file size (and pray for the file not being truncated/deleted in the middle ;))
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	rewind(fp);

	// let's not forget the null pointer at the end
	char *buffer = malloc(size+1);
	buffer[size] = '\0';
	if(buffer == NULL)
		exit(1);

	if(fread(buffer, size, 1, fp) <= 0) {
		fclose(fp);
		free(buffer);
		exit(1);
	}
	fclose(fp);

	return buffer;
}

void writeStringToFile(char* file, char* str) {
	FILE *fp = fopen(file, "w");
	if(!fp) {
		printf("Unable to open/create file %s\n", file);
		exit(1);
	}

	fwrite(str, strlen(str), 1, fp);

	fclose(fp);
}

struct LinesArray readFileToLinesArray(char* file) {
	char* str = readFileToString(file);

	// count line number
	int lineno = 0;
	int len = strlen(str);
	for (int i = 0; i < len; i++)
		if(str[i] == '\n')
			lineno++;

	char** lines = malloc(sizeof(void*)*lineno);
	if (lines == NULL)
		exit(1);

	// add to array
	int begin = 0;
	int pos = 0;
	for (int i = 0; i < lineno; i++) {
		while (str[pos] != '\n')
			pos++;

		// needed for strcpy and to get rid of the '\r'
		str[pos-1]= '\0';

		lines[i] = malloc(pos-begin);
		if (lines[i] == NULL)
			exit(1);

		strcpy(lines[i], str+begin);

		begin = ++pos;
	}

	free(str);
	struct LinesArray ret = {lineno, lines};
	return ret;
}

void LinesArray_free(struct LinesArray* l) {
	for (int i = 0; i < l->number; i++)
		free(l->lines[i]);
}

int get_split_pos(char* str, char delim, int to_scan) {
	for (int i=0; i<to_scan; i++) {
		if (str[i] == delim)
			return i;
	}
	return -1;
}

// assume the delimiter is *always* present in the file
char** cut_on_delim(char* str, char delim) {
	// count the numbers 'no' of occurences
	int len = strlen(str);
	int no = 1;
	int pos = 0;
	int j = 0;
	while ((j = get_split_pos(str+pos, delim, len-pos)) != -1) {
		pos+=j+1;
		no++;
	}

	char** cuts = malloc(no*sizeof(char*));
	if (cuts == NULL)
		exit(1);

	pos = 0;
	for (int i = 0; i < no; i++) {
		cuts[i] = str+pos;
		pos += get_split_pos(str+pos, delim, len-pos);
		str[pos++] = '\0';
	}

	return cuts;
}

int append_text(char* str, char* to_append, int size) {
	while (strlen(str)+strlen(to_append)+2 > size) {
		size *= 2;
		if (realloc(str, size) == NULL)
			exit(1);
	}
	memcpy(str+strlen(str), to_append, strlen(to_append));
	str[strlen(str)+strlen(to_append)] = '\0';
	return size;
}
