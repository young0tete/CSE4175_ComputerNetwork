#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[]) {

	//argument input error
	if (argc != 5) {
		printf("usage: ./crc_encoder input_file output_file generator dataword_size");
		exit(1);
	}

	//read input and output file
	FILE* input;
	FILE* output;
	input = fopen(argv[1], "rb");
	output = fopen(argv[2], "wb");

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

	//dataword size error
	int dataword_size = atoi(argv[4]);
	if ((strcmp(argv[4], "4") != 0) && (strcmp(argv[4], "8") != 0)) {
		printf("dataword size must be 4 or 8.");
		exit(1);
	}

	//check file size
	fseek(input, 0, SEEK_END);
	int input_size = ftell(input);
	fseek(input, 0, SEEK_SET);

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

	//ascii -> dataword
	int** dataword = (int**)malloc(sizeof(int*) * input_size);
	for (int i = 0; i < input_size; i++) {
		dataword[i] = (int*)malloc(sizeof(int) * 8);
	}
	for (int i = 0; i < input_size; i++) {
		for (int j = 7; j >= 0; j--) {
			dataword[i][j] = input_ascii[i] % 2;
			input_ascii[i] /= 2;
		}
	}

	//dataword_size=4 case
	int** dataword4 = (int**)malloc(sizeof(int*) * input_size * 2);
	if (dataword_size == 4) {
		for (int i = 0; i < input_size * 2; i++) {
			dataword4[i] = (int*)malloc(sizeof(int) * 4);
		}

		for (int i = 0; i < input_size; i++) {
			for (int j = 7; j >= 0; j--) {
				if (j <= 3) {
					dataword4[2 * i][j] = dataword[i][j];
				}
				else {
					dataword4[2 * i + 1][j % 4] = dataword[i][j];
				}
			}
		}
	}

	//get the divisor
	int divisor_len = (int)strlen(argv[3]);
	int* divisor = (int*)malloc(sizeof(int) * divisor_len);
	for (int i = 0; i < divisor_len; i++) {
		divisor[i] = argv[3][i] - '0';
	}

	int q = (8 / dataword_size); //ds=8 -> 1, ds=4 -> 2
	int** codeword = (int**)malloc(sizeof(int*) * (input_size)*q);
	for (int i = 0; i < input_size * q; i++) {
		codeword[i] = (int*)malloc(sizeof(int) * (divisor_len + dataword_size - 1));
	}
	int* tmp = (int*)malloc(sizeof(int) * (divisor_len));
	int last = 0;

	if (dataword_size == 8) {
		for (int i = 0; i < input_size * q; i++) {
			//attatching 0s
			for (int j = 0; j < dataword_size; j++) {
				codeword[i][j] = dataword[i][j];
			}
			for (int j = dataword_size; j < (divisor_len + dataword_size - 1); j++) {
				codeword[i][j] = 0;
			}

			//initialization
			for (int j = 0; j < divisor_len; j++) {
				tmp[j] = codeword[i][j];
			}

			for (int j = 0; j < dataword_size; j++) {
				if (tmp[0] == 1) {
					for (int k = 1; k < divisor_len; k++) {
						tmp[k - 1] = tmp[k] ^ divisor[k];
					}
				}
				else {
					for (int k = 1; k < divisor_len; k++) {
						tmp[k - 1] = tmp[k];
					}
				}
				if ((j + divisor_len) < (divisor_len + dataword_size - 1)) {
					tmp[divisor_len - 1] = codeword[i][j + divisor_len];
				}
			}
			for (int j = dataword_size; j < (divisor_len + dataword_size - 1); j++) {
				codeword[i][j] = tmp[j - dataword_size];
			}
		}
	}
	else if (dataword_size == 4) {
		for (int i = 0; i < input_size * q; i++) {
			//attatching 0s
			for (int j = 0; j < dataword_size; j++) {
				codeword[i][j] = dataword4[i][j];
			}

			for (int j = dataword_size; j < (divisor_len + dataword_size - 1); j++) {
				codeword[i][j] = 0;
			}

			//initialization
			for (int j = 0; j < divisor_len; j++) {
				tmp[j] = codeword[i][j];
			}

			for (int j = 0; j < dataword_size; j++) {
				if (tmp[0] == 1) {
					for (int k = 1; k < divisor_len; k++) {
						tmp[k - 1] = tmp[k] ^ divisor[k];
					}
				}
				else {
					for (int k = 1; k < divisor_len; k++) {
						tmp[k - 1] = tmp[k];
					}
				}
				if ((j + divisor_len) < (divisor_len + dataword_size - 1)) {
					tmp[divisor_len - 1] = codeword[i][j + divisor_len];
				}
			}
			for (int j = dataword_size; j < (divisor_len + dataword_size - 1); j++) {
				codeword[i][j] = tmp[j - dataword_size];
			}
		}
	}

	//check the sum of sizes of the codewords
	int codeword_size = dataword_size + divisor_len - 1;
	int codeword_num = input_size * q;
	int total_bits = codeword_size * codeword_num;
	int padding_num = 8 - (total_bits % 8);
	int header_dummy[8] = { 0 };
	for (int i = 7; i >= 0; i--) {
		header_dummy[i] = padding_num % 2;
		padding_num /= 2;
	}
	padding_num = 8 - (total_bits % 8);

	//padding and write output file
	int* final = (int*)malloc(sizeof(int) * (8+padding_num+(codeword_num*codeword_size)));
	for (int i = 0; i < 8; i++) {
		final[i] = header_dummy[i];
	}
	for (int i = 8; i < 8 + padding_num; i++) {
		final[i] = 0;
	}

	int k = 8 + padding_num;
	for (int i = 0; i < codeword_num; i++) {
		for (int j = 0; j < codeword_size; j++) {
			final[k] = codeword[i][j];
			k += 1;
		}
	}

	for (int i = 0; i < (8 + padding_num + (codeword_num * codeword_size)); i += 8) {
		unsigned char byte = 0; // 바이너리 데이터를 저장할 변수 초기화

		// 8비트(1바이트) 단위로 데이터를 계산하여 바이너리로 저장
		for (int j = 0; j < 8; j++) {
			if (i + j < (8 + padding_num + (codeword_num * codeword_size))) {
				byte = (byte << 1) | final[i + j]; // final 배열의 값을 왼쪽으로 이동하고 해당 비트를 추가
			}
		}

		// 바이너리 데이터를 파일에 씁니다.
		fwrite(&byte, sizeof(unsigned char), 1, output);
	}

	//free
	free(input_data);
	free(input_ascii);
	for (int i = 0; i < input_size; i++) {
		free(dataword[i]);
	}
	free(dataword);
	for (int i = 0; i < input_size * 2; i++) {
		free(dataword4[i]);
	}
	free(dataword4);
	for (int i = 0; i < input_size * q; i++) {
		free(codeword[i]);
	}
	free(codeword);
	free(divisor);
	free(tmp);

	fclose(input);
	fclose(output);
	free(final);

	return 0;
}