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
#include "splitmerge.c"


//~~~~~~~~~~~~~~~~
//
// TYPES
//
typedef struct Merge_Bundle {
    String *files;
    u32     file_count;
    u32     file_capacity;
    
    u32    unique_id;
    String file_name;
} Merge_Bundle;

typedef struct Merge_Bundle_Array {
    Merge_Bundle *data;
    i32 count;
    i32 capacity;
} Merge_Bundle_Array;


//~~~~~~~~~~~~~~~~
//
// HEADER
//
static bool
is_valid_header(Shared_Header header) {
    if((header.validation_0 == 'S') &&
       (header.validation_1 == '+') &&
       (header.validation_2 == 'M'))
    {
        return true;
    }
    return false;
}


//~~~~~~~~~~~~~~~~
//
// ENDIAN
//
static u16
swap_endian_u16(u16 value) {
    u16 result = ((value << 8) & 0xff00 |
                  (value >> 8) & 0x00ff);
    
    return result;
}

static u32
swap_endian_u32(u32 value) {
    u32 result = ((value << 24) & 0xff000000 |
                  (value <<  8) & 0x00ff0000 |
                  (value >>  8) & 0x0000ff00 |
                  (value >> 24) & 0x000000ff);
    
    return result;
}

static u64
swap_endian_u64(u64 value) {
    u64 result = ((value << 32) & 0xffffffff00000000 |
                  (value >> 32) & 0x00000000ffffffff);
    
    result = ((result << 16) & 0xffff0000ffff0000 |
              (result >> 16) & 0x0000ffff0000ffff);
    
    result = ((result << 8) & 0xff00ff00ff00ff00 |
              (result >> 8) & 0x00ff00ff00ff00ff);
    
    return result;
}

static bool
should_swap_endian(u8 flags) {
    if(is_big_endian() != is_flag_set(flags, Header_Flag__Big_Endian)) {
        return true;
    }
    return false;
}

static Merge_Bundle
make_merge_bundle(u32 unique_id) {
    Merge_Bundle result = {0};
    
    result.unique_id     = unique_id;
    result.file_capacity = 128;
    result.files         = ZI_ALLOC(String, result.file_capacity);
    
    return result;
}


//~~~~~~~~~~~~~~~~
//
// BUNDLE
//
static void
append_bundle(Merge_Bundle_Array *array, Merge_Bundle bundle) {
    if(array) {
        if(array->count + 1 >= array->capacity) {
            array->capacity += 4;
            array->data = ZI_REALLOC(Merge_Bundle, array->data, array->capacity);
        }
        
        array->data[array->count] = bundle;
        array->count += 1;
    }
}

static void
append_file_name(Merge_Bundle *bundle, String file_name, u32 file_index) {
    if(bundle) {
        if(file_index >= bundle->file_capacity) {
            while(file_index >= bundle->file_capacity) {
                bundle->file_capacity += 128;
            }
            bundle->files = ZI_REALLOC(String, bundle->files, bundle->file_capacity);
        }
        
        bundle->files[file_index] = file_name;
        bundle->file_count += 1;
    }
}


