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
#include <stdio.h>
#include <stdint.h>


//~~~~~~~~~~~~~~~~
//
// PLATFORM API
//
#define PLATFORM_ALLOC(name) void * name(i64 size)
#define PLATFORM_REALLOC(name) void * name(void *old_ptr, i64 new_size)
#define PLATFORM_FREE(name) void name(void *ptr)

#define PLATFORM_IS_HANDLE_VALID(name) bool name(File_Handle handle)
#define PLATFORM_CLOSE_FILE(name) void name(File_Handle handle)
#define PLATFORM_OPEN_FILE_FOR_READING(name) File_Handle name(char *file_name)
#define PLATFORM_OPEN_FILE_FOR_WRITING(name) File_Handle name(char *file_name)
#define PLATFORM_MOVE_FILE_POINTER(name) bool name(File_Handle handle, i64 desired_offset)
#define PLATFORM_READ_FILE(name) i64 name(File_Data *file, File_Handle handle, i64 read_amount)
#define PLATFORM_WRITE_FILE(name) i64 name(File_Handle handle, u8 *data, i64 length)

#define PLATFORM_GET_SIZE_OF_FILE(name) i64 name(File_Handle handle)
#define PLATFORM_GET_REMAINING_SIZE_OF_FILE(name) i64 name(File_Handle handle)

#define PLATFORM_GET_RANDOM_U64(name) u64 name(u64 *state)
#define PLATFORM_SET_RANDOM_SEED(name) u64 name()


//~~~~~~~~~~~~~~~~
//
// FOR
//
#define for_range(type, index, from, to) for(type index = from; index < to; index = index + 1)
#define rfor_range(type, index, from, to) for(type index = to - 1; index >= from; index = index - 1)

#define For(type, index, count) for_range(type, index, 0, count)
#define rfor(type, index, count) rfor_range(type, index, 0, count)


//~~~~~~~~~~~~~~~~
//
//
//
#define is_flag_set(var, flag) (((var) & (flag)) == (flag))

//~ NOTE(Patrik): ntstring is a null-terminated string (char *str)
// cstring is a pointer and a length (char *data, i32 length)
// String is the struct version of cstring
// cstring and String does not need to be null-terminated
#define UNPACK_NTSTRING(str) (str), (sizeof(str) - 1)


//~~~~~~~~~~~~~~~~
//
// MEMORY
//
#define SPLTMRG_ALLOC(type, count) (type *)os_alloc(count * sizeof(type))
#define SPLTMRG_REALLOC(type, data, new_count) (type *)os_realloc(data, new_count * sizeof(type))
#define SPLTMRG_FREE(data) os_free(data)


//~~~~~~~~~~~~~~~~
//
// TYPES
//
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t i32;
typedef int64_t i64;

#if !defined(__cplusplus)
typedef enum bool {
    false = 0,
    true  = 1,
} bool;
#endif

typedef struct File_Data {
    u8  *data;
    i64  length;
    i64  capacity;
} File_Data;

typedef struct String {
    char *data;
    i32   length;
    i32   capacity;
} String;

#define SPLITMERGE_FILE_VERSION 1

enum Header_Flags {
    Header_Flag__None       = 0x0,
	Header_Flag__Big_Endian = 0x1,
};

#include "splitmerge_header.h"

#define SPLITMERGE_HEADER_VALIDATION "S+M"
#define SPLITMERGE_FILE_EXTENSION ".spltmrg"
#define SPLITMERGE_FILE_EXTENSION_CSTRING UNPACK_NTSTRING(SPLITMERGE_FILE_EXTENSION)

#define SPLITMERGE_MAX_FILE_NAME_LENGTH 0xFFFF

#define SPLITMERGE_WELCOME_MSG "SPLITMERGE"
#define SPLITMERGE_FILE_LIMIT 0x7FFBFF
#define SPLITMERGE_MAX_UNSPLIT_FILE_SIZE (0xFFFF * (SPLITMERGE_FILE_LIMIT - sizeof(Shared_Header)))
#define SPLITMERGE_MAX_FILE_NAME_AND_HEADER_SIZE (SPLITMERGE_MAX_FILE_NAME_LENGTH + sizeof(First_Header))
#define SPLITMERGE_MAX_FIRST_FILE_SIZE (SPLITMERGE_FILE_LIMIT - sizeof(First_Header))

#define SPLITMERGE_NITRO_WELCOME_MSG "SPLITMERGE_NITRO"
#define SPLITMERGE_NITRO_FILE_LIMIT 0x63FFC00
#define SPLITMERGE_NITRO_MAX_UNSPLIT_FILE_SIZE \
0xFFFF * (SPLITMERGE_NITRO_FILE_LIMIT - sizeof(Shared_Header)))
#define SPLITMERGE_NITRO_MAX_FIRST_FILE_SIZE (SPLITMERGE_NITRO_FILE_LIMIT - sizeof(First_Header))
