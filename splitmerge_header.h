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
// PRAGMA PUSH
//
#pragma pack(push, 1)


//~~~~~~~~~~~~~~~~
//
// SHARED
//
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


//~~~~~~~~~~~~~~~~
//
// FIRST
//
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


//~~~~~~~~~~~~~~~~
//
// PRAGMA POP
//
#pragma pack(pop)

