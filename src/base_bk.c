#include <stdlib.h>
#include "varray.h"
#include "base.h"
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

const char def_digits[64] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";

char* base_convert_digits(long num, int out_base, int min_len, const char* digits) {
    varray_t v_ar = var_new(sizeof(char)); 
    int num_digs = 0; 
    while(num > 0) { 
        // Get next digit 
        int digit_index = (num % out_base); 
        char dig = digits[digit_index]; 
        var_add(&v_ar, &dig); 
        // Shift converting number 
        num = num / out_base; 
        num_digs ++; 
    } 
    // Add extra digits to get up to minimum length
    if (num_digs < min_len) {
        int rep = min_len - num_digs;
        for(int i = 0; i < rep; i++) { 
            // Add zero digit 
            char dig = digits[0]; 
            var_add(&v_ar, &dig); 
            num_digs ++; 
        } 
    } 
    // Reverse String 
    char* o_str = malloc(num_digs + 1); 
    for(int i = 0; i < num_digs; i++) { 
        o_str[i] = ((char*)(v_ar.arr))[num_digs - i - 1]; 
    } 
    o_str[num_digs] = '\0'; 
    // Free v_array
    var_free(&v_ar); 
    return o_str; 
}

char* base_convert(long num, int out_base, int min_len) {
    return base_convert_digits(num, out_base, min_len, def_digits);
}

char* base_convert_string_digits(char* str, int str_len, int in_base, int out_base, int min_len, const char* in_digits, const char* out_digits) {
    // Set digits array to default array if null
    if (in_digits == NULL) {
        in_digits = def_digits;
    }
    if (out_digits == NULL) {
        out_digits = def_digits;
    }

    uint64_t num = base_convert_num_digits(str, str_len, in_base, in_digits);
    
    // Convert to out_base
    return base_convert_digits(num, out_base, min_len, out_digits);
}

char* base_convert_string(char* str, int str_len, int in_base, int out_base, int min_len) {
    return base_convert_string_digits(str, str_len, in_base, out_base, min_len, def_digits, def_digits);
}

uint64_t base_convert_num_digits(char* str, int str_len, int in_base, const char* in_digits) {
    // Convert string to a number
    uint64_t num = 0;
    for(int i = 0; i < str_len; i++) {
        // Get current character and find it's position in the digits array
        char c = str[i];
        if (c == '\0') {
            break;
        }
        int a = 0;    // Default value if character not found
        for(int j = 0; j < in_base; j++) {
            if (c == in_digits[j]) {
                a = j;
                j = in_base;
            }
        }
        // Shift number and add
        num = (num * in_base) + a;
    }
    return num;
}

bool is_number(char* str, unsigned int str_len, int in_base, const char* in_digits) {
    if (in_digits == NULL) {
        in_digits = def_digits;
    }
    for(int i = 0; i < str_len; i++) {
        char c = str[i];
        if (c == '\0') {
            break;
        }
        // Get position in digit string
        int a = -1;
        for(int j = 0; j < in_base; j++) {
            if (c == in_digits[j]) {
                a = j;
                j = in_base;
            }
        }
        // Return false if digit not found
        if (a == -1) {
            return false;
        }
    }
    return true;
}

uint64_t base_convert_num(char* str, int str_len, int in_base) {
    return base_convert_num_digits(str, str_len, in_base, def_digits);
}



