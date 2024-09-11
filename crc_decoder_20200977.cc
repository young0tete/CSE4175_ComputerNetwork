#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int i, j, k;

	//argument input error
	if (argc != 6) {
		printf("./crc_decoder input_file output_file result_file generator dataword_size");
		exit(1);
	}

	//read input and output file
	FILE* input;
	FILE* output;
	FILE* result;
	input = fopen(argv[1], "rb");
	output = fopen(argv[2], "wb");
	result = fopen(argv[3], "w");

	//input open error
	if (input == NULL) {
		printf("input file open error.");
		exit(1);
	}

	//output open error
	if (output == NULL) {
		printf("output file open error.");
		exit(1);
	}

	//result open error
	if (result == NULL) {
		printf("result file open error.");
		exit(1);
	}

	//dataword size error
	int dataword_size = atoi(argv[5]);
	if ((strcmp(argv[5], "4") != 0) && (strcmp(argv[5], "8") != 0)) {
		printf("dataword size must be 4 or 8.");
		exit(1);
	}

	//get the divisor
	int divisor_len = (int)strlen(argv[4]);
	int* divisor = (int*)malloc(sizeof(int) * divisor_len);
	for (int i = 0; i < divisor_len; i++) {
		divisor[i] = argv[4][i] - '0';
	}

	//check file size
	fseek(input, 0, SEEK_END);
	int input_size = ftell(input);
	fseek(input, 0, SEEK_SET);

	//개행문자 제외
	input_size -= 1;

	//get input data
	char* input_data = (char*)malloc(sizeof(char) * input_size);
	for (int i = 0; i < input_size; i++) {
		fscanf(input, "%c", &input_data[i]);
	}

	//input_data -> ascii
	int* input_ascii = (int*)malloc(sizeof(int) * input_size);
	for (int i = 0; i < input_size; i++) {
		input_ascii[i] = (unsigned char)input_data[i];
	}

	//ascii -> binary_data
	int** binary_data = (int**)malloc(sizeof(int*) * input_size);
	for (int i = 0; i < input_size; i++) {
		binary_data[i] = (int*)malloc(sizeof(int) * 8);
	}
	for (int i = 0; i < input_size; i++) {
		for (int j = 7; j >= 0; j--) {
			binary_data[i][j] = input_ascii[i] % 2;
			input_ascii[i] /= 2;
		}
	}

	//check the size of 8bits blocks
	int blocks = input_size;

	//find the # of padding bits
	int padding = 0;
	if (binary_data[0][5] == 1) padding += 4;
	if (binary_data[0][6] == 1) padding += 2;
	if (binary_data[0][7] == 1) padding += 1;

	//get the codeword
	int codeword_size = dataword_size + divisor_len - 1;
	int codeword_num = (blocks * 8 - 8 - padding) / codeword_size;
	int** codeword = (int**)malloc(sizeof(int*) * codeword_num);
	for (int i = 0; i < codeword_num; i++) {
		codeword[i] = (int*)malloc(sizeof(int) * codeword_size);
	}

	//get meaningful datas
	int* meaningful = (int*)malloc(sizeof(int) * (codeword_size * codeword_num));
	k = 0;
	for (i = 1; i < input_size; i++) {
		for (j = padding; j < 8; j++) {
			meaningful[k] = binary_data[i][j];
			k += 1;
		}
		padding = 0;
	}

	for (i = 0; i < (codeword_size * codeword_num); i++) {
		codeword[i / codeword_size][i % codeword_size] = meaningful[i];
	}

	//codeword error check	
	//int* tmp = (int*)malloc(sizeof(int) * (divisor_len));
	//int* tmp = (int*)malloc(sizeof(int) * (divisor_len + dataword_size - 1));
	int tmp[300] = { 0 }; //////
	int error = 0;
	int is_error = 0;

	for (i = 0; i < codeword_num; i++) {
		//initialization
		for (j = 0; j < divisor_len; j++) {
			tmp[j] = codeword[i][j];
		}
		is_error = 0;

		for (j = 0; j < dataword_size; j++) {
			if (tmp[0] == 1) {
				for (k = 1; k < divisor_len; k++) {
					tmp[k - 1] = tmp[k] ^ divisor[k];
				}
			}
			else {
				for (k = 1; k < divisor_len; k++) {
					tmp[k - 1] = tmp[k];
				}
			}
			if ((j + divisor_len) < (divisor_len + dataword_size - 1)) {
				tmp[divisor_len - 1] = codeword[i][j + divisor_len];
			}
		}

		for (j = 0; j < divisor_len - 1; j++) {
			if (tmp[j] == 1) {
				is_error = 1;
			}
		}
		if (is_error == 1) {
			error += 1;
		}
	}

	
	//write output
	int final[999999] = { 0 };
	//int* final = (int*)malloc(sizeof(int) * (codeword_num * dataword_size));
	k = 0;
	for (i = 0; i < codeword_num; i++) {
		for (j = 0; j < dataword_size; j++) {
			final[k] = codeword[i][j];
			k += 1;
		}
	}

	for (int i = 0; i < (codeword_num * dataword_size); i += 8) {
		unsigned char byte = 0; // 바이너리 데이터를 저장할 변수 초기화

		// 8비트(1바이트) 단위로 데이터를 계산하여 바이너리로 저장
		for (int j = 0; j < 8; j++) {
			if (i + j < (codeword_num * dataword_size)) {
				byte = (byte << 1) | final[i + j]; // final 배열의 값을 왼쪽으로 이동하고 해당 비트를 추가
			}
		}

		// 바이너리 데이터를 파일에 씁니다.
		fwrite(&byte, sizeof(unsigned char), 1, output);
	}


	//write result
	fprintf(result, "%d %d", codeword_num, error);

	fclose(input);
	fclose(output);
	fclose(result);

	//free
	free(input_data);
	free(input_ascii);
	for (int i = 0; i < codeword_num; i++) {
		//free(codeword[i]);
	}
	free(codeword);
	for (i = 0; i < input_size; i++) {
		//free(binary_data[i]);
	}
	free(binary_data);
	free(divisor);
	//free(tmp);
	//free(meaningful);
	//free(final);

	return 0;
}