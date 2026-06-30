#include "cal.h"
#include <stdio.h>
#include <stdlib.h>

char input_buffer[32]; 
int buf_idx = 0;       

void input_calculator(char key) {

	if (key == '=') {
		input_buffer[buf_idx] = '\0';
		
		int32_t num1 = 0, num2 = 0;
		char op = '\0';
		int32_t result = 0;

		if (sscanf(input_buffer, "%d%c%d", &num1, &op, &num2) == 3) {
			if (op == '+') result = num1 + num2;
			else if (op == '-') result = num1 - num2;
			else if (op == '*') result = num1 * num2;
			else if (op == '/') {
				if (num2 != 0) result = num1 / num2;
				else result = 0; 
			}

			printf("result : %d", result);
			buf_idx = sprintf(input_buffer, "d", result);
		}
		return;
	}

	if (buf_idx < 31) {
		input_buffer[buf_idx] = key;
		buf_idx++;
		input_buffer[buf_idx] = '\0'; 
	}
}