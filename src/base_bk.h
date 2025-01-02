#ifndef BASE_H
#define BASE_H

#include <stdint.h>
#include <stdbool.h>

char* base_convert_digits(long num, int out_base, int min_len, const char* digits);
char* base_convert(long num, int out_base, int min_len);
char* base_convert_string_digits(char* str, int str_len, int in_base, int out_base, int min_len, const char* in_digits, const char* out_digits);
char* base_convert_string(char* str, int str_len, int in_base, int out_base, int min_len);
uint64_t base_convert_num_digits (char* str, int str_len, int in_base, const char* in_digits);
uint64_t base_convert_num (char* str, int str_len, int in_base);
bool is_number(char* str, unsigned int str_len, int in_base, const char* in_digits);



#endif
