//~~~~~~~~~~~~~~~~
// MIT License
//
// Copyright (c) 2021 Patrik Johansson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


//~~~~~~~~~~~~~~~~
//
// INCLUDES
//
#include <stdlib.h>
#include <time.h>


//~~~~~~~~~~~~~~~~
//
// PLATFORM API
//
#define os_alloc crt_alloc
#define os_realloc crt_realloc
#define os_free crt_free

#define os_is_handle_valid crt_is_handle_valid
#define os_close_file crt_close_file
#define os_open_file_for_reading crt_open_file_for_reading
#define os_open_file_for_writing crt_open_file_for_writing
#define os_move_file_pointer crt_move_file_pointer
#define os_read_file crt_read_file
#define os_write_file crt_write_file

#define os_get_size_of_file crt_get_size_of_file
#define os_get_remaining_size_of_file crt_get_remaining_size_of_file

#define os_get_random_u64 crt_get_random_u64
#define os_set_random_seed crt_set_random_seed


//~~~~~~~~~~~~~~~~
//
// TYPES
//
typedef FILE * File_Handle;


//~~~~~~~~~~~~~~~~
//
// MEMORY
//
static
PLATFORM_ALLOC(crt_alloc) {
    void *result = malloc(size);
    
    For(i64, it_index, size) {
        *((u8*)result + it_index) = 0;
    }
    
    return result;
}

static
PLATFORM_REALLOC(crt_realloc) {
    void *result = realloc(old_ptr, new_size);
    return result;
}

static
PLATFORM_FREE(crt_free) {
    free(ptr);
}


//~~~~~~~~~~~~~~~~
//
// FILE
//
static
PLATFORM_IS_HANDLE_VALID(crt_is_handle_valid) {
    if(handle) {
        return true;
    }
    return false;
}

static
PLATFORM_CLOSE_FILE(crt_close_file) {
    fclose(handle);
}

static
PLATFORM_OPEN_FILE_FOR_READING(crt_open_file_for_reading) {
    File_Handle result = 0;
    
    if(file_name) {
        result = fopen(file_name, "rb");
    }
    
    return result;
}

static
PLATFORM_OPEN_FILE_FOR_WRITING(crt_open_file_for_writing) {
    File_Handle result = 0;
    
    if(file_name) {
        result = fopen(file_name, "wb");
    }
    
    return result;
}

static
PLATFORM_MOVE_FILE_POINTER(crt_move_file_pointer) {
    if(handle) {
        fpos_t current = 0;
        fgetpos(handle, &current);
        
        current += desired_offset;
        fsetpos(handle, &current);
        
        return true;
    }
    
    return false;
}

static
PLATFORM_READ_FILE(crt_read_file) {
    i64 result = 0;
    
    if(file && handle) {
        if(read_amount > file->capacity - file->length) {
            read_amount = file->capacity - file->length;
        }
        
        fpos_t offset = 0;
        fgetpos(handle, &offset);
        
        result = fread(file->data + file->length, 1, read_amount, handle);
        
        file->length += result;
        
        offset += result;
        fsetpos(handle, &offset);
    }
    
    return result;
}

static
PLATFORM_WRITE_FILE(crt_write_file) {
    i64 result = 0;
    
    if(handle) {
        fpos_t offset = 0;
        fgetpos(handle, &offset);
        
        result = fwrite(data, 1, length, handle);
        
        offset += result;
        fsetpos(handle, &offset);
    }
    
    return result;
}

static
PLATFORM_GET_SIZE_OF_FILE(crt_get_size_of_file) {
    i64 result = 0;
    
    if(handle) {
        fpos_t current_offset = 0;
        fpos_t end_offset     = 0;
        fpos_t begin_offset   = 0;
        
        fgetpos(handle, &current_offset);
        fseek(handle, 0, SEEK_SET);
        fgetpos(handle, &begin_offset);
        fseek(handle, 0, SEEK_END);
        fgetpos(handle, &end_offset);
        
        result = end_offset - begin_offset;
        
        fsetpos(handle, &current_offset);
    }
    
    return result;
}

static
PLATFORM_GET_REMAINING_SIZE_OF_FILE(crt_get_remaining_size_of_file) {
    i64 result = 0;
    
    if(handle) {
        fpos_t current_offset = 0;
        fpos_t end_offset     = 0;
        
        fgetpos(handle, &current_offset);
        fseek(handle, 0, SEEK_END);
        fgetpos(handle, &end_offset);
        
        result = end_offset - current_offset;
        
        fsetpos(handle, &current_offset);
    }
    
    return result;
}

static
PLATFORM_GET_RANDOM_U64(crt_get_random_u64) {
    u64 result = 0;
    
    if(state) {
        result = *state;
        
        result ^= result << 13;
        result ^= result >> 17;
        result ^= result <<  5;
        
        *state = result;
    }
    
    return result;
}

static
PLATFORM_SET_RANDOM_SEED(crt_set_random_seed) {
    srand((unsigned int)time(0));
    
    u64 result = rand() | ((u64)rand()) << 32;
    
    crt_get_random_u64(&result);
    
    return result;
}
