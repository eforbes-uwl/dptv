#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


void * add_array_list(void * array, uint32_t * num, uint64_t * ar_size, void * data, uint64_t data_size) {
    uint64_t ins_pos = (*num * data_size);
    // Resize if going to run out of room
    if ((ins_pos + data_size) > *ar_size) {
        (*ar_size) += (*ar_size) / 2 + data_size;
        array = realloc(array, *ar_size);
        // If realloc failed return with error
        if (array == NULL) {return array;}
    }
    // Copy data into array
    memcpy(array + ins_pos, data, data_size);
    (*num) ++;
    // Return new pointer to array
    return array;
}


void * add_array_list_new(void * array, uint32_t * num, uint64_t * ar_size, void ** data, uint64_t data_size) {
    uint64_t ins_pos = (*num * data_size);
    // Resize if going to run out of room
    if ((ins_pos + data_size) > *ar_size) {
        (*ar_size) += (*ar_size) / 2 + data_size;
        array = realloc(array, *ar_size);
        // Make sure allocation succeeded
        assert(array);
    }
    // Clear new data section
    *data = array + ins_pos;
    memset(*data, 0, data_size);
    (*num) ++;
    // Return new pointer to array
    return array;
}


void * finish_array_list(void * array, uint32_t * num, uint64_t data_size) {
    // Resize array to size needed
    return realloc(array, (*num * data_size));
}