//~~~~~~~~~~~~~~~~
//
// MAIN
//
int
main(int arg_count, char **arg_data) {
    Merge_Bundle_Array master_list = {0};
    master_list.capacity = 4;
    master_list.data     = ZI_ALLOC(Merge_Bundle, master_list.capacity);
    
    printf("SPLITMERGE <merge>\n");
    printf("%d potential split files.\n", arg_count - 1);
    
    String source_path = set_string_from_ntstring(arg_data[0]);
    
    for_range(int, arg_index, 1, arg_count) {
        String arg = set_string_from_ntstring(arg_data[arg_index]);
        
        File_Handle file_handle = open_file_for_reading(arg.data);
        
		if(is_file(file_handle)) {
            File_Data file = make_file_data(SPLITMERGE_MAX_FILE_NAME_AND_HEADER_SIZE);
            
            if(read_file(&file, file_handle, sizeof(First_Header)) > 0) {
                Shared_Header *shared_header = (Shared_Header*)file.data;
                
                if(is_valid_header(*shared_header)) {
                    Merge_Bundle *bundle = 0;
                    
                    if(should_swap_endian(shared_header->flags)) {
                        shared_header->version    = swap_endian_u16(shared_header->version);
                        shared_header->unique_id  = swap_endian_u32(shared_header->unique_id);
                        shared_header->file_index = swap_endian_u16(shared_header->file_index);
                    }
                    
                    For(i32, it_index, master_list.count) {
                        Merge_Bundle *it = master_list.data + it_index;
                        
                        if(it->unique_id == shared_header->unique_id) {
                            bundle = it;
                            break;
                        }
                    }
                    
                    if(!bundle) {
                        Merge_Bundle new_bundle = make_merge_bundle(shared_header->unique_id);
                        append_bundle(&master_list, new_bundle);
                        bundle = &master_list.data[master_list.count - 1];
                    }
                    
                    if(shared_header->file_index == 0) {
                        First_Header *header = (First_Header*)file.data;
                        
                        if(should_swap_endian(shared_header->flags)) {
                            header->file_name_length = swap_endian_u16(header->file_name_length);
                        }
                        
                        char *file_name_pos = (char*)file.data + file.length;
                        
                        bundle->file_name = make_string(64);
                        
                        append_string(&bundle->file_name, source_path);
                        
                        {
                            i32 index = find_index_of_last(bundle->file_name, '/');
                            
                            if(index >= 0) {
                                bundle->file_name.length -= bundle->file_name.length - index - 1;
                            } else {
                                index = find_index_of_last(bundle->file_name, '\\');
                                
                                if(index >= 0) {
                                    bundle->file_name.length -= bundle->file_name.length - index - 1;
                                }
                            }
                        }
                        
                        append_cstring(&bundle->file_name, UNPACK_NTSTRING("merged_output/"));
                        
                        if(read_file(&file, file_handle, header->file_name_length) > 0) {
                            append_cstring(&bundle->file_name, file_name_pos, header->file_name_length);
                            null_terminate(&bundle->file_name);
                        }
                    }
                    
                    append_file_name(bundle, arg, shared_header->file_index);
                }
            }
            
            ZI_FREE(file.data);
		}
        
        close_file(file_handle);
	}
    
    if(master_list.count > 0) {
        File_Data dest_file   = make_file_data(SPLITMERGE_FILE_LIMIT);
        File_Data source_file = make_file_data(SPLITMERGE_FILE_LIMIT);
        
        For(i32, bundle_index, master_list.count) {
            printf("---===##===---\n");
            
            Merge_Bundle *bundle = master_list.data + bundle_index;
            
            File_Handle dest_handle = open_file_for_writing(bundle->file_name.data);
            
            if(dest_handle) {
                For(u32, file_index, bundle->file_count) {
                    File_Handle source_handle = open_file_for_reading(bundle->files[file_index].data);
                    
                    source_file.length = 0;
                    if(source_handle) {
                        if(read_file(&source_file, source_handle, source_file.capacity) > 0) {
                            u8  *data   = source_file.data;
                            i64  length = source_file.length;
                            
                            if(file_index == 0) {
                                First_Header *header = (First_Header*)source_file.data;
                                
                                data   += sizeof(First_Header) + header->file_name_length + 1;
                                length -= sizeof(First_Header) + header->file_name_length + 1;
                            } else {
                                data   += sizeof(Shared_Header);
                                length -= sizeof(Shared_Header);
                            }
                            
                            printf("Merging bundle %d/%d - file %u/%u\n",
                                   bundle_index + 1, master_list.count,
                                   file_index + 1, bundle->file_count);
                            write_data_to_file(dest_handle, data, length);
                        }
                    }
                    
                    close_file(source_handle);
                }
            }
            
            close_file(dest_handle);
        }
        
        ZI_FREE(source_file.data);
        ZI_FREE(dest_file.data);
    }
    
	return 0;
}
