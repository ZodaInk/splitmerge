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
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


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

// NOTE(Patrik): ntstring is a null-terminated string (char *str)
// cstring is a pointer and a length (char *data, i32 length)
// String is the struct version of cstring
#define UNPACK_NTSTRING(str) (str), (sizeof(str) - 1)


//~~~~~~~~~~~~~~~~
//
// MEMORY
//
#define ZI_ALLOC(type, count) (type *)spltmrg_alloc(count * sizeof(type))
#define ZI_REALLOC(type, data, new_count) (type *)realloc(data, new_count * sizeof(type))
#define ZI_FREE(data) free(data)


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

typedef FILE* File_Handle;

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

enum Header_Flags {
	Header_Flag__None       = 0x0,
	Header_Flag__Big_Endian = 0x1,
	
	Header_Flag__All = 0xff,
};

#pragma pack(push, 1)

typedef struct Shared_Header {
	//~ NOTE(Patrik): The first byte should always be the flags.
    // This is mainly so that you can easily determine the endianness of the header.
	unsigned char flags;
    
    //~ NOTE(Patrik): Header validation bytes should always be "S+M" in that order.
    char validation_0; // 'S'
    char validation_1; // '+'
    char validation_2; // 'M'
    
    //~ NOTE(Patrik): Header version number.
    u16 version;
	
	//~ NOTE(Patrik): This id is to identify which file belongs to each other.
	// How this number is made is not too important, it just needs to not
    // conflict with another file currently trying to be merged.
	u32 unique_id;
	
	//~ NOTE(Patrik): The order in which to merge the split files.
	u16 file_index;
} Shared_Header;

typedef struct First_Header {
	//~ NOTE(Patrik): Shared header should always be first.
	Shared_Header shared;
    
    //~ NOTE(Patrik): total_file_count is the amount of split files.
    // Used to validate that the files recieved when merging is the correct amount.
    u16 total_file_count;
	
    //~ NOTE(Patrik): file_name_length is the length of the string.
    // There is no null terminator included in the file or in the length of the string.
	u16 file_name_length;
} First_Header;

#pragma pack(pop)

#define SPLITMERGE_FILE_VERSION 0

#define SPLITMERGE_FILE_LIMIT 0x7fffff
#define SPLITMERGE_MAX_UNSPLIT_FILE_SIZE (0xffff * (SPLITMERGE_FILE_LIMIT - sizeof(Shared_Header)))
#define SPLITMERGE_MAX_FILE_NAME_AND_HEADER_SIZE (0xffff + sizeof(First_Header))
#define SPLITMERGE_MAX_FIRST_FILE_SIZE (SPLITMERGE_FILE_LIMIT - sizeof(First_Header))
