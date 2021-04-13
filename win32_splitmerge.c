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
#include <windows.h>


//~~~~~~~~~~~~~~~~
//
// PLATFORM API
//
#define os_alloc win32_alloc
#define os_realloc win32_realloc
#define os_free win32_free

#define os_is_handle_valid win32_is_handle_valid
#define os_close_file win32_close_file
#define os_open_file_for_reading win32_open_file_for_reading
#define os_open_file_for_writing win32_open_file_for_writing
#define os_move_file_pointer win32_move_file_pointer
#define os_read_file win32_read_file
#define os_write_file win32_write_file

#define os_get_size_of_file win32_get_size_of_file
#define os_get_remaining_size_of_file win32_get_remaining_size_of_file

#define os_get_random_u64 win32_get_random_u64
#define os_set_random_seed win32_set_random_seed


//~~~~~~~~~~~~~~~~
//
// TYPES
//
typedef HANDLE File_Handle;


//~~~~~~~~~~~~~~~~
//
// MEMORY
//
static
PLATFORM_ALLOC(win32_alloc) {
    void *result = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    
    return result;
}

static
PLATFORM_REALLOC(win32_realloc) {
    void *result = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, old_ptr, new_size);
    return result;
}

static
PLATFORM_FREE(win32_free) {
    HeapFree(GetProcessHeap(), 0, ptr);
}


//~~~~~~~~~~~~~~~~
//
// FILE
//
static
PLATFORM_IS_HANDLE_VALID(win32_is_handle_valid) {
    if(handle != INVALID_HANDLE_VALUE) {
        return true;
    }
    return false;
}

static
PLATFORM_CLOSE_FILE(win32_close_file) {
    CloseHandle(handle);
}

static
PLATFORM_OPEN_FILE_FOR_READING(win32_open_file_for_reading) {
    File_Handle result = INVALID_HANDLE_VALUE;
    
    if(file_name) {
        result = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0,
							 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }
    
    return result;
}

static
PLATFORM_OPEN_FILE_FOR_WRITING(win32_open_file_for_writing) {
    File_Handle result = INVALID_HANDLE_VALUE;
    
    if(file_name) {
        result = CreateFileA(file_name, GENERIC_WRITE, 0, 0,
							 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    }
    
    return result;
}

static
PLATFORM_MOVE_FILE_POINTER(win32_move_file_pointer) {
    LARGE_INTEGER distance_to_move = {0};
    distance_to_move.QuadPart = desired_offset;
    
    if(SetFilePointerEx(handle, distance_to_move, 0, FILE_CURRENT)) {
        return true;
    }
    return false;
}

static
PLATFORM_READ_FILE(win32_read_file) {
    i64 result = 0;
    
    if(file && file->data) {
        if(read_amount > file->capacity - file->length) {
            read_amount = file->capacity - file->length;
        }
        
        DWORD bytes_read    = 0;
        DWORD bytes_to_read = 0;
        
        while(read_amount > 0) {
            if(read_amount > 0xFFFFFFFF) {
                bytes_to_read = 0xFFFFFFFF;
                read_amount  -= bytes_to_read;
            } else {
                bytes_to_read = (DWORD)read_amount;
                read_amount   = 0;
            }
            
            if(ReadFile(handle, file->data + file->length, bytes_to_read, &bytes_read, 0)) {
                result       += bytes_read;
                file->length += bytes_read;
            }
        }
    }
    
    return result;
}

static
PLATFORM_WRITE_FILE(win32_write_file) {
    i64 result = 0;
    
    if(data) {
        DWORD bytes_written  = 0;
        DWORD bytes_to_write = 0;
        
        while(length > 0) {
            if(length > 0xFFFFFFFF) {
                bytes_to_write  = 0xFFFFFFFF;
                length         -= bytes_to_write;
            } else {
                bytes_to_write = (DWORD)length;
                length         = 0;
            }
            
            if(WriteFile(handle, data, bytes_to_write, &bytes_written, 0)) {
                result += bytes_written;
                data   += bytes_written;
            }
        }
    }
    
    return result;
}

static
PLATFORM_GET_SIZE_OF_FILE(win32_get_size_of_file) {
    i64 result = 0;
    
    LARGE_INTEGER size = {0};
    
    if(GetFileSizeEx(handle, &size)) {
        result = size.QuadPart;
    }
    
    return result;
}

static
PLATFORM_GET_REMAINING_SIZE_OF_FILE(win32_get_remaining_size_of_file) {
    i64 result = 0;
    
    LARGE_INTEGER distance_to_move = {0};
    LARGE_INTEGER current_offset   = {0};
    LARGE_INTEGER end_offset       = {0};
    
    if(SetFilePointerEx(handle, distance_to_move, &current_offset, FILE_CURRENT)) {
        if(SetFilePointerEx(handle, distance_to_move, &end_offset, FILE_END)) {
            result = end_offset.QuadPart - current_offset.QuadPart;
            
            SetFilePointerEx(handle, current_offset, 0, FILE_BEGIN);
        }
    }
    
    return result;
}

static
PLATFORM_GET_RANDOM_U64(win32_get_random_u64) {
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
PLATFORM_SET_RANDOM_SEED(win32_set_random_seed) {
    u64 result = 0;
    
    SYSTEMTIME time = {0};
    GetLocalTime(&time);
    
    result  = time.wYear + time.wMilliseconds;
    result |= ((u64)time.wMonth ) << 32 | ((u64)time.wDayOfWeek) << 35;
    result |= ((u64)time.wDay   ) << 40 | ((u64)time.wHour)      << 45;
    result |= ((u64)time.wMinute) << 51 | ((u64)time.wSecond)    << 57;
    
    win32_get_random_u64(&result);
    
    return result;
}
